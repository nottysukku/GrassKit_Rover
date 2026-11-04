/**
  ******************************************************************************
  * @file           : safety.c
  * @brief          : Safety supervisor implementation. Monitors communication
  *                   timeout, hardware e-stop button, and system health.
  ******************************************************************************
  */

#include "safety.h"
#include "motor_control.h"
#include "wifi_st67w61.h"
#include "debug.h"
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

void Safety_FeedCommunicationWatchdog(void)
{
    Safety_KickWatchdog();
}

void Safety_Update(uint32_t delta_ms)
{
    /* 1. Check Physical Blue Button B1 on Nucleo (Active LOW when pressed) */
    if (HAL_GPIO_ReadPin(USER_BUTTON_PORT, USER_BUTTON_PIN) == GPIO_PIN_RESET) {
        if (!g_safety.estop_latched) {
            Safety_TriggerEmergencyStop();
            Debug_Log("[SAFETY] Physical E-Stop button pressed!\r\n");
            WiFi_BroadcastTelemetry("ALARM:HARDWARE_ESTOP_BUTTON_PRESSED\r\n", 37);
        }
        return;
    }

    if (g_safety.estop_latched) {
        return;
    }

    /* 2. Check Wireless Communication Heartbeat Timeout */
    uint32_t now = HAL_GetTick();
    if ((now - g_safety.last_heartbeat_ms) > g_safety.timeout_limit_ms) {
        /* Only auto-stop if rover was actively driving */
        if (Motor_GetState() == MOTOR_STATE_RUNNING) {
            g_safety.comm_timeout_occurred = true;
            Motor_Stop();
            Debug_Log("[SAFETY] Wi-Fi communication timeout! Auto-stopped motors.\r\n");
            WiFi_BroadcastTelemetry("WARN:COMM_TIMEOUT_AUTO_STOP\r\n", 29);
        }
    }
}

void Safety_TriggerEmergencyStop(void)
{
    g_safety.estop_latched = true;
    Motor_EmergencyStop();
}

void Safety_ResetEmergencyStop(void)
{
    g_safety.estop_latched = false;
    g_safety.comm_timeout_occurred = false;
    g_safety.last_heartbeat_ms = HAL_GetTick();
    Motor_ResetFault();
    Debug_Log("[SAFETY] E-Stop reset.\r\n");
}

bool Safety_IsEmergencyStopActive(void)
{
    return g_safety.estop_latched;
}

bool Safety_IsTimeoutActive(void)
{
    return g_safety.comm_timeout_occurred;
}
