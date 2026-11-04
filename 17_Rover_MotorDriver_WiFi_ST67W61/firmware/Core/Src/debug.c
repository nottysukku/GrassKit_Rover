/**
  ******************************************************************************
  * @file           : debug.c
  * @brief          : Serial logging (USART2 VCP) and periodic telemetry.
  ******************************************************************************
  */

#include "debug.h"
#include "motor_control.h"
#include "wifi_st67w61.h"
#include "command_parser.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

UART_HandleTypeDef huart2_debug;
static uint32_t s_telemetry_timer_ms = 0;

/* Ring buffer for USB Serial (USART2 VCP) command input */
#define USB_RX_LINE_MAX               96U
static uint8_t s_uart_rx_byte = 0;
static char s_rx_line_buf[USB_RX_LINE_MAX];
static volatile uint8_t s_rx_idx = 0;
static char s_ready_cmd[USB_RX_LINE_MAX];
static volatile bool s_cmd_pending = false;

void Debug_Init(void)
{
    s_telemetry_timer_ms = 0;
    s_rx_idx = 0;
    s_cmd_pending = false;

    DEBUG_UART_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure PA2 (TX) and PA3 (RX) as USART2 AF4 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DEBUG_UART_TX_PIN | DEBUG_UART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 Base Configuration: 115200 8-N-1 */
    huart2_debug.Instance = DEBUG_UART_INSTANCE;
    huart2_debug.Init.BaudRate = DEBUG_UART_BAUDRATE;
    huart2_debug.Init.WordLength = UART_WORDLENGTH_8B;
    huart2_debug.Init.StopBits = UART_STOPBITS_1;
    huart2_debug.Init.Parity = UART_PARITY_NONE;
    huart2_debug.Init.Mode = UART_MODE_TX_RX;
    huart2_debug.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2_debug.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2_debug.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2_debug.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_UART_Init(&huart2_debug);

    /* Enable USART2 RX interrupt so USB WebApp (web_serial_server.py) works */
    HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    HAL_UART_Receive_IT(&huart2_debug, &s_uart_rx_byte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        char c = (char)s_uart_rx_byte;
        if (c == '\n' || c == '\r')
        {
            if (s_rx_idx > 0 && !s_cmd_pending)
            {
                s_rx_line_buf[s_rx_idx] = '\0';
                memcpy(s_ready_cmd, s_rx_line_buf, s_rx_idx + 1);
                s_cmd_pending = true;
            }
            s_rx_idx = 0;
        }
        else if (s_rx_idx < (USB_RX_LINE_MAX - 1))
        {
            s_rx_line_buf[s_rx_idx++] = c;
        }
        else
        {
            s_rx_idx = 0;
        }

        HAL_UART_Receive_IT(&huart2_debug, &s_uart_rx_byte, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_NEFLAG(huart);
        __HAL_UART_CLEAR_FEFLAG(huart);
        HAL_UART_Receive_IT(&huart2_debug, &s_uart_rx_byte, 1);
    }
}

void Debug_ProcessRx(void)
{
    if (s_cmd_pending)
    {
        char local_cmd[USB_RX_LINE_MAX];
        __disable_irq();
        strncpy(local_cmd, s_ready_cmd, sizeof(local_cmd));
        local_cmd[sizeof(local_cmd) - 1] = '\0';
        s_cmd_pending = false;
        __enable_irq();

        Command_Parse(local_cmd);
    }
}

void Debug_PrintBanner(void)
{
    Debug_Log("\r\n======================================================\r\n");
    Debug_Log(" GRASSKIT ROVER: NUCLEO-L073RZ + X-NUCLEO-67W61M1     \r\n");
    Debug_Log(" Motor Driver  : ST X-STM32MP-RBT01 (Dual STSPIN948)  \r\n");
    Debug_Log(" Wi-Fi Shield  : ST67W611M1 Wi-Fi 6 Coprocessor       \r\n");
    Debug_Log(" Network Mode  : SoftAP (SSID: %s, Port: %d)   \r\n", WIFI_DEFAULT_SSID, WIFI_DEFAULT_PORT);
    Debug_Log(" PWM Drive     : 20 kHz Phase-Interleaved + Soft-Start\r\n");
    Debug_Log(" Control Links : USB Serial (115200) + Wi-Fi web page \r\n");
    Debug_Log("======================================================\r\n\r\n");
}

void Debug_Log(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    /* vsnprintf returns the untruncated length: never send past the buffer */
    if (len >= (int)sizeof(buf))
    {
        len = (int)sizeof(buf) - 1;
    }
    if (len > 0)
    {
        HAL_UART_Transmit(&huart2_debug, (uint8_t *)buf, (uint16_t)len, 50);
    }
}

void Debug_UpdatePeriodic(uint32_t delta_ms)
{
    s_telemetry_timer_ms += delta_ms;

    if (s_telemetry_timer_ms >= SAFETY_TELEMETRY_INTERVAL_MS)
    {
        s_telemetry_timer_ms = 0;

        const char *state_str = "IDLE";
        switch (Motor_GetState())
        {
            case MOTOR_STATE_DISABLED: state_str = "DISABLED"; break;
            case MOTOR_STATE_IDLE:     state_str = "IDLE"; break;
            case MOTOR_STATE_RUNNING:  state_str = "RUNNING"; break;
            case MOTOR_STATE_BRAKING:  state_str = "BRAKING"; break;
            case MOTOR_STATE_FAULT:    state_str = "FAULT"; break;
            case MOTOR_STATE_ESTOP:    state_str = "ESTOP"; break;
        }

        float m1 = Motor_GetMotorSpeed(1);
        float m2 = Motor_GetMotorSpeed(2);
        float m3 = Motor_GetMotorSpeed(3);
        float m4 = Motor_GetMotorSpeed(4);
        float avg_spd = (fabsf(m1) + fabsf(m2) + fabsf(m3) + fabsf(m4)) * 0.25f;
        float virtual_rpm = 900.0f + (avg_spd * 61.0f);

        char t_buf[160];
        int len = snprintf(t_buf, sizeof(t_buf),
                           "TELE:STATE=%s,RPM=%.0f,THROT=%.0f%%,GEAR=1,BOOST=0.00,M1=%.0f,M2=%.0f,M3=%.0f,M4=%.0f,FLT=%u\r\n",
                           state_str,
                           virtual_rpm,
                           fabsf(Motor_GetThrottle()),
                           m1, m2, m3, m4,
                           Motor_GetFaultMask());

        if (len > 0)
        {
            /* Broadcast to USB Serial when running, faulted, or connected */
            if (Motor_GetState() == MOTOR_STATE_RUNNING || Motor_GetState() == MOTOR_STATE_FAULT)
            {
                HAL_UART_Transmit(&huart2_debug, (uint8_t *)t_buf, (uint16_t)len, 25);
            }
            if (WiFi_IsClientConnected())
            {
                WiFi_BroadcastTelemetry(t_buf, (uint16_t)len);
            }
        }
    }
}
