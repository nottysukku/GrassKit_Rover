/**
  ******************************************************************************
  * @file           : hardware_config.h
  * @brief          : Pin assignments, peripheral mappings, and hardware safety
  *                   configuration for STM32 NUCLEO-L073RZ Race Car Demonstrator.
  ******************************************************************************
  * Hardware Target:
  *   - MCU: STM32L073RZ (NUCLEO-L073RZ)
  *   - Motor Driver: ST X-STM32MP-RBT01 (Dual STSPIN948, 4 DC Brushed Motors)
  *   - Audio Amplifier: MAX98357A I2S Mono Class-D Amplifier
  *   - Bluetooth Module: ST X-NUCLEO-IDB05A2 (SPBTLE-1S / BlueNRG-M0) + UART
  *   - Chassis: 4-Wheel Mecanum Drive Chassis
  ******************************************************************************
  */

#ifndef __HARDWARE_CONFIG_H
#define __HARDWARE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l0xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* =============================================================================
 * 1. SYSTEM & CPU CLOCK CONFIGURATION
 * =============================================================================
 */
#define SYSCLOCK_FREQ_HZ              32000000UL  /* 32 MHz CPU Clock (PLL from HSI16) */
#define SYSTICK_FREQ_HZ               1000UL      /* 1 ms tick */

/* =============================================================================
 * 2. MOTOR DRIVER BOARD: ST X-STM32MP-RBT01 (Dual STSPIN948)
 * =============================================================================
 * The X-STM32MP-RBT01 features two STSPIN948 ICs driving 4 Brushed DC motors.
 * 
 * [VERIFY HARDWARE] CONNECTOR NOTE:
 * The board features a 40-pin Raspberry Pi-compatible header (2x20 pins, 2.54mm).
 * Wiring from Nucleo-L073RZ Morpho/Arduino pins to the 40-pin header is documented
 * in hardware/pinout.md and wiring.md.
 *
 * Motor Channels:
 *   - Motor 1 (M1): Front-Left  (FL) - Screw Terminal J1 Pins 1-2
 *   - Motor 2 (M2): Front-Right (FR) - Screw Terminal J1 Pins 3-4
 *   - Motor 3 (M3): Rear-Left   (RL) - Screw Terminal J4 Pins 1-2
 *   - Motor 4 (M4): Rear-Right  (RR) - Screw Terminal J4 Pins 3-4
 *
 * Voltage Limits:
 *   - Motor Supply (VIN): 5.0 V to 48.0 V Max (Typical: 7.4V - 12.0V Battery)
 *   - Logic Supply: 3.3 V from Nucleo 3V3
 *   - Maximum Peak Current per Bridge: 4.5 A (continuous rating: 2.5 A per motor)
 *
 * SAFETY RULE: NEVER CONNECT MOTOR BATTERY DIRECTLY TO NUCLEO 5V OR 3V3!
 * COMMON GROUND: Nucleo GND MUST be wired to X-STM32MP-RBT01 GND terminal/pin.
 * =============================================================================
 */

/* Motor PWM Timer: TIM3 (4 Channels, 20 kHz ultrasonic PWM) */
#define MOTOR_PWM_TIMER               TIM3
#define MOTOR_PWM_TIMER_CLK_ENABLE()  __HAL_RCC_TIM3_CLK_ENABLE()
#define MOTOR_PWM_FREQ_HZ             20000UL     /* 20 kHz (silent, above audible range) */
#define MOTOR_PWM_PERIOD_TICKS        1600UL      /* 32 MHz / 20 kHz = 1600 ticks */
#define MOTOR_PWM_MAX_DUTY            MOTOR_PWM_PERIOD_TICKS

/* Motor 1: Front-Left (PWM: PB4 / TIM3_CH1, DIR: PC0) */
#define MOTOR1_PWM_GPIO_PORT          GPIOB
#define MOTOR1_PWM_PIN                GPIO_PIN_4
#define MOTOR1_PWM_CHANNEL            TIM_CHANNEL_1
#define MOTOR1_DIR_GPIO_PORT          GPIOC
#define MOTOR1_DIR_PIN                GPIO_PIN_0

/* Motor 2: Front-Right (PWM: PB5 / TIM3_CH2, DIR: PC1) */
#define MOTOR2_PWM_GPIO_PORT          GPIOB
#define MOTOR2_PWM_PIN                GPIO_PIN_5
#define MOTOR2_PWM_CHANNEL            TIM_CHANNEL_2
#define MOTOR2_DIR_GPIO_PORT          GPIOC
#define MOTOR2_DIR_PIN                GPIO_PIN_1

/* Motor 3: Rear-Left (PWM: PB0 / TIM3_CH3, DIR: PC2) */
#define MOTOR3_PWM_GPIO_PORT          GPIOB
#define MOTOR3_PWM_PIN                GPIO_PIN_0
#define MOTOR3_PWM_CHANNEL            TIM_CHANNEL_3
#define MOTOR3_DIR_GPIO_PORT          GPIOC
#define MOTOR3_DIR_PIN                GPIO_PIN_2

/* Motor 4: Rear-Right (PWM: PB1 / TIM3_CH4, DIR: PC3) */
#define MOTOR4_PWM_GPIO_PORT          GPIOB
#define MOTOR4_PWM_PIN                GPIO_PIN_1
#define MOTOR4_PWM_CHANNEL            TIM_CHANNEL_4
#define MOTOR4_DIR_GPIO_PORT          GPIOC
#define MOTOR4_DIR_PIN                GPIO_PIN_3

/* Motor Driver Global Enable / Standby Pin (Active High): PC4 */
#define MOTOR_ENABLE_GPIO_PORT        GPIOC
#define MOTOR_ENABLE_PIN              GPIO_PIN_4

/* Motor Driver Fault Input Pin (Active Low open-drain with pull-up): PC5 */
#define MOTOR_FAULT_GPIO_PORT         GPIOC
#define MOTOR_FAULT_PIN               GPIO_PIN_5

/* Acceleration / Deceleration Ramping Parameters */
#define MOTOR_RAMP_STEP_PER_MS        2.0f        /* Max % change per millisecond */
#define MOTOR_DEADBAND_PERCENT        4.0f        /* Deadband below which PWM is 0 */

/* =============================================================================
 * 3. AUDIO SYSTEM: MAX98357A I2S CLASS-D AMPLIFIER
 * =============================================================================
 * The MAX98357A is a 3.2W digital Class-D audio amplifier receiving I2S audio.
 *
 * Connections to Nucleo-L073RZ (I2S2 / SPI2 peripheral):
 *   - BCLK (Bit Clock):   PB13 (I2S2_CK)
 *   - LRC / WS (Word):    PB12 (I2S2_WS)
 *   - DIN (Data In):      PB15 (I2S2_SD)
 *   - VIN:                +5V (from Nucleo 5V pin)
 *   - GND:                Common Ground
 *   - GAIN:               GND (9 dB) or Open (15 dB)
 *   - SD_MODE:            Open (Stereo Mix to Mono)
 *
 * Audio Synthesis Specs:
 *   - Sampling Rate: 16000 Hz (16 kHz, 16-bit Mono/Stereo)
 *   - DMA Ping-Pong Buffer: 256 samples (128 samples per half-buffer)
 * =============================================================================
 */
#define AUDIO_SAMPLE_RATE_HZ          16000UL
#define AUDIO_BUFFER_SIZE_SAMPLES     256         /* Ping-pong: 128 half-buffer */
#define AUDIO_NUM_CHANNELS            2           /* I2S stereo frame format */

#define AUDIO_I2S_INSTANCE            SPI2
#define AUDIO_I2S_CLK_ENABLE()        __HAL_RCC_SPI2_CLK_ENABLE()

#define AUDIO_I2S_WS_PORT             GPIOB
#define AUDIO_I2S_WS_PIN              GPIO_PIN_12
#define AUDIO_I2S_CK_PORT             GPIOB
#define AUDIO_I2S_CK_PIN              GPIO_PIN_13
#define AUDIO_I2S_SD_PORT             GPIOB
#define AUDIO_I2S_SD_PIN              GPIO_PIN_15

/* =============================================================================
 * 4. BLUETOOTH CONNECTIVITY
 * =============================================================================
 * Supported interfaces:
 *   A) ST X-NUCLEO-IDB05A2 BLE Shield (BlueNRG-M0 / SPBTLE-1S via SPI1)
 *   B) External UART Bluetooth Module (HC-05, HM-10, ESP32 on USART1)
 *   C) Debug / PC Virtual COM Port (ST-LINK on USART2)
 * =============================================================================
 */

/* ST-LINK VCP UART: USART2 (115200 Baud, 8-N-1) */
#define DEBUG_UART_INSTANCE           USART2
#define DEBUG_UART_CLK_ENABLE()       __HAL_RCC_USART2_CLK_ENABLE()
#define DEBUG_UART_BAUDRATE           115200UL
#define DEBUG_UART_TX_PORT            GPIOA
#define DEBUG_UART_TX_PIN             GPIO_PIN_2
#define DEBUG_UART_RX_PORT            GPIOA
#define DEBUG_UART_RX_PIN             GPIO_PIN_3

/* Secondary / External Bluetooth UART: USART1 (115200 Baud or 9600 Baud) */
#define BT_UART_INSTANCE              USART1
#define BT_UART_CLK_ENABLE()          __HAL_RCC_USART1_CLK_ENABLE()
#define BT_UART_BAUDRATE              115200UL
#define BT_UART_TX_PORT               GPIOA
#define BT_UART_TX_PIN                GPIO_PIN_9
#define BT_UART_RX_PORT               GPIOA
#define BT_UART_RX_PIN                GPIO_PIN_10

/* X-NUCLEO-IDB05A2 SPI Interface (SPI1 on Arduino UNO R3 headers) */
#define IDB05A2_SPI_INSTANCE          SPI1
#define IDB05A2_SPI_CLK_ENABLE()      __HAL_RCC_SPI1_CLK_ENABLE()
#define IDB05A2_SCK_PORT              GPIOA
#define IDB05A2_SCK_PIN               GPIO_PIN_5  /* Arduino D13 */
#define IDB05A2_MISO_PORT             GPIOA
#define IDB05A2_MISO_PIN              GPIO_PIN_6  /* Arduino D12 */
#define IDB05A2_MOSI_PORT             GPIOA
#define IDB05A2_MOSI_PIN              GPIO_PIN_7  /* Arduino D11 */
#define IDB05A2_CS_PORT               GPIOA
#define IDB05A2_CS_PIN                GPIO_PIN_1  /* Arduino A1 */
#define IDB05A2_IRQ_PORT              GPIOA
#define IDB05A2_IRQ_PIN               GPIO_PIN_0  /* Arduino A0 */
#define IDB05A2_RST_PORT              GPIOA
#define IDB05A2_RST_PIN               GPIO_PIN_8  /* Arduino D7 */

/* =============================================================================
 * 5. USER INTERFACE & DIAGNOSTICS (NUCLEO-L073RZ)
 * =============================================================================
 */
#define USER_LED_PORT                 GPIOB
#define USER_LED_PIN                  GPIO_PIN_2  /* Use PB2 as status LED */
#define USER_BUTTON_PORT              GPIOC
#define USER_BUTTON_PIN               GPIO_PIN_13 /* Nucleo Blue User Button B1 */

/* =============================================================================
 * 6. HARDWARE SAFETY & TIMEOUT PARAMETERS
 * =============================================================================
 */
#define SAFETY_COMM_TIMEOUT_MS        1500UL      /* Stop motor if no command for 1.5s */
#define SAFETY_TELEMETRY_INTERVAL_MS  200UL       /* Send status update every 200ms */
#define SAFETY_WATCHDOG_RELOAD_MS     2000UL      /* Independent Watchdog timeout */

/* Virtual Engine RPM Limits */
#define ENGINE_IDLE_RPM               900.0f
#define ENGINE_MAX_RPM                7200.0f
#define ENGINE_REV_LIMIT_RPM          7000.0f

#ifdef __cplusplus
}
#endif

#endif /* __HARDWARE_CONFIG_H */
