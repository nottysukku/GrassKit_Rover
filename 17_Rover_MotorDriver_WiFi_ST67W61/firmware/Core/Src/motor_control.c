/**
  ******************************************************************************
  * @file           : motor_control.c
  * @brief          : Motor driver implementation for STSPIN948 on X-STM32MP-RBT01.
  *                   Handles 4 brushed DC motors with differential & Mecanum drive.
  ******************************************************************************
  */

#include "motor_control.h"
#include <math.h>
#include <string.h>

static MotorControl_t g_motor_ctrl;
static TIM_HandleTypeDef htim3_motors;

static const uint16_t s_motor_en_pins[4] = {
    MOTOR1_EN_PIN, MOTOR2_EN_PIN, MOTOR3_EN_PIN, MOTOR4_EN_PIN
};

/* Private Helper Declarations */
static void Motor_Hardware_Init_GPIO(void);
static void Motor_Hardware_Init_TIM3(void);
static void Motor_SetBridgesEnabled(bool enable);
static bool Motor_CheckDriverFault(uint32_t delta_ms);
static void Motor_ApplyPWMAndDirection(uint8_t id, float speed);
static float Motor_Clamp(float val, float min_val, float max_val);
static void Motor_SetTargets(const float m[4]);
static void Motor_LoadSettings(void);

/* Saved wheel settings: 4 words in the data EEPROM */
#define MOTOR_CFG_ADDR    (DATA_EEPROM_BASE + 0x40UL)
#define MOTOR_CFG_MAGIC   0x47524B32UL   /* "GRK2" */
#define MOTOR_CFG_SALT    0x5A5A5A5AUL

void Motor_LoadDefaults(void)
{
    g_motor_ctrl.drive_mode = DRIVE_MODE_MECANUM_OMNI;
    g_motor_ctrl.strafe_flip = false;
    /* Wiring check 2026-11-02: FL is wired like RL and FR like RR, so each
     * front wheel uses the same direction setting as the rear wheel behind it. */
    g_motor_ctrl.link_sides = true;
    for (int i = 0; i < 4; i++)
    {
        /* Bench test 2026-10-30: on both rear wheels PH high turned the wheel
         * backwards. So forward = PH low, and with the side link the front
         * wheels match. */
        g_motor_ctrl.motors[i].invert_dir = true;
        g_motor_ctrl.motors[i].trim_pct = 100U;
    }
}

void Motor_Init(void)
{
    memset(&g_motor_ctrl, 0, sizeof(g_motor_ctrl));
    g_motor_ctrl.state = MOTOR_STATE_DISABLED;
    Motor_LoadDefaults();
    Motor_LoadSettings();

    /* Initialize low-level peripherals */
    Motor_Hardware_Init_GPIO();
    Motor_Hardware_Init_TIM3();

    /* Ensure all four bridges are held disabled (outputs high-Z) initially */
    Motor_SetBridgesEnabled(false);

    g_motor_ctrl.state = MOTOR_STATE_IDLE;
}

static void Motor_Hardware_Init_GPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO Clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Bridge EN/nFAULT Pins: PC4, PC5, PC9, PC8 Output Open-Drain.
     * The RBT01 pulls each line to 3V3 (bridge enabled by default), so latch
     * them low before switching to output mode. */
    HAL_GPIO_WritePin(MOTOR_EN_GPIO_PORT, MOTOR_EN_ALL_PINS, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = MOTOR_EN_ALL_PINS;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MOTOR_EN_GPIO_PORT, &GPIO_InitStruct);

    /* Direction (PHx) Pins: PC0, PC1, PC2, PC3 Output Push-Pull with pull-down */
    GPIO_InitStruct.Pin = MOTOR1_DIR_PIN | MOTOR2_DIR_PIN | MOTOR3_DIR_PIN | MOTOR4_DIR_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* All Direction outputs low initially */
    HAL_GPIO_WritePin(GPIOC, MOTOR1_DIR_PIN | MOTOR2_DIR_PIN | MOTOR3_DIR_PIN | MOTOR4_DIR_PIN, GPIO_PIN_RESET);
}

static void Motor_Hardware_Init_TIM3(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    MOTOR_PWM_TIMER_CLK_ENABLE();

    /* Motor 1 PWM: Configure PC6 as TIM3_CH1 Alternate Function AF2 with pull-down */
    GPIO_InitStruct.Pin = MOTOR1_PWM_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(MOTOR1_PWM_GPIO_PORT, &GPIO_InitStruct);

    /* Motor 3 (PB0 / CH3) and Motor 4 (PB1 / CH4) as TIM3 AF2 with pull-down */
    GPIO_InitStruct.Pin = MOTOR3_PWM_PIN | MOTOR4_PWM_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Motor 2 (PB5 / CH2) as TIM3 AF4 */
    GPIO_InitStruct.Pin = MOTOR2_PWM_PIN;
    GPIO_InitStruct.Alternate = GPIO_AF4_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Timer Base Configuration: 32 MHz / 1 (Prescaler 0) = 32 MHz tick.
     * Period = 1600 - 1 -> Frequency = 32,000,000 / 1600 = 20,000 Hz (20 kHz) */
    htim3_motors.Instance = MOTOR_PWM_TIMER;
    htim3_motors.Init.Prescaler = 0;
    htim3_motors.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3_motors.Init.Period = MOTOR_PWM_PERIOD_TICKS - 1;
    htim3_motors.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3_motors.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim3_motors);

    /* PB5 (AF4) carries TIM22_CH2 by default: route TIM3_CH2 to it instead */
    HAL_TIMEx_RemapConfig(&htim3_motors, TIM3_TI2_GPIOB5_AF4);

    /* Output Compare Configuration:
     * Phase-interleave Bridge A (M1/CH1, M3/CH3 -> PWM Mode 1, active at start of period)
     * against Bridge B (M2/CH2, M4/CH4 -> PWM Mode 2, active at end of period).
     * This cuts instantaneous peak bus current and di/dt switching spikes in half! */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(&htim3_motors, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim3_motors, &sConfigOC, TIM_CHANNEL_3);

    sConfigOC.OCMode = TIM_OCMODE_PWM2;
    sConfigOC.Pulse = MOTOR_PWM_PERIOD_TICKS; /* CCR = 1600 -> 0% initial duty in PWM2 */
    HAL_TIM_PWM_ConfigChannel(&htim3_motors, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim3_motors, &sConfigOC, TIM_CHANNEL_4);

    /* Start all 4 PWM channels */
    HAL_TIM_PWM_Start(&htim3_motors, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3_motors, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3_motors, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3_motors, TIM_CHANNEL_4);
}

static void Motor_SetBridgesEnabled(bool enable)
{
    g_motor_ctrl.bridges_enabled = enable;
    if (enable)
    {
        /* Release the open-drain lines so the RBT01 39k pull-ups bring them to 3.3V */
        HAL_GPIO_WritePin(MOTOR_EN_GPIO_PORT, MOTOR_EN_ALL_PINS, GPIO_PIN_SET);
        g_motor_ctrl.fault_blanking_ms = MOTOR_FAULT_BLANKING_MS;
        memset(g_motor_ctrl.fault_samples, 0, sizeof(g_motor_ctrl.fault_samples));
    }
    else
    {
        /* Pull all EN lines to GND to disable power output stages */
        HAL_GPIO_WritePin(MOTOR_EN_GPIO_PORT, MOTOR_EN_ALL_PINS, GPIO_PIN_RESET);
        g_motor_ctrl.fault_blanking_ms = 0;
    }
}

/* Read the EN/nFAULT lines continuously for one whole PWM period (50 us) and
 * return the pins that were high at any moment. A real driver fault holds its
 * line low far longer than that, while switching noise picked up from the
 * motor and PWM wires pulls a line low for only part of each period. (The main
 * loop runs every 1 ms = exactly 20 PWM periods, so a single read per loop
 * lands at the same point of the PWM cycle every time and can see that noise
 * on every read; bench test 2026-11-02 tripped false faults this way.) */
static uint32_t Motor_ReadEnHighOverPwmPeriod(void)
{
    uint32_t high = 0U;
    uint32_t prev = htim3_motors.Instance->CNT;
    uint32_t elapsed = 0U;
    for (uint32_t guard = 0U; (elapsed < MOTOR_PWM_PERIOD_TICKS) && (guard < 1000U); guard++)
    {
        high |= MOTOR_EN_GPIO_PORT->IDR;
        uint32_t now = htim3_motors.Instance->CNT;
        elapsed += (now >= prev) ? (now - prev) : (now + MOTOR_PWM_PERIOD_TICKS - prev);
        prev = now;
    }
    return high;
}

static bool Motor_CheckDriverFault(uint32_t delta_ms)
{
    if (!g_motor_ctrl.bridges_enabled)
    {
        return false;
    }

    if (g_motor_ctrl.fault_blanking_ms > 0)
    {
        if (delta_ms >= g_motor_ctrl.fault_blanking_ms)
        {
            g_motor_ctrl.fault_blanking_ms = 0;
        }
        else
        {
            g_motor_ctrl.fault_blanking_ms -= delta_ms;
        }
        return false;
    }

    uint8_t current_mask = 0;
    uint32_t high = Motor_ReadEnHighOverPwmPeriod();
    for (uint8_t i = 0; i < 4; i++)
    {
        if ((high & s_motor_en_pins[i]) == 0U)
        {
            if (g_motor_ctrl.fault_samples[i] < 255)
            {
                g_motor_ctrl.fault_samples[i]++;
            }
            if (g_motor_ctrl.fault_samples[i] >= MOTOR_FAULT_DEBOUNCE_SAMPLES)
            {
                current_mask |= (uint8_t)(1U << i);
            }
        }
        else
        {
            g_motor_ctrl.fault_samples[i] = 0;
        }
    }

    g_motor_ctrl.fault_mask = current_mask;
    return (current_mask != 0);
}

void Motor_Start(void)
{
    if (g_motor_ctrl.state == MOTOR_STATE_ESTOP)
    {
        return;
    }

    /* Auto-clear prior driver UVLO/overcurrent fault when starting a new run */
    if (g_motor_ctrl.state == MOTOR_STATE_FAULT)
    {
        Motor_ResetFault();
    }

    Motor_SetBridgesEnabled(true);
    g_motor_ctrl.state = MOTOR_STATE_RUNNING;
}

void Motor_Stop(void)
{
    for (int i = 0; i < 4; i++)
    {
        g_motor_ctrl.motors[i].target_speed = 0.0f;
    }
    g_motor_ctrl.overall_throttle = 0.0f;
    g_motor_ctrl.overall_steer = 0.0f;
    g_motor_ctrl.overall_strafe = 0.0f;

    if (g_motor_ctrl.state == MOTOR_STATE_RUNNING)
    {
        g_motor_ctrl.state = MOTOR_STATE_IDLE;
    }
}

void Motor_Brake(void)
{
    for (int i = 0; i < 4; i++)
    {
        g_motor_ctrl.motors[i].target_speed = 0.0f;
        g_motor_ctrl.motors[i].current_speed = 0.0f;
        Motor_ApplyPWMAndDirection(i, 0.0f);
    }
    g_motor_ctrl.overall_throttle = 0.0f;
    g_motor_ctrl.overall_steer = 0.0f;
    g_motor_ctrl.overall_strafe = 0.0f;

    if (g_motor_ctrl.state == MOTOR_STATE_RUNNING)
    {
        g_motor_ctrl.state = MOTOR_STATE_BRAKING;
    }
}

void Motor_EmergencyStop(void)
{
    Motor_SetBridgesEnabled(false);

    for (int i = 0; i < 4; i++)
    {
        g_motor_ctrl.motors[i].target_speed = 0.0f;
        g_motor_ctrl.motors[i].current_speed = 0.0f;
        Motor_ApplyPWMAndDirection(i, 0.0f);
    }

    g_motor_ctrl.overall_throttle = 0.0f;
    g_motor_ctrl.overall_steer = 0.0f;
    g_motor_ctrl.overall_strafe = 0.0f;
    g_motor_ctrl.estop_active = true;
    g_motor_ctrl.state = MOTOR_STATE_ESTOP;
}

void Motor_ResetFault(void)
{
    g_motor_ctrl.fault_detected = false;
    g_motor_ctrl.fault_mask = 0;
    g_motor_ctrl.estop_active = false;
    memset(g_motor_ctrl.fault_samples, 0, sizeof(g_motor_ctrl.fault_samples));

    Motor_SetBridgesEnabled(false);
    g_motor_ctrl.state = MOTOR_STATE_IDLE;
}

void Motor_SetDrive(float throttle, float turn, float strafe)
{
    if (g_motor_ctrl.state == MOTOR_STATE_ESTOP)
    {
        return;
    }

    g_motor_ctrl.overall_throttle = Motor_Clamp(throttle, -100.0f, 100.0f);
    g_motor_ctrl.overall_steer = Motor_Clamp(turn, -100.0f, 100.0f);
    g_motor_ctrl.overall_strafe = Motor_Clamp(strafe, -100.0f, 100.0f);

    float th = g_motor_ctrl.overall_throttle;
    float tn = g_motor_ctrl.overall_steer;
    float sf = g_motor_ctrl.overall_strafe;
    if (g_motor_ctrl.drive_mode == DRIVE_MODE_DIFFERENTIAL)
    {
        sf = 0.0f;  /* plain wheels cannot move sideways */
    }
    else if (g_motor_ctrl.strafe_flip)
    {
        sf = -sf;
    }

    /* + = wheel rolls forward. turn > 0 drives the left side forward and the
     * right side back (clockwise). strafe > 0 moves right on mecanum wheels. */
    float m[4];
    m[0] = th + tn + sf;   /* FL */
    m[1] = th - tn - sf;   /* FR */
    m[2] = th + tn - sf;   /* RL */
    m[3] = th - tn + sf;   /* RR */

    /* Keep the mix shape: scale all wheels together if any exceeds 100 % */
    float max_mag = 0.0f;
    for (int i = 0; i < 4; i++)
    {
        float abs_val = fabsf(m[i]);
        if (abs_val > max_mag) max_mag = abs_val;
    }
    if (max_mag > 100.0f)
    {
        float scale = 100.0f / max_mag;
        for (int i = 0; i < 4; i++)
        {
            m[i] *= scale;
        }
    }

    /* Per-wheel trim evens out motors that run faster than the others */
    for (int i = 0; i < 4; i++)
    {
        m[i] = Motor_Clamp(m[i] * (float)g_motor_ctrl.motors[i].trim_pct / 100.0f, -100.0f, 100.0f);
    }

    Motor_SetTargets(m);
}

void Motor_SetWheels(const float speed_percent[4])
{
    if (g_motor_ctrl.state == MOTOR_STATE_ESTOP)
    {
        return;
    }
    float m[4];
    for (int i = 0; i < 4; i++)
    {
        m[i] = Motor_Clamp(speed_percent[i], -100.0f, 100.0f);
    }
    g_motor_ctrl.overall_throttle = 0.0f;
    g_motor_ctrl.overall_steer = 0.0f;
    g_motor_ctrl.overall_strafe = 0.0f;
    Motor_SetTargets(m);
}

static void Motor_SetTargets(const float m[4])
{
    /* If a previous bench-supply UVLO sag tripped FAULT and a fresh drive
     * command arrives after stopping, automatically recover */
    if (g_motor_ctrl.state == MOTOR_STATE_FAULT)
    {
        Motor_ResetFault();
    }

    /* All four wheels start together; Motor_UpdateRamp keeps them in step */
    bool any_motion = false;
    for (int i = 0; i < 4; i++)
    {
        g_motor_ctrl.motors[i].target_speed = m[i];
        if (fabsf(m[i]) > 1.0f)
        {
            any_motion = true;
        }
    }

    /* Auto-enable bridges when driving */
    if (g_motor_ctrl.state != MOTOR_STATE_RUNNING && any_motion)
    {
        Motor_Start();
    }
}

void Motor_SetIndividual(uint8_t motor_id, float speed_percent)
{
    if (motor_id < 1 || motor_id > 4) return;
    if (g_motor_ctrl.state == MOTOR_STATE_ESTOP) return;

    if (g_motor_ctrl.state == MOTOR_STATE_FAULT)
    {
        Motor_ResetFault();
    }

    g_motor_ctrl.motors[motor_id - 1].target_speed = Motor_Clamp(speed_percent, -100.0f, 100.0f);

    if (g_motor_ctrl.state != MOTOR_STATE_RUNNING && fabsf(speed_percent) > 1.0f)
    {
        Motor_Start();
    }
}

void Motor_SetMode(DriveMode_t mode)
{
    g_motor_ctrl.drive_mode = mode;
}

DriveMode_t Motor_GetMode(void)
{
    return g_motor_ctrl.drive_mode;
}

/* Same-side partner: FL(0) <-> RL(2), FR(1) <-> RR(3) */
static int Motor_SidePartner(int idx)
{
    return idx ^ 2;
}

void Motor_SetInvert(uint8_t motor_id, bool invert)
{
    if (motor_id >= 1 && motor_id <= 4)
    {
        int idx = motor_id - 1;
        g_motor_ctrl.motors[idx].invert_dir = invert;
        if (g_motor_ctrl.link_sides)
        {
            g_motor_ctrl.motors[Motor_SidePartner(idx)].invert_dir = invert;
        }
    }
}

void Motor_SetLinkSides(bool link)
{
    g_motor_ctrl.link_sides = link;
    if (link)
    {
        /* The rear wheels' directions were checked on the bench: fronts follow them */
        g_motor_ctrl.motors[0].invert_dir = g_motor_ctrl.motors[2].invert_dir;
        g_motor_ctrl.motors[1].invert_dir = g_motor_ctrl.motors[3].invert_dir;
    }
}

bool Motor_GetLinkSides(void) { return g_motor_ctrl.link_sides; }

bool Motor_GetInvert(uint8_t motor_id)
{
    if (motor_id >= 1 && motor_id <= 4)
    {
        return g_motor_ctrl.motors[motor_id - 1].invert_dir;
    }
    return false;
}

void Motor_UpdateRamp(uint32_t delta_ms)
{
    if (Motor_CheckDriverFault(delta_ms))
    {
        Motor_SetBridgesEnabled(false);
        g_motor_ctrl.state = MOTOR_STATE_FAULT;
        g_motor_ctrl.fault_detected = true;
        for (int i = 0; i < 4; i++)
        {
            g_motor_ctrl.motors[i].target_speed = 0.0f;
            g_motor_ctrl.motors[i].current_speed = 0.0f;
            Motor_ApplyPWMAndDirection(i, 0.0f);
        }
        return;
    }

    if (g_motor_ctrl.state == MOTOR_STATE_FAULT || g_motor_ctrl.state == MOTOR_STATE_ESTOP)
    {
        return;
    }

    /* Synchronised ramp: every wheel covers the same fraction of its remaining
     * change each tick, so all four reach their targets at the same moment and
     * the speed ratios between wheels (the rover's heading) hold while speeding
     * up or slowing down. The wheel with the most to do sets the pace, and no
     * wheel changes faster than its own accel/decel limit. */
    float delta[4];
    float need_ms = 0.0f;
    for (int i = 0; i < 4; i++)
    {
        float target = g_motor_ctrl.motors[i].target_speed;
        float current = g_motor_ctrl.motors[i].current_speed;
        bool accelerating = (fabsf(target) > fabsf(current)) && (target * current >= 0.0f);
        float rate = accelerating ? MOTOR_RAMP_STEP_PER_MS : MOTOR_RAMP_DOWN_STEP_PER_MS;
        delta[i] = target - current;
        float t = fabsf(delta[i]) / rate;
        if (t > need_ms)
        {
            need_ms = t;
        }
    }
    float frac = (need_ms <= (float)delta_ms) ? 1.0f : ((float)delta_ms / need_ms);

    bool all_stopped = true;
    for (int i = 0; i < 4; i++)
    {
        float target = g_motor_ctrl.motors[i].target_speed;
        float current = (frac >= 1.0f) ? target : (g_motor_ctrl.motors[i].current_speed + delta[i] * frac);

        if (fabsf(current) > 0.01f || fabsf(target) > 0.01f)
        {
            all_stopped = false;
        }

        g_motor_ctrl.motors[i].current_speed = current;
        Motor_ApplyPWMAndDirection(i, current);
    }

    /* Once all wheels have ramped down to 0 in IDLE state, place bridges back in high-Z */
    if (g_motor_ctrl.state == MOTOR_STATE_IDLE && all_stopped && g_motor_ctrl.bridges_enabled)
    {
        Motor_SetBridgesEnabled(false);
    }
}

static void Motor_ApplyPWMAndDirection(uint8_t id, float speed)
{
    uint8_t dir = (speed >= 0.0f) ? 1 : 0;
    if (g_motor_ctrl.motors[id].invert_dir)
    {
        dir = dir ? 0 : 1;
    }

    float abs_speed = fabsf(speed);

    if (abs_speed < MOTOR_DEADBAND_PERCENT)
    {
        abs_speed = 0.0f;
    }

    uint16_t duty = (uint16_t)((abs_speed / 100.0f) * (float)MOTOR_PWM_PERIOD_TICKS);
    if (duty > MOTOR_PWM_MAX_DUTY) duty = MOTOR_PWM_MAX_DUTY;

    g_motor_ctrl.motors[id].pwm_duty = duty;
    g_motor_ctrl.motors[id].direction = dir;

    /* Apply GPIO Direction (PHx) and Phase-Interleaved PWM Compare:
     * - M1 (CH1) & M3 (CH3): PWM Mode 1 -> compare = duty
     * - M2 (CH2) & M4 (CH4): PWM Mode 2 -> compare = PERIOD - duty */
    GPIO_PinState pin_state = (dir == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    switch (id)
    {
        case 0:
            HAL_GPIO_WritePin(MOTOR1_DIR_GPIO_PORT, MOTOR1_DIR_PIN, pin_state);
            __HAL_TIM_SET_COMPARE(&htim3_motors, MOTOR1_PWM_CHANNEL, duty);
            break;
        case 1:
            HAL_GPIO_WritePin(MOTOR2_DIR_GPIO_PORT, MOTOR2_DIR_PIN, pin_state);
            __HAL_TIM_SET_COMPARE(&htim3_motors, MOTOR2_PWM_CHANNEL, MOTOR_PWM_PERIOD_TICKS - duty);
            break;
        case 2:
            HAL_GPIO_WritePin(MOTOR3_DIR_GPIO_PORT, MOTOR3_DIR_PIN, pin_state);
            __HAL_TIM_SET_COMPARE(&htim3_motors, MOTOR3_PWM_CHANNEL, duty);
            break;
        case 3:
            HAL_GPIO_WritePin(MOTOR4_DIR_GPIO_PORT, MOTOR4_DIR_PIN, pin_state);
            __HAL_TIM_SET_COMPARE(&htim3_motors, MOTOR4_PWM_CHANNEL, MOTOR_PWM_PERIOD_TICKS - duty);
            break;
        default:
            break;
    }
}

static float Motor_Clamp(float val, float min_val, float max_val)
{
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

void Motor_SetTrim(uint8_t motor_id, uint8_t trim_pct)
{
    if (motor_id < 1 || motor_id > 4) return;
    if (trim_pct < MOTOR_TRIM_MIN_PCT) trim_pct = MOTOR_TRIM_MIN_PCT;
    if (trim_pct > MOTOR_TRIM_MAX_PCT) trim_pct = MOTOR_TRIM_MAX_PCT;
    g_motor_ctrl.motors[motor_id - 1].trim_pct = trim_pct;
}

uint8_t Motor_GetTrim(uint8_t motor_id)
{
    if (motor_id < 1 || motor_id > 4) return 100U;
    return g_motor_ctrl.motors[motor_id - 1].trim_pct;
}

void Motor_SetStrafeFlip(bool flip) { g_motor_ctrl.strafe_flip = flip; }
bool Motor_GetStrafeFlip(void) { return g_motor_ctrl.strafe_flip; }

uint8_t Motor_GetDutyPercent(uint8_t motor_id)
{
    if (motor_id < 1 || motor_id > 4) return 0U;
    return (uint8_t)(((uint32_t)g_motor_ctrl.motors[motor_id - 1].pwm_duty * 100UL + (MOTOR_PWM_PERIOD_TICKS / 2UL)) /
                     MOTOR_PWM_PERIOD_TICKS);
}

uint8_t Motor_GetDirPin(uint8_t motor_id)
{
    if (motor_id < 1 || motor_id > 4) return 0U;
    return g_motor_ctrl.motors[motor_id - 1].direction;
}

static uint32_t Motor_PackFlags(void)
{
    uint32_t w = 0U;
    for (int i = 0; i < 4; i++)
    {
        if (g_motor_ctrl.motors[i].invert_dir) w |= (1UL << i);
    }
    if (g_motor_ctrl.drive_mode == DRIVE_MODE_MECANUM_OMNI) w |= (1UL << 4);
    if (g_motor_ctrl.strafe_flip) w |= (1UL << 5);
    if (!g_motor_ctrl.link_sides) w |= (1UL << 6);   /* stored inverted: 0 = linked (default) */
    return w;
}

static uint32_t Motor_PackTrims(void)
{
    uint32_t w = 0U;
    for (int i = 0; i < 4; i++)
    {
        w |= ((uint32_t)g_motor_ctrl.motors[i].trim_pct) << (8 * i);
    }
    return w;
}

static void Motor_LoadSettings(void)
{
    const volatile uint32_t *cfg = (const volatile uint32_t *)MOTOR_CFG_ADDR;
    uint32_t magic = cfg[0], flags = cfg[1], trims = cfg[2], check = cfg[3];
    if ((magic != MOTOR_CFG_MAGIC) || (check != (magic ^ flags ^ trims ^ MOTOR_CFG_SALT)))
    {
        return; /* nothing saved yet: keep defaults */
    }
    for (int i = 0; i < 4; i++)
    {
        g_motor_ctrl.motors[i].invert_dir = ((flags >> i) & 1UL) != 0UL;
        Motor_SetTrim((uint8_t)(i + 1), (uint8_t)((trims >> (8 * i)) & 0xFFUL));
    }
    g_motor_ctrl.drive_mode = ((flags >> 4) & 1UL) ? DRIVE_MODE_MECANUM_OMNI : DRIVE_MODE_DIFFERENTIAL;
    g_motor_ctrl.strafe_flip = ((flags >> 5) & 1UL) != 0UL;
    Motor_SetLinkSides(((flags >> 6) & 1UL) == 0UL);
}

bool Motor_SaveSettings(void)
{
    uint32_t w[4];
    w[0] = MOTOR_CFG_MAGIC;
    w[1] = Motor_PackFlags();
    w[2] = Motor_PackTrims();
    w[3] = w[0] ^ w[1] ^ w[2] ^ MOTOR_CFG_SALT;

    bool ok = (HAL_FLASHEx_DATAEEPROM_Unlock() == HAL_OK);
    for (uint32_t i = 0; ok && (i < 4U); i++)
    {
        ok = (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, MOTOR_CFG_ADDR + (4U * i), w[i]) == HAL_OK);
    }
    (void)HAL_FLASHEx_DATAEEPROM_Lock();

    const volatile uint32_t *cfg = (const volatile uint32_t *)MOTOR_CFG_ADDR;
    for (uint32_t i = 0; ok && (i < 4U); i++)
    {
        ok = (cfg[i] == w[i]);
    }
    return ok;
}

MotorState_t Motor_GetState(void) { return g_motor_ctrl.state; }
float Motor_GetThrottle(void) { return g_motor_ctrl.overall_throttle; }
float Motor_GetMotorSpeed(uint8_t motor_id)
{
    if (motor_id < 1 || motor_id > 4) return 0.0f;
    return g_motor_ctrl.motors[motor_id - 1].current_speed;
}
bool Motor_IsFaultActive(void) { return g_motor_ctrl.fault_detected; }
uint8_t Motor_GetFaultMask(void) { return g_motor_ctrl.fault_mask; }
