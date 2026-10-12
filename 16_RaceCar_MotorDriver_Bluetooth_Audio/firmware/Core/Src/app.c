/**
  ******************************************************************************
  * @file           : app.c
  * @brief          : Master application executive and state machine.
  ******************************************************************************
  */

#include "app.h"
#include "motor_control.h"
#include "engine_model.h"
#include "audio_engine.h"
#include "bluetooth.h"
#include "command_parser.h"
#include "safety.h"
#include "debug.h"

static AppState_t g_app_state = APP_STATE_BOOT;
static uint32_t s_last_loop_tick = 0;
static uint32_t s_led_timer_ms = 0;

void App_Init(void)
{
    g_app_state = APP_STATE_INIT;

    /* Initialize LED indicator (PB2) */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = USER_LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(USER_LED_PORT, &GPIO_InitStruct);

    /* Initialize sub-modules */
    Debug_Init();
    Bluetooth_Init();
    Motor_Init();
    Engine_Init();
    Audio_Init();
    Safety_Init();
    Command_Init();

    /* Send boot confirmation */
    Debug_PrintBanner();
    Debug_Log("STATUS:BOOT_OK\r\n");
    Debug_Log("STATUS:MOTOR_INIT_OK\r\n");
    Debug_Log("STATUS:AUDIO_INIT_OK\r\n");
    Debug_Log("STATUS:BLUETOOTH_INIT_OK\r\n");

    s_last_loop_tick = HAL_GetTick();
    g_app_state = APP_STATE_IDLE;
}

void App_Run(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t delta_ms = now - s_last_loop_tick;

    if (delta_ms >= 1) {
        s_last_loop_tick = now;

        /* 1. Process incoming Bluetooth/UART serial commands */
        Bluetooth_Process();

        /* 2. Update Safety supervisor (watchdog and hardware buttons) */
        Safety_Update(delta_ms);

        /* 3. Update Motor PWM ramps */
        Motor_UpdateRamp(delta_ms);

        /* 4. Update Engine RPM and gearbox physics */
        float th = Motor_GetThrottle();
        Engine_Update(delta_ms, th);

        /* 5. Update Audio parameters from engine model */
        float rpm = Engine_GetRPM();
        bool shifting = Engine_IsShifting();
        bool rev_lim = Engine_IsRevLimiter();
        bool bov_pop = Engine_ConsumeBOVTrigger();
        Audio_UpdateParameters(rpm, th, shifting, rev_lim, bov_pop);

        /* 6. State Machine Synchronization */
        if (Safety_IsEmergencyStopActive()) {
            g_app_state = APP_STATE_ESTOP;
        } else if (Motor_IsFaultActive()) {
            g_app_state = APP_STATE_FAULT;
        } else if (shifting) {
            g_app_state = APP_STATE_GEAR_SHIFT;
        } else if (Motor_GetState() == MOTOR_STATE_BRAKING) {
            g_app_state = APP_STATE_BRAKING;
        } else if (Motor_GetState() == MOTOR_STATE_RUNNING) {
            g_app_state = APP_STATE_RUNNING;
        } else {
            g_app_state = APP_STATE_IDLE;
        }

        /* 7. Periodic telemetry logging */
        Debug_UpdatePeriodic(delta_ms);

        /* 8. Status Heartbeat LED Flash */
        s_led_timer_ms += delta_ms;
        uint32_t led_period = 500; /* 1 Hz flash in idle */
        if (g_app_state == APP_STATE_RUNNING) {
            led_period = 100;      /* Fast 5 Hz flash when driving */
        } else if (g_app_state == APP_STATE_ESTOP || g_app_state == APP_STATE_FAULT) {
            led_period = 50;       /* Rapid strobe on fault/estop */
        }

        if (s_led_timer_ms >= led_period) {
            s_led_timer_ms = 0;
            HAL_GPIO_TogglePin(USER_LED_PORT, USER_LED_PIN);
        }
    }
}

AppState_t App_GetState(void)
{
    return g_app_state;
}
