/**
  ******************************************************************************
  * @file           : hardware_config.h
  * @brief          : Pin assignments, peripheral mappings, and hardware safety
  *                   configuration for STM32 NUCLEO-L073RG + X-NUCLEO-67W61M1
  *                   + X-STM32MP-RBT01 Rover.
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
 * Controlled via 40-pin header CN2.
 * 4 Brushed DC motors configured in differential / skid-steer drive.
 *
 * NOTE ON PIN CONFLICT RESOLUTION:
 * PB4 is used as CHIP_EN by the X-NUCLEO-67W61M1 shield.
 * Therefore, MOTOR1_PWM is remapped to PC6 (TIM3_CH1 AF2 on CN10.4).
 * MOTOR3_EN_PIN is moved to PC9 (CN10.1).
 * =============================================================================
 */

/* Motor PWM Timer: TIM3 (4 Channels, 20 kHz ultrasonic PWM) */
#define MOTOR_PWM_TIMER               TIM3
#define MOTOR_PWM_TIMER_CLK_ENABLE()  __HAL_RCC_TIM3_CLK_ENABLE()
#define MOTOR_PWM_FREQ_HZ             20000UL     /* 20 kHz (silent, above human audible range) */
#define MOTOR_PWM_PERIOD_TICKS        1600UL      /* 32 MHz / 20 kHz = 1600 ticks */
#define MOTOR_PWM_MAX_DUTY            MOTOR_PWM_PERIOD_TICKS

/* Motor 1: Front-Left (FL), U2 bridge A
 * PWM: PC6 (TIM3_CH1 AF2 -> Morpho CN10.4 -> RBT01 CN2.32)
 * DIR: PC0 (CN7.38 -> RBT01 CN2.23)
 * EN:  PC4 (CN10.34 -> RBT01 CN2.13) */
#define MOTOR1_PWM_GPIO_PORT          GPIOC
#define MOTOR1_PWM_PIN                GPIO_PIN_6
#define MOTOR1_PWM_CHANNEL            TIM_CHANNEL_1
#define MOTOR1_DIR_GPIO_PORT          GPIOC
#define MOTOR1_DIR_PIN                GPIO_PIN_0
#define MOTOR1_EN_PIN                 GPIO_PIN_4

/* Motor 2: Front-Right (FR), U2 bridge B
 * PWM: PB5 (TIM3_CH2 AF4 -> Morpho CN10.29 -> RBT01 CN2.33)
 * DIR: PC1 (CN7.36 -> RBT01 CN2.24)
 * EN:  PC5 (CN10.6 -> RBT01 CN2.15) */
#define MOTOR2_PWM_GPIO_PORT          GPIOB
#define MOTOR2_PWM_PIN                GPIO_PIN_5
#define MOTOR2_PWM_CHANNEL            TIM_CHANNEL_2
#define MOTOR2_DIR_GPIO_PORT          GPIOC
#define MOTOR2_DIR_PIN                GPIO_PIN_1
#define MOTOR2_EN_PIN                 GPIO_PIN_5

/* Motor 3: Rear-Left (RL), U3 bridge A
 * PWM: PB0 (TIM3_CH3 AF2 -> Morpho CN7.34 -> RBT01 CN2.31)
 * DIR: PC2 (CN7.35 -> RBT01 CN2.19)
 * EN:  PC9 (CN10.1 -> RBT01 CN2.16) */
#define MOTOR3_PWM_GPIO_PORT          GPIOB
#define MOTOR3_PWM_PIN                GPIO_PIN_0
#define MOTOR3_PWM_CHANNEL            TIM_CHANNEL_3
#define MOTOR3_DIR_GPIO_PORT          GPIOC
#define MOTOR3_DIR_PIN                GPIO_PIN_2
#define MOTOR3_EN_PIN                 GPIO_PIN_9

/* Motor 4: Rear-Right (RR), U3 bridge B
 * PWM: PB1 (TIM3_CH4 AF2 -> Morpho CN10.24 -> RBT01 CN2.36)
 * DIR: PC3 (CN7.37 -> RBT01 CN2.21)
 * EN:  PC8 (CN10.2 -> RBT01 CN2.18) */
#define MOTOR4_PWM_GPIO_PORT          GPIOB
#define MOTOR4_PWM_PIN                GPIO_PIN_1
#define MOTOR4_PWM_CHANNEL            TIM_CHANNEL_4
#define MOTOR4_DIR_GPIO_PORT          GPIOC
#define MOTOR4_DIR_PIN                GPIO_PIN_3
#define MOTOR4_EN_PIN                 GPIO_PIN_8

/* Bridge EN/nFAULT lines (open-drain, one per motor, all on GPIOC).
 * Released = bridge enabled, pulled LOW by MCU = bridge disabled. */
#define MOTOR_EN_GPIO_PORT            GPIOC
#define MOTOR_EN_ALL_PINS             (MOTOR1_EN_PIN | MOTOR2_EN_PIN | MOTOR3_EN_PIN | MOTOR4_EN_PIN)

/* RBT01 EN line blanking and debounce:
 * Extended blanking (60 ms) and debounce (30 ms) ride through temporary voltage
 * dips on current-limited bench supplies (e.g. 0.5A limit) during 4-wheel spin-up. */
#define MOTOR_FAULT_BLANKING_MS       60UL
#define MOTOR_FAULT_DEBOUNCE_SAMPLES  30U

/* Motor Ramping, Staggered Start & Slew Rate:
 * - Soft acceleration ramp (0.35% / ms -> ~285 ms for 0..100%) lets Back-EMF build
 *   gradually so 4 motors starting in tandem do not trip a 0.5A bench supply.
 * - Crisp deceleration ramp (2.0% / ms) ensures fast, safe stopping.
 * - All four wheels ramp together and reach their targets at the same moment
 *   (no staggered start), so the rover holds its heading while speeding up. */
#define MOTOR_RAMP_STEP_PER_MS        0.35f       /* Smooth acceleration (% per ms) */
#define MOTOR_RAMP_DOWN_STEP_PER_MS   2.0f        /* Crisp deceleration (% per ms) */
#define MOTOR_DEADBAND_PERCENT        4.0f        /* Minimum deadband threshold */

/* =============================================================================
 * 3. WI-FI EXPANSION BOARD: ST X-NUCLEO-67W61M1 (ST67W611M1)
 * =============================================================================
 * Communication interface: SPI1 + Handshake GPIOs (Arduino Uno V3 Headers)
 * =============================================================================
 */

#define WIFI_SPI_INSTANCE             SPI1
#define WIFI_SPI_CLK_ENABLE()         __HAL_RCC_SPI1_CLK_ENABLE()

/* SPI1 Bus Pins (Standard Arduino Header CN5) */
#define WIFI_SPI_SCK_PORT             GPIOA
#define WIFI_SPI_SCK_PIN              GPIO_PIN_5  /* Arduino D13 */
#define WIFI_SPI_MISO_PORT            GPIOA
#define WIFI_SPI_MISO_PIN             GPIO_PIN_6  /* Arduino D12 */
#define WIFI_SPI_MOSI_PORT            GPIOA
#define WIFI_SPI_MOSI_PIN             GPIO_PIN_7  /* Arduino D11 */
#define WIFI_SPI_CS_PORT              GPIOB
#define WIFI_SPI_CS_PIN               GPIO_PIN_6  /* Arduino D10 */

/* Control & Handshake Pins (Arduino Header CN9) */
#define WIFI_RDY_PORT                 GPIOB
#define WIFI_RDY_PIN                  GPIO_PIN_3  /* Arduino D3 - Input Ready */
#define WIFI_CHIP_EN_PORT             GPIOB
#define WIFI_CHIP_EN_PIN              GPIO_PIN_4  /* Arduino D5 - Output Enable */
#define WIFI_BOOT_PORT                GPIOB
#define WIFI_BOOT_PIN                 GPIO_PIN_10 /* Arduino D6 - Boot Mode (LOW = Run) */

/* Wi-Fi SoftAP Network Defaults */
#define WIFI_DEFAULT_SSID             "GrassKit_Rover"
#define WIFI_DEFAULT_PASSWORD         "GrassKit2026"
#define WIFI_DEFAULT_CHANNEL          1
#define WIFI_DEFAULT_PORT             80      /* web page + plain TCP commands */
#define WIFI_HOST_IP_STR              "192.168.4.1"

/* =============================================================================
 * 4. DEBUG / CONSOLE UART (ST-LINK Virtual COM Port)
 * =============================================================================
 */
#define DEBUG_UART_INSTANCE           USART2
#define DEBUG_UART_CLK_ENABLE()       __HAL_RCC_USART2_CLK_ENABLE()
#define DEBUG_UART_BAUDRATE           115200UL
#define DEBUG_UART_TX_PORT            GPIOA
#define DEBUG_UART_TX_PIN             GPIO_PIN_2
#define DEBUG_UART_RX_PORT            GPIOA
#define DEBUG_UART_RX_PIN             GPIO_PIN_3

/* =============================================================================
 * 5. USER INTERFACE & DIAGNOSTICS (NUCLEO-L073RG)
 * =============================================================================
 */
#define USER_LED_PORT                 GPIOB
#define USER_LED_PIN                  GPIO_PIN_2  /* Status Heartbeat LED */
#define USER_BUTTON_PORT              GPIOC
#define USER_BUTTON_PIN               GPIO_PIN_13 /* Nucleo Blue User Button B1 (E-Stop) */

/* =============================================================================
 * 6. HARDWARE SAFETY & TIMEOUT PARAMETERS
 * =============================================================================
 */
#define SAFETY_COMM_TIMEOUT_MS        1500UL      /* Auto-stop motor if no packet for 1.5s */
#define SAFETY_TELEMETRY_INTERVAL_MS  250UL       /* Transmit telemetry every 250ms */

#ifdef __cplusplus
}
#endif

#endif /* __HARDWARE_CONFIG_H */
