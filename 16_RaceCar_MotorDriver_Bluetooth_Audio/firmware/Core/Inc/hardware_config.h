/**
  ******************************************************************************
  * @file           : hardware_config.h
  * @brief          : Pin assignments, peripheral mappings, and hardware safety
  *                   configuration for STM32 NUCLEO-L073RZ Race Car Demonstrator.
  ******************************************************************************
  * Hardware Target:
  *   - MCU: STM32L073RZ (NUCLEO-L073RZ)
  *   - Motor Driver: ST X-STM32MP-RBT01 (Dual STSPIN948, 4 DC Brushed Motors)
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
 * Pin mapping checked against ST UM3482 Rev 1 (Table 2, Table 11, schematics
 * 4/10 and 5/10) and the STSPIN948 datasheet DS14271 Rev 2 (Table 8, Table 9).
 * Wiring from the Nucleo-L073RZ to the 40-pin header (CN2) is documented in
 * hardware/pinout.md and docs/wiring.md.
 *
 * Both drivers run in dual independent full-bridge mode (default jumpers:
 * MODE1 LOW, MODE2 HIGH, MODE3 LOW). Each full-bridge x (A or B) has:
 *   - PWM1x      : speed. LOW = both low-sides on (brake), HIGH = drive
 *   - PHx        : direction (board net "GPIOx")
 *   - EN/nFAULTx : open-drain enable input + fault output. The board pulls it
 *                  HIGH (39k to 3V3), so a bridge is ENABLED unless the MCU
 *                  holds the line LOW. The driver pulls it LOW itself on
 *                  UVLO, overcurrent and thermal shutdown.
 * nSTDBY is tied high on the board: CN2 carries no global enable and no
 * separate fault pin.
 *
 * Motor Channels (CN4 / CN5 terminal order is 2A, 1A, 1B, 2B):
 *   - Motor 1 (M1): Front-Left  (FL) - U2 bridge A - CN4 terminals 2A + 1A
 *   - Motor 2 (M2): Front-Right (FR) - U2 bridge B - CN4 terminals 1B + 2B
 *   - Motor 3 (M3): Rear-Left   (RL) - U3 bridge A - CN5 terminals 2A + 1A
 *   - Motor 4 (M4): Rear-Right  (RR) - U3 bridge B - CN5 terminals 1B + 2B
 *   Never wire a motor 1A-1B or 2A-2B: that straddles two different bridges.
 *
 * Voltage Limits:
 *   - Motor Supply (CN3): outputs stay off below the 5.2 V (typ) UVLO threshold,
 *     48.0 V Max (Typical: 7.4V - 12.0V Battery)
 *   - Logic Supply: 3.3 V from Nucleo 3V3 (CN2 pin 1)
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

/* Motor 1: Front-Left, U2 bridge A
 * (PWM: PB4 / TIM3_CH1 -> CN2.32, PH: PC0 -> CN2.23, EN/nFAULT: PC4 -> CN2.13) */
#define MOTOR1_PWM_GPIO_PORT          GPIOB
#define MOTOR1_PWM_PIN                GPIO_PIN_4
#define MOTOR1_PWM_CHANNEL            TIM_CHANNEL_1
#define MOTOR1_DIR_GPIO_PORT          GPIOC
#define MOTOR1_DIR_PIN                GPIO_PIN_0
#define MOTOR1_EN_PIN                 GPIO_PIN_4

/* Motor 2: Front-Right, U2 bridge B
 * (PWM: PB5 / TIM3_CH2 -> CN2.33, PH: PC1 -> CN2.24, EN/nFAULT: PC5 -> CN2.15) */
#define MOTOR2_PWM_GPIO_PORT          GPIOB
#define MOTOR2_PWM_PIN                GPIO_PIN_5
#define MOTOR2_PWM_CHANNEL            TIM_CHANNEL_2
#define MOTOR2_DIR_GPIO_PORT          GPIOC
#define MOTOR2_DIR_PIN                GPIO_PIN_1
#define MOTOR2_EN_PIN                 GPIO_PIN_5

/* Motor 3: Rear-Left, U3 bridge A
 * (PWM: PB0 / TIM3_CH3 -> CN2.31, PH: PC2 -> CN2.19, EN/nFAULT: PC6 -> CN2.16) */
#define MOTOR3_PWM_GPIO_PORT          GPIOB
#define MOTOR3_PWM_PIN                GPIO_PIN_0
#define MOTOR3_PWM_CHANNEL            TIM_CHANNEL_3
#define MOTOR3_DIR_GPIO_PORT          GPIOC
#define MOTOR3_DIR_PIN                GPIO_PIN_2
#define MOTOR3_EN_PIN                 GPIO_PIN_6

/* Motor 4: Rear-Right, U3 bridge B
 * (PWM: PB1 / TIM3_CH4 -> CN2.36, PH: PC3 -> CN2.21, EN/nFAULT: PC8 -> CN2.18) */
#define MOTOR4_PWM_GPIO_PORT          GPIOB
#define MOTOR4_PWM_PIN                GPIO_PIN_1
#define MOTOR4_PWM_CHANNEL            TIM_CHANNEL_4
#define MOTOR4_DIR_GPIO_PORT          GPIOC
#define MOTOR4_DIR_PIN                GPIO_PIN_3
#define MOTOR4_EN_PIN                 GPIO_PIN_8

/* Bridge EN/nFAULT lines (open-drain, one per motor, all on GPIOC).
 * Released = bridge enabled, driven low = outputs high-Z. */
#define MOTOR_EN_GPIO_PORT            GPIOC
#define MOTOR_EN_ALL_PINS             (MOTOR1_EN_PIN | MOTOR2_EN_PIN | MOTOR3_EN_PIN | MOTOR4_EN_PIN)

/* A released EN/nFAULT line rises through 39k / 10nF on the RBT01: ignore it for
 * this long after enabling, then require repeated low samples to flag a fault */
#define MOTOR_FAULT_BLANKING_MS       10UL
#define MOTOR_FAULT_DEBOUNCE_SAMPLES  3U

/* Acceleration / Deceleration Ramping Parameters */
#define MOTOR_RAMP_STEP_PER_MS        2.0f        /* Max % change per millisecond */
#define MOTOR_DEADBAND_PERCENT        4.0f        /* Deadband below which PWM is 0 */

/* =============================================================================
 * 3. BLUETOOTH CONNECTIVITY
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
 * 4. USER INTERFACE & DIAGNOSTICS (NUCLEO-L073RZ)
 * =============================================================================
 */
#define USER_LED_PORT                 GPIOB
#define USER_LED_PIN                  GPIO_PIN_2  /* Use PB2 as status LED */
#define USER_BUTTON_PORT              GPIOC
#define USER_BUTTON_PIN               GPIO_PIN_13 /* Nucleo Blue User Button B1 */

/* =============================================================================
 * 5. HARDWARE SAFETY & TIMEOUT PARAMETERS
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
