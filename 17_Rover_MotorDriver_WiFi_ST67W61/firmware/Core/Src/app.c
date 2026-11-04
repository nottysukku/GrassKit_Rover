/**
  ******************************************************************************
  * @file           : app.c
  * @brief          : Master application executive and supervisory state machine.
  ******************************************************************************
  */

#include "app.h"
#include "motor_control.h"
#include "wifi_st67w61.h"
#include "command_parser.h"
#include "safety.h"
#include "debug.h"

static AppState_t g_app_state = APP_STATE_BOOT;
static uint32_t s_last_loop_tick = 0;
static uint32_t s_led_timer_ms = 0;

void App_Init(void)
{
    g_app_state = APP_STATE_INIT;

    /* Initialize Status LED indicator (PB2) */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = USER_LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(USER_LED_PORT, &GPIO_InitStruct);

    /* Initialize serial debug console first (USART2 @ 115200 baud) */
    Debug_Init();
    Debug_PrintBanner();

    /* Motor driver outputs initialization: hold bridges disabled initially */
    Motor_Init();
    Debug_Log("[INIT] Motor controller initialized (20 kHz ultrasonic PWM on TIM3)\r\n");

    /* Safety & Command subsystems */
    Safety_Init();
    Command_Init();
    Debug_Log("[INIT] Safety supervisor initialized (Blue button E-stop & comms watchdog)\r\n");

    /* Initialize Wi-Fi Coprocessor (X-NUCLEO-67W61M1) */
    g_app_state = APP_STATE_WIFI_SETUP;
    WiFi_Init();

    s_last_loop_tick = HAL_GetTick();
    g_app_state = APP_STATE_IDLE;
    Debug_Log("[INIT] System initialization complete. Entering main control loop.\r\n");
}

void App_Run(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t delta_ms = now - s_last_loop_tick;

    if (delta_ms >= 1) {
        s_last_loop_tick = now;

        /* 1. Process USB Serial (USART2 VCP) & Wi-Fi Coprocessor packets */
        Debug_ProcessRx();
        WiFi_Process();

        /* 2. Update Safety supervisor (watchdog & physical E-stop button) */
        Safety_Update(delta_ms);

        /* 3. Update Motor PWM ramps & check driver fault feedback */
        Motor_UpdateRamp(delta_ms);

        /* 4. State Machine Synchronization */
        if (Safety_IsEmergencyStopActive()) {
            g_app_state = APP_STATE_ESTOP;
        } else if (Motor_IsFaultActive()) {
            if (g_app_state != APP_STATE_FAULT) {
                uint8_t mask = Motor_GetFaultMask();
                Debug_Log("ALARM:DRIVER_FAULT:M1=%u,M2=%u,M3=%u,M4=%u\r\n",
                          mask & 1U, (mask >> 1) & 1U, (mask >> 2) & 1U, (mask >> 3) & 1U);
            }
            g_app_state = APP_STATE_FAULT;
        } else if (Motor_GetState() == MOTOR_STATE_BRAKING) {
            g_app_state = APP_STATE_BRAKING;
        } else if (Motor_GetState() == MOTOR_STATE_RUNNING) {
            g_app_state = APP_STATE_RUNNING;
        } else {
            g_app_state = APP_STATE_IDLE;
        }

        /* 5. Periodic telemetry transmission */
        Debug_UpdatePeriodic(delta_ms);

        /* 6. Heartbeat LED Blink Pattern */
        s_led_timer_ms += delta_ms;
        uint32_t blink_period_ms = 500; /* Default 1 Hz blink */

        if (g_app_state == APP_STATE_ESTOP || g_app_state == APP_STATE_FAULT) {
            blink_period_ms = 100; /* Rapid panic blink */
        } else if (WiFi_IsClientConnected()) {
            blink_period_ms = 250; /* Active connection blink */
        }

        if (s_led_timer_ms >= blink_period_ms) {
            s_led_timer_ms = 0;
            HAL_GPIO_TogglePin(USER_LED_PORT, USER_LED_PIN);
        }
    }
}

AppState_t App_GetState(void)
{
    return g_app_state;
}
