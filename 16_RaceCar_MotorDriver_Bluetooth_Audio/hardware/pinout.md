# Hardware Pinout Mapping

This document details the verified pin connections between the **STM32 NUCLEO-L073RZ** microcontroller, the **ST X-STM32MP-RBT01** motor driver board, the **MAX98357A** I2S Class-D audio amplifier, and the **ST X-NUCLEO-IDB05A2** / UART Bluetooth module.

---

## 1. STM32 NUCLEO-L073RZ Pin Allocation Table

| MCU Pin | Connector & Pin | Peripheral / AF | Function / Signal | Connected Target Board / Pin | Logic Voltage | Signal Dir | Safety / Notes |
|:---|:---|:---|:---|:---|:---|:---|:---|
| **PB4** | Morpho CN7.19 / D5 | TIM3_CH1 (AF2) | Motor 1 PWM (FL) | X-STM32MP-RBT01 (40-pin Pin 12 / PWM0) | 3.3 V | Output | 20 kHz ultrasonic PWM |
| **PB5** | Morpho CN7.13 / D4 | TIM3_CH2 (AF2) | Motor 2 PWM (FR) | X-STM32MP-RBT01 (40-pin Pin 33 / PWM1) | 3.3 V | Output | 20 kHz ultrasonic PWM |
| **PB0** | Morpho CN10.31 / A3 | TIM3_CH3 (AF2) | Motor 3 PWM (RL) | X-STM32MP-RBT01 (40-pin Pin 35 / PWM) | 3.3 V | Output | 20 kHz ultrasonic PWM |
| **PB1** | Morpho CN10.29 / - | TIM3_CH4 (AF2) | Motor 4 PWM (RR) | X-STM32MP-RBT01 (40-pin Pin 32 / PWM) | 3.3 V | Output | 20 kHz ultrasonic PWM |
| **PC0** | Morpho CN7.38 / A5 | GPIO_Output | Motor 1 DIR (FL) | X-STM32MP-RBT01 (40-pin Pin 11 / GPIO17) | 3.3 V | Output | Direction control |
| **PC1** | Morpho CN7.36 / A4 | GPIO_Output | Motor 2 DIR (FR) | X-STM32MP-RBT01 (40-pin Pin 13 / GPIO27) | 3.3 V | Output | Direction control |
| **PC2** | Morpho CN7.34 / - | GPIO_Output | Motor 3 DIR (RL) | X-STM32MP-RBT01 (40-pin Pin 15 / GPIO22) | 3.3 V | Output | Direction control |
| **PC3** | Morpho CN7.32 / - | GPIO_Output | Motor 4 DIR (RR) | X-STM32MP-RBT01 (40-pin Pin 16 / GPIO23) | 3.3 V | Output | Direction control |
| **PC4** | Morpho CN10.34 / - | GPIO_Output | Motor Driver EN / STBY | X-STM32MP-RBT01 (40-pin Pin 7 / GPIO4) | 3.3 V | Output | Active HIGH enable, LOW=Standby |
| **PC5** | Morpho CN10.6 / - | GPIO_Input (Pull-Up) | Motor Driver FAULT | X-STM32MP-RBT01 (40-pin Pin 29 / GPIO5) | 3.3 V | Input | Active LOW open-drain fault report |
| **PB12** | Morpho CN10.16 / - | SPI2_NSS (AF0) | I2S2_WS (Word Select / LRC) | MAX98357A LRC / WSEL | 3.3 V | Output | 16 kHz audio frame sync |
| **PB13** | Morpho CN10.30 / - | SPI2_SCK (AF0) | I2S2_CK (Bit Clock) | MAX98357A BCLK | 3.3 V | Output | Audio bit clock (512 kHz) |
| **PB15** | Morpho CN10.26 / - | SPI2_MOSI (AF0) | I2S2_SD (Serial Data) | MAX98357A DIN | 3.3 V | Output | 16-bit PCM audio stream |
| **PA2** | Morpho CN10.35 / D1 | USART2_TX (AF4) | ST-LINK VCP TX | PC USB / Virtual COM Port | 3.3 V | Output | 115200 Baud Console / Telemetry |
| **PA3** | Morpho CN10.37 / D0 | USART2_RX (AF4) | ST-LINK VCP RX | PC USB / Virtual COM Port | 3.3 V | Input | 115200 Baud Command Receiver |
| **PA9** | Morpho CN10.21 / D8 | USART1_TX (AF4) | Aux Bluetooth TX | External BT Module RX (HC-05/HM-10) | 3.3 V | Output | 115200 Baud |
| **PA10** | Morpho CN10.33 / D2 | USART1_RX (AF4) | Aux Bluetooth RX | External BT Module TX (HC-05/HM-10) | 3.3 V | Input | 115200 Baud |
| **PA5** | Arduino CN5.6 / D13 | SPI1_SCK (AF0) | BLE SPI Clock | X-NUCLEO-IDB05A2 D13 / SCK | 3.3 V | Output | 8 MHz SPI bus |
| **PA6** | Arduino CN5.5 / D12 | SPI1_MISO (AF0) | BLE SPI MISO | X-NUCLEO-IDB05A2 D12 / MISO | 3.3 V | Input | BlueNRG MISO |
| **PA7** | Arduino CN5.4 / D11 | SPI1_MOSI (AF0) | BLE SPI MOSI | X-NUCLEO-IDB05A2 D11 / MOSI | 3.3 V | Output | BlueNRG MOSI |
| **PA1** | Arduino CN8.2 / A1 | GPIO_Output | BLE SPI Chip Select | X-NUCLEO-IDB05A2 A1 / CS | 3.3 V | Output | Active LOW SPI CS |
| **PA0** | Arduino CN8.1 / A0 | GPIO_Input (Pull-Down)| BLE Interrupt Request | X-NUCLEO-IDB05A2 A0 / IRQ | 3.3 V | Input | Active HIGH BLE packet ready |
| **PA8** | Arduino CN9.8 / D7 | GPIO_Output | BLE Module Reset | X-NUCLEO-IDB05A2 D7 / RST | 3.3 V | Output | Active LOW hardware reset |
| **PB2** | Morpho CN10.22 / - | GPIO_Output | Status Heartbeat LED | Onboard Status Indicator | 3.3 V | Output | Flashes to indicate run/idle/estop |
| **PC13** | Morpho CN7.23 / B1 | GPIO_Input | Hardware E-STOP Button | Nucleo Blue User Pushbutton B1 | 3.3 V | Input | Active LOW physical safety button |
| **5V** | Arduino CN6.5 / Power | Power Rail (+5V) | Main +5V Supply | MAX98357A VIN | 5.0 V | Power | Sourced from Nucleo 5V regulator |
| **3V3**| Arduino CN6.4 / Power | Power Rail (+3.3V) | Logic VDD | X-STM32MP-RBT01 40-pin Pin 1 | 3.3 V | Power | Logic reference supply |
| **GND**| Arduino / Morpho GND | Common Ground | System GND | ALL BOARDS (RBT01, MAX98357A, BT) | 0.0 V | Common | **CRITICAL: ALL GROUNDS MUST BE TIED** |

---

## 2. ST X-STM32MP-RBT01 (XMPSRBT01A) Header Mapping

The board provides a 40-pin standard Raspberry Pi-compatible header (J...):

| 40-Pin Header Pin | Signal Name | Connected Nucleo Pin | Function |
|:---|:---|:---|:---|
| **Pin 1** | 3.3V VDD | Nucleo 3V3 | Logic supply reference for STSPIN948 & sensors |
| **Pin 6, 9, 14, 20**| GND | Nucleo GND | Common ground reference |
| **Pin 7** | GPIO4 | Nucleo PC4 | STSPIN948 Enable / Standby (Active High) |
| **Pin 11** | GPIO17 | Nucleo PC0 | Motor 1 Direction (M1 DIR) |
| **Pin 12** | GPIO18 (PWM0) | Nucleo PB4 | Motor 1 PWM Speed (M1 PWM) |
| **Pin 13** | GPIO27 | Nucleo PC1 | Motor 2 Direction (M2 DIR) |
| **Pin 15** | GPIO22 | Nucleo PC2 | Motor 3 Direction (M3 DIR) |
| **Pin 16** | GPIO23 | Nucleo PC3 | Motor 4 Direction (M4 DIR) |
| **Pin 29** | GPIO5 | Nucleo PC5 | Motor Driver Fault Line (Active Low Open-Drain) |
| **Pin 32** | GPIO12 (PWM0) | Nucleo PB1 | Motor 4 PWM Speed (M4 PWM) |
| **Pin 33** | GPIO13 (PWM1) | Nucleo PB5 | Motor 2 PWM Speed (M2 PWM) |
| **Pin 35** | GPIO19 (PWM1) | Nucleo PB0 | Motor 3 PWM Speed (M3 PWM) |

> [!NOTE]
> `[VERIFY HARDWARE]` Board Jumper Note:
> The X-STM32MP-RBT01 has configuration jumpers J1, J2, and J15. Verify that jumpers J1/J2 match the PWM/DIR mode rather than parallel bridge mode. Trimpots R70 and R71 set the current limit reference threshold (VREF) for the two STSPIN948 ICs.

---

## 3. MAX98357A Class-D Audio Amplifier Connections

| MAX98357A Pin | Connected Nucleo Pin | Description |
|:---|:---|:---|
| **VIN** | Nucleo +5V (CN6.5) | Amplifier power supply (2.5 V - 5.5 V) |
| **GND** | Nucleo GND (CN6.6) | Power & signal ground |
| **BCLK** | Nucleo PB13 (I2S2_CK) | I2S Bit Clock (512 kHz @ 16 kHz sample rate) |
| **LRC / WSEL** | Nucleo PB12 (I2S2_WS) | Left/Right Frame Clock (16 kHz) |
| **DIN** | Nucleo PB15 (I2S2_SD) | I2S Serial Audio Data |
| **GAIN** | Nucleo GND (or leave open) | Tie to GND for 9 dB gain (recommended for low distortion) |
| **SD_MODE** | Leave Open | Internal 100k pull-up outputs stereo mix (L+R)/2 to mono speaker |
| **+ / - Outputs** | Speaker Terminals | 4 Ω or 8 Ω (3 W) speaker. **Do NOT ground either terminal!** |
