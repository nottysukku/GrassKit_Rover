/**
  ******************************************************************************
  * @file           : motor_control.c
  * @brief          : Motor driver implementation for STSPIN948 on X-STM32MP-RBT01.
  *                   Handles 4 brushed DC motors and 4-wheel Mecanum kinematics.
  ******************************************************************************
  */

#include "motor_control.h"
#include <math.h>
#include <string.h>

static MotorControl_t g_motor_ctrl;
static TIM_HandleTypeDef htim3_motors;

/* Private Helper Declarations */
static void Motor_Hardware_Init_GPIO(void);
static void Motor_Hardware_Init_TIM3(void);
static void Motor_ApplyPWMAndDirection(uint8_t id, float speed);
static float Motor_Clamp(float val, float min_val, float max_val);

void Motor_Init(void)
{
    memset(&g_motor_ctrl, 0, sizeof(g_motor_ctrl));
    g_motor_ctrl.state = MOTOR_STATE_DISABLED;
    g_motor_ctrl.drive_mode = DRIVE_MODE_MECANUM_OMNI;

    /* Initialize low-level peripherals */
    Motor_Hardware_Init_GPIO();
    Motor_Hardware_Init_TIM3();

    /* Ensure motor driver is held in standby / disabled initially */
    HAL_GPIO_WritePin(MOTOR_ENABLE_GPIO_PORT, MOTOR_ENABLE_PIN, GPIO_PIN_RESET);

    g_motor_ctrl.state = MOTOR_STATE_IDLE;
}

static void Motor_Hardware_Init_GPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO Clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Direction Pins: PC0, PC1, PC2, PC3 Output Push-Pull */
    GPIO_InitStruct.Pin = MOTOR1_DIR_PIN | MOTOR2_DIR_PIN | MOTOR3_DIR_PIN | MOTOR4_DIR_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* All Direction outputs low initially */
    HAL_GPIO_WritePin(GPIOC, MOTOR1_DIR_PIN | MOTOR2_DIR_PIN | MOTOR3_DIR_PIN | MOTOR4_DIR_PIN, GPIO_PIN_RESET);

    /* Motor Enable Pin: PC4 Output Push-Pull */
    GPIO_InitStruct.Pin = MOTOR_ENABLE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MOTOR_ENABLE_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(MOTOR_ENABLE_GPIO_PORT, MOTOR_ENABLE_PIN, GPIO_PIN_RESET);

    /* Motor Fault Pin: PC5 Input with Pull-Up */
    GPIO_InitStruct.Pin = MOTOR_FAULT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(MOTOR_FAULT_GPIO_PORT, &GPIO_InitStruct);
}

static void Motor_Hardware_Init_TIM3(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    MOTOR_PWM_TIMER_CLK_ENABLE();

    /* Configure PB4 (CH1), PB5 (CH2), PB0 (CH3), PB1 (CH4) as TIM3 Alternate Function */
    GPIO_InitStruct.Pin = MOTOR1_PWM_PIN | MOTOR2_PWM_PIN | MOTOR3_PWM_PIN | MOTOR4_PWM_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
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

    /* Output Compare Configuration (PWM Mode 1, initial duty 0) */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(&htim3_motors, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim3_motors, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim3_motors, &sConfigOC, TIM_CHANNEL_3);
    HAL_TIM_PWM_ConfigChannel(&htim3_motors, &sConfigOC, TIM_CHANNEL_4);

    /* Start PWM channels */
    HAL_TIM_PWM_Start(&htim3_motors, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3_motors, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3_motors, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3_motors, TIM_CHANNEL_4);
}

void Motor_Start(void)
{
    if (g_motor_ctrl.estop_active) {
        return; /* Require explicit reset if e-stop was tripped */
    }

    HAL_GPIO_WritePin(MOTOR_ENABLE_GPIO_PORT, MOTOR_ENABLE_PIN, GPIO_PIN_SET);
    g_motor_ctrl.state = MOTOR_STATE_RUNNING;
}

void Motor_Stop(void)
{
    /* Set all target speeds to zero gracefully */
    for (int i = 0; i < 4; i++) {
        g_motor_ctrl.motors[i].target_speed = 0.0f;
    }
    g_motor_ctrl.overall_throttle = 0.0f;
    g_motor_ctrl.overall_steer = 0.0f;
    g_motor_ctrl.overall_strafe = 0.0f;
    g_motor_ctrl.state = MOTOR_STATE_IDLE;
}

void Motor_Brake(void)
{
    /* Active braking: Immediately zero PWM duty but keep bridge enabled */
    for (int i = 0; i < 4; i++) {
        g_motor_ctrl.motors[i].target_speed = 0.0f;
        g_motor_ctrl.motors[i].current_speed = 0.0f;
        Motor_ApplyPWMAndDirection(i, 0.0f);
    }
    g_motor_ctrl.overall_throttle = 0.0f;
    g_motor_ctrl.state = MOTOR_STATE_BRAKING;
}

void Motor_EmergencyStop(void)
{
    /* Instant hardware shutdown: Disable driver & set duty to 0 */
    HAL_GPIO_WritePin(MOTOR_ENABLE_GPIO_PORT, MOTOR_ENABLE_PIN, GPIO_PIN_RESET);

    __HAL_TIM_SET_COMPARE(&htim3_motors, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3_motors, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim3_motors, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim3_motors, TIM_CHANNEL_4, 0);

    for (int i = 0; i < 4; i++) {
        g_motor_ctrl.motors[i].target_speed = 0.0f;
        g_motor_ctrl.motors[i].current_speed = 0.0f;
        g_motor_ctrl.motors[i].pwm_duty = 0;
    }
    g_motor_ctrl.overall_throttle = 0.0f;
    g_motor_ctrl.estop_active = true;
    g_motor_ctrl.state = MOTOR_STATE_ESTOP;
}

void Motor_ResetFault(void)
{
    g_motor_ctrl.estop_active = false;
    g_motor_ctrl.fault_detected = false;
    Motor_Stop();
}

void Motor_SetMode(DriveMode_t mode)
{
    g_motor_ctrl.drive_mode = mode;
}

/**
 * @brief  Mecanum Inverse Kinematics Solver
 * @param  throttle: Forward/Backward velocity (-100% to +100%)
 * @param  steer: Yaw / rotational velocity   (-100% to +100%)
 * @param  strafe: Lateral strafe velocity    (-100% to +100%)
 */
void Motor_SetDrive(float throttle, float steer, float strafe)
{
    if (g_motor_ctrl.estop_active) {
        return;
    }

    g_motor_ctrl.overall_throttle = fabsf(throttle);
    g_motor_ctrl.overall_steer = steer;
    g_motor_ctrl.overall_strafe = strafe;

    float fl, fr, rl, rr;

    if (g_motor_ctrl.drive_mode == DRIVE_MODE_RACECAR && strafe == 0.0f) {
        /* Standard differential race car steering */
        fl = throttle + steer;
        fr = throttle - steer;
        rl = throttle + steer;
        rr = throttle - steer;
    } else {
        /* Full 4-wheel Mecanum Holonomic Kinematics */
        fl = throttle + strafe + steer;
        fr = throttle - strafe - steer;
        rl = throttle - strafe + steer;
        rr = throttle + strafe - steer;
    }

    /* Normalization: prevent clipping if sum exceeds 100% */
    float max_val = fabsf(fl);
    if (fabsf(fr) > max_val) max_val = fabsf(fr);
    if (fabsf(rl) > max_val) max_val = fabsf(rl);
    if (fabsf(rr) > max_val) max_val = fabsf(rr);

    if (max_val > 100.0f) {
        float scale = 100.0f / max_val;
        fl *= scale;
        fr *= scale;
        rl *= scale;
        rr *= scale;
    }

    g_motor_ctrl.motors[0].target_speed = Motor_Clamp(fl, -100.0f, 100.0f);
    g_motor_ctrl.motors[1].target_speed = Motor_Clamp(fr, -100.0f, 100.0f);
    g_motor_ctrl.motors[2].target_speed = Motor_Clamp(rl, -100.0f, 100.0f);
    g_motor_ctrl.motors[3].target_speed = Motor_Clamp(rr, -100.0f, 100.0f);

    /* Auto-enable if positive throttle requested and in idle */
    if (g_motor_ctrl.state == MOTOR_STATE_IDLE &&
        (fabsf(fl) > 1.0f || fabsf(fr) > 1.0f || fabsf(rl) > 1.0f || fabsf(rr) > 1.0f)) {
        Motor_Start();
    }
}

void Motor_SetIndividual(uint8_t motor_id, float speed_percent)
{
    if (motor_id < 4 && !g_motor_ctrl.estop_active) {
        g_motor_ctrl.motors[motor_id].target_speed = Motor_Clamp(speed_percent, -100.0f, 100.0f);
    }
}

/**
 * @brief  Acceleration / Deceleration S-ramp filter update (called every 1 ms)
 */
void Motor_UpdateRamp(uint32_t delta_ms)
{
    if (g_motor_ctrl.estop_active) {
        return;
    }

    /* Check hardware fault line (active low) */
    if (HAL_GPIO_ReadPin(MOTOR_FAULT_GPIO_PORT, MOTOR_FAULT_PIN) == GPIO_PIN_RESET) {
        g_motor_ctrl.fault_detected = true;
        g_motor_ctrl.state = MOTOR_STATE_FAULT;
        Motor_EmergencyStop();
        return;
    }

    float max_step = MOTOR_RAMP_STEP_PER_MS * (float)delta_ms;

    for (int i = 0; i < 4; i++) {
        float diff = g_motor_ctrl.motors[i].target_speed - g_motor_ctrl.motors[i].current_speed;

        if (fabsf(diff) <= max_step) {
            g_motor_ctrl.motors[i].current_speed = g_motor_ctrl.motors[i].target_speed;
        } else if (diff > 0.0f) {
            g_motor_ctrl.motors[i].current_speed += max_step;
        } else {
            g_motor_ctrl.motors[i].current_speed -= max_step;
        }

        Motor_ApplyPWMAndDirection(i, g_motor_ctrl.motors[i].current_speed);
    }
}

static void Motor_ApplyPWMAndDirection(uint8_t id, float speed)
{
    float abs_speed = fabsf(speed);
    uint8_t dir = (speed >= 0.0f) ? 1 : 0;
    uint32_t duty = 0;

    if (abs_speed < MOTOR_DEADBAND_PERCENT) {
        duty = 0;
    } else {
        duty = (uint32_t)((abs_speed / 100.0f) * (float)MOTOR_PWM_PERIOD_TICKS);
        if (duty > MOTOR_PWM_MAX_DUTY) {
            duty = MOTOR_PWM_MAX_DUTY;
        }
    }

    g_motor_ctrl.motors[id].pwm_duty = duty;
    g_motor_ctrl.motors[id].direction = dir;

    GPIO_PinState pin_state = (dir == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    switch (id) {
        case 0: /* Front Left */
            HAL_GPIO_WritePin(MOTOR1_DIR_GPIO_PORT, MOTOR1_DIR_PIN, pin_state);
            __HAL_TIM_SET_COMPARE(&htim3_motors, MOTOR1_PWM_CHANNEL, duty);
            break;
        case 1: /* Front Right (inverted direction for symmetric mounting) */
            HAL_GPIO_WritePin(MOTOR2_DIR_GPIO_PORT, MOTOR2_DIR_PIN, (dir == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
            __HAL_TIM_SET_COMPARE(&htim3_motors, MOTOR2_PWM_CHANNEL, duty);
            break;
        case 2: /* Rear Left */
            HAL_GPIO_WritePin(MOTOR3_DIR_GPIO_PORT, MOTOR3_DIR_PIN, pin_state);
            __HAL_TIM_SET_COMPARE(&htim3_motors, MOTOR3_PWM_CHANNEL, duty);
            break;
        case 3: /* Rear Right (inverted direction for symmetric mounting) */
            HAL_GPIO_WritePin(MOTOR4_DIR_GPIO_PORT, MOTOR4_DIR_PIN, (dir == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
            __HAL_TIM_SET_COMPARE(&htim3_motors, MOTOR4_PWM_CHANNEL, duty);
            break;
        default:
            break;
    }
}

MotorState_t Motor_GetState(void)
{
    return g_motor_ctrl.state;
}

float Motor_GetThrottle(void)
{
    return g_motor_ctrl.overall_throttle;
}

float Motor_GetMotorSpeed(uint8_t motor_id)
{
    if (motor_id < 4) {
        return g_motor_ctrl.motors[motor_id].current_speed;
    }
    return 0.0f;
}

bool Motor_IsFaultActive(void)
{
    return g_motor_ctrl.fault_detected;
}

static float Motor_Clamp(float val, float min_val, float max_val)
{
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}
