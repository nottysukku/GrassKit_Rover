# Hardware Pinout Mapping

This document details the verified pin connections between the **STM32 NUCLEO-L073RZ** microcontroller, the **ST X-STM32MP-RBT01** motor driver board and the **ST X-NUCLEO-IDB05A2** / UART Bluetooth module.

---

## 1. STM32 NUCLEO-L073RZ Pin Allocation Table

| MCU Pin | Connector & Pin | Peripheral / AF | Function / Signal | Connected Target Board / Pin | Logic Voltage | Signal Dir | Safety / Notes |
|:---|:---|:---|:---|:---|:---|:---|:---|
| **PB4** | Morpho CN10.27 / D5 | TIM3_CH1 (AF2) | Motor 1 PWM (FL) | X-STM32MP-RBT01 CN2 Pin 32 (`STSPIN948_1_PWM1A`) | 3.3 V | Output | 20 kHz ultrasonic PWM |
| **PB5** | Morpho CN10.29 / D4 | TIM3_CH2 (AF4) | Motor 2 PWM (FR) | X-STM32MP-RBT01 CN2 Pin 33 (`STSPIN948_1_PWM1B`) | 3.3 V | Output | 20 kHz ultrasonic PWM |
| **PB0** | Morpho CN7.34 / A3 | TIM3_CH3 (AF2) | Motor 3 PWM (RL) | X-STM32MP-RBT01 CN2 Pin 31 (`STSPIN948_2_PWM1A`) | 3.3 V | Output | 20 kHz ultrasonic PWM |
| **PB1** | Morpho CN10.24 / - | TIM3_CH4 (AF2) | Motor 4 PWM (RR) | X-STM32MP-RBT01 CN2 Pin 36 (`STSPIN948_2_PWM1B`) | 3.3 V | Output | 20 kHz ultrasonic PWM |
| **PC0** | Morpho CN7.38 / A5 | GPIO_Output | Motor 1 DIR (FL) | X-STM32MP-RBT01 CN2 Pin 23 (`STSPIN948_1_GPIOA` = PHA) | 3.3 V | Output | Direction (phase) control |
| **PC1** | Morpho CN7.36 / A4 | GPIO_Output | Motor 2 DIR (FR) | X-STM32MP-RBT01 CN2 Pin 24 (`STSPIN948_1_GPIOB` = PHB) | 3.3 V | Output | Direction (phase) control |
| **PC2** | Morpho CN7.35 / - | GPIO_Output | Motor 3 DIR (RL) | X-STM32MP-RBT01 CN2 Pin 19 (`STSPIN948_2_GPIOA` = PHA) | 3.3 V | Output | Direction (phase) control |
| **PC3** | Morpho CN7.37 / - | GPIO_Output | Motor 4 DIR (RR) | X-STM32MP-RBT01 CN2 Pin 21 (`STSPIN948_2_GPIOB` = PHB) | 3.3 V | Output | Direction (phase) control |
| **PC4** | Morpho CN10.34 / - | GPIO_Output (Open-Drain) | Motor 1 EN / nFAULT | X-STM32MP-RBT01 CN2 Pin 13 (`STSPIN948_1_ENA_nFAULTA`) | 3.3 V | Bidirectional | LOW = bridge off (high-Z); released = enabled; read back LOW = driver fault |
| **PC5** | Morpho CN10.6 / - | GPIO_Output (Open-Drain) | Motor 2 EN / nFAULT | X-STM32MP-RBT01 CN2 Pin 15 (`STSPIN948_1_ENA_nFAULTB`) | 3.3 V | Bidirectional | Same as above |
| **PC6** | Morpho CN10.4 / - | GPIO_Output (Open-Drain) | Motor 3 EN / nFAULT | X-STM32MP-RBT01 CN2 Pin 16 (`STSPIN948_2_ENA_nFAULTA`) | 3.3 V | Bidirectional | Same as above |
| **PC8** | Morpho CN10.2 / - | GPIO_Output (Open-Drain) | Motor 4 EN / nFAULT | X-STM32MP-RBT01 CN2 Pin 18 (`STSPIN948_2_ENA_nFAULTB`) | 3.3 V | Bidirectional | Same as above |
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
| **3V3**| Arduino CN6.4 / Power | Power Rail (+3.3V) | Logic VDD | X-STM32MP-RBT01 40-pin Pin 1 | 3.3 V | Power | Logic reference supply |
| **GND**| Arduino / Morpho GND | Common Ground | System GND | ALL BOARDS (RBT01, BT) | 0.0 V | Common | **CRITICAL: ALL GROUNDS MUST BE TIED** |

---

## 2. ST X-STM32MP-RBT01 (XMPSRBT01A) Header Mapping

The board's 40-pin connector is **CN2**. Signal names below are taken from ST
user manual **UM3482 Rev 1** (Table 11 and schematic sheets 3–5 of 10). The
board is designed for an STM32MP1/MP2 Discovery kit, so the header does **not**
follow Raspberry Pi GPIO function names: most pins belong to the on-board
sensors and LEDs.

Find pin 1 by its square solder pad. Odd pins (1, 3 … 39) are in the same row
as pin 1, even pins (2, 4 … 40) in the other row.

| CN2 Pin | RBT01 Signal | Connected Nucleo Pin | Function |
|:---|:---|:---|:---|
| **Pin 1** (or 17) | 3V3 | Nucleo 3V3 | Logic supply for both STSPIN948 and the sensors |
| **Pin 6** (also 9, 14, 20, 25, 30, 34, 39) | GND | Nucleo GND | Common ground reference |
| **Pin 13** | `STSPIN948_1_ENA_nFAULTA` | Nucleo PC4 | Motor 1 enable / fault (open-drain, 39 kΩ pull-up on board) |
| **Pin 15** | `STSPIN948_1_ENA_nFAULTB` | Nucleo PC5 | Motor 2 enable / fault |
| **Pin 16** | `STSPIN948_2_ENA_nFAULTA` | Nucleo PC6 | Motor 3 enable / fault |
| **Pin 18** | `STSPIN948_2_ENA_nFAULTB` | Nucleo PC8 | Motor 4 enable / fault |
| **Pin 19** | `STSPIN948_2_GPIOA` (PHA) | Nucleo PC2 | Motor 3 direction |
| **Pin 21** | `STSPIN948_2_GPIOB` (PHB) | Nucleo PC3 | Motor 4 direction |
| **Pin 23** | `STSPIN948_1_GPIOA` (PHA) | Nucleo PC0 | Motor 1 direction |
| **Pin 24** | `STSPIN948_1_GPIOB` (PHB) | Nucleo PC1 | Motor 2 direction |
| **Pin 31** | `STSPIN948_2_PWM1A` | Nucleo PB0 | Motor 3 PWM speed |
| **Pin 32** | `STSPIN948_1_PWM1A` | Nucleo PB4 | Motor 1 PWM speed |
| **Pin 33** | `STSPIN948_1_PWM1B` | Nucleo PB5 | Motor 2 PWM speed |
| **Pin 36** | `STSPIN948_2_PWM1B` | Nucleo PB1 | Motor 4 PWM speed |

### Pins that must stay unconnected

| CN2 Pin | RBT01 Signal | Why |
|:---|:---|:---|
| 7, 8, 10, 11, 35 | `IIS2MDC_INT_DRDY`, `ISM330DHC_INT1`, `ISM330DHC_INT2`, `LPS22HH_INT_DRDY`, `VL53L5CX_INT` | Sensor interrupt **outputs**. Driving them from an MCU output shorts two outputs together. |
| 12, 22, 26, 29, 37 | `GPIO_LED3`, `GPIO_LED5`, `GPIO_LED4`, `GPIO_LED2`, `GPIO_LED1` | Active-low on-board LEDs (not used by this firmware) |
| 3, 5, 27, 28, 38, 40 | I2C bus, ID EEPROM, ToF reset / LPn | Sensor bus (not used by this firmware) |
| 2, 4 | 5V0 | Not needed; never connect to the motor supply |

### STSPIN948 control logic (dual independent full-bridge mode)

| ENx | PWM1x | PHx | OUT1x | OUT2x | Result |
|:---:|:---:|:---:|:---:|:---:|:---|
| 0 | X | X | High-Z | High-Z | Bridge off, motor coasts |
| 1 | 0 | X | Low-side on | Low-side on | Brake |
| 1 | 1 | 1 | High-side on | Low-side on | Drive, current 1x → 2x |
| 1 | 1 | 0 | Low-side on | High-side on | Drive, current 2x → 1x |

`ENx` is pulled **HIGH on the board**: a bridge is enabled whenever nothing holds
its `EN/nFAULT` line low, and it then drives at full voltage as soon as its
`PWM1x` input is high and the motor supply is above the 5.2 V (typ) UVLO
threshold. The firmware therefore holds all four lines low at idle.

> [!NOTE]
> Board configuration (all defaults are correct for this project):
> - Mode jumpers **J6 / J7 / J8** (driver U2) and **J11 / J12 / J13** (driver U3) at
>   `1-2 / 2-3 / 1-2` = MODE1 LOW, MODE2 HIGH, MODE3 LOW = dual independent
>   full-bridge, fixed OFF-time. J1 only write-protects the ID EEPROM.
> - Current-limit trimmers: **R51** (M1), **R55** (M2), **R71** (M3), **R76** (M4).
>   See `docs/motor_control.md` for the VREF formula.
