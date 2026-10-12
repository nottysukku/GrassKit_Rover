/**
  ******************************************************************************
  * @file           : safety.c
  * @brief          : Safety supervisor implementation. Monitors communication
  *                   timeout, hardware e-stop button, and system health.
  ******************************************************************************
  */

#include "safety.h"
#include "motor_control.h"
#include "engine_model.h"
#include "bluetooth.h"
#include <string.h>

static SafetySupervisor_t g_safety;

void Safety_Init(void)
{
    memset(&g_safety, 0, sizeof(g_safety));
    g_safety.last_heartbeat_ms = HAL_GetTick();
    g_safety.timeout_limit_ms = SAFETY_COMM_TIMEOUT_MS;
    g_safety.estop_latched = false;
    g_safety.comm_timeout_occurred = false;

    /* Initialize Nucleo User Button B1 (PC13) */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = USER_BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL; /* Nucleo board has external pull-up */
    HAL_GPIO_Init(USER_BUTTON_PORT, &GPIO_InitStruct);
}

void Safety_KickWatchdog(void)
{
    g_safety.last_heartbeat_ms = HAL_GetTick();
    g_safety.comm_timeout_occurred = false;
}

void Safety_Update(uint32_t delta_ms)
{
    /* 1. Check Physical Blue Button B1 on Nucleo (Active LOW when pressed) */
    if (HAL_GPIO_ReadPin(USER_BUTTON_PORT, USER_BUTTON_PIN) == GPIO_PIN_RESET) {
        if (!g_safety.estop_latched) {
            Safety_TriggerEmergencyStop();
            Bluetooth_Send("ALARM:HARDWARE_ESTOP_BUTTON_PRESSED\r\n");
        }
        return;
    }

    if (g_safety.estop_latched) {
        return;
    }

    /* 2. Check Bluetooth Communication Heartbeat Timeout */
    uint32_t now = HAL_GetTick();
    if ((now - g_safety.last_heartbeat_ms) > g_safety.timeout_limit_ms) {
        /* Only auto-stop if car was actively running */
        if (Motor_GetState() == MOTOR_STATE_RUNNING) {
            g_safety.comm_timeout_occurred = true;
            Motor_Stop();
            Engine_SetThrottle(0.0f);
            Bluetooth_Send("WARN:COMM_TIMEOUT_AUTO_STOP\r\n");
        }
    }
}

void Safety_TriggerEmergencyStop(void)
{
    g_safety.estop_latched = true;
    Motor_EmergencyStop();
    Engine_SetThrottle(0.0f);
}

void Safety_ResetEmergencyStop(void)
{
    g_safety.estop_latched = false;
    g_safety.comm_timeout_occurred = false;
    g_safety.last_heartbeat_ms = HAL_GetTick();
    Motor_ResetFault();
}

bool Safety_IsEmergencyStopActive(void)
{
    return g_safety.estop_latched;
}

bool Safety_IsTimeoutActive(void)
{
    return g_safety.comm_timeout_occurred;
}
