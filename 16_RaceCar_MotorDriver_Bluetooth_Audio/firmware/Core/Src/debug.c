/**
  ******************************************************************************
  * @file           : debug.c
  * @brief          : Serial logging and periodic telemetry implementation.
  ******************************************************************************
  */

#include "debug.h"
#include "bluetooth.h"
#include "motor_control.h"
#include "engine_model.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static uint32_t s_telemetry_timer_ms = 0;

void Debug_Init(void)
{
    s_telemetry_timer_ms = 0;
}

void Debug_PrintBanner(void)
{
    Bluetooth_Send("\r\n======================================================\r\n");
    Bluetooth_Send(" STM32L073RZ RACECAR DEMONSTRATOR                   \r\n");
    Bluetooth_Send(" Motor Driver: ST X-STM32MP-RBT01 (Dual STSPIN948)    \r\n");
    Bluetooth_Send(" Wireless    : X-NUCLEO-IDB05A2 / UART Bluetooth     \r\n");
    Bluetooth_Send(" Mecanum 4WD : Active Holonomic Kinematics           \r\n");
    Bluetooth_Send(" Type 'HELP' for command list.                       \r\n");
    Bluetooth_Send("======================================================\r\n\r\n");
}

void Debug_Log(const char *fmt, ...)
{
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Bluetooth_Send(buf);
}

void Debug_UpdatePeriodic(uint32_t delta_ms)
{
    s_telemetry_timer_ms += delta_ms;

    if (s_telemetry_timer_ms >= SAFETY_TELEMETRY_INTERVAL_MS) {
        s_telemetry_timer_ms = 0;

        /* Only emit periodic status if vehicle is running or connected */
        if (Motor_GetState() == MOTOR_STATE_RUNNING || Bluetooth_IsConnected()) {
            char t_buf[128];
            snprintf(t_buf, sizeof(t_buf),
                     "TELE:RPM=%.0f,THROT=%.0f%%,GEAR=%u,BOOST=%.2f,M1=%.0f,M2=%.0f,M3=%.0f,M4=%.0f\r\n",
                     Engine_GetRPM(),
                     Motor_GetThrottle(),
                     Engine_GetGear(),
                     Engine_GetBoost(),
                     Motor_GetMotorSpeed(0),
                     Motor_GetMotorSpeed(1),
                     Motor_GetMotorSpeed(2),
                     Motor_GetMotorSpeed(3));
            Bluetooth_Send(t_buf);
        }
    }
}
