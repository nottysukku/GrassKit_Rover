# Hardware Specifications & Board Analysis

This document provides in-depth technical analysis for each component of the race car demonstrator.

---

## 1. Motor Driver Board: ST X-STM32MP-RBT01

- **Board Model Code**: `X-STM32MP-RBT01`
- **Hardware Revision Marking**: `XMPSRBT01A`
- **Original Target Application**: Microprocessor robotics expansion board for STM32MP1 series (STM32MP157F-DK2, STM32MP257F-DK) and Raspberry Pi 40-pin interface.
- **Motor Driver ICs**: Two **STSPIN948** devices.
  - Dual full-bridge motor drivers capable of driving up to 4 Brushed DC motors independently.
  - Integrated N-channel power MOSFETs with low RDS(on).
  - Maximum output voltage rating: **Up to 48 V DC**.
  - Current capability: **Up to 2.5 A continuous per motor** (4.5 A peak r.m.s. per bridge).
  - Built-in adjustable current limiters using shunts `R050` (50 mΩ) and one reference trimmer per bridge (`R51`, `R55`, `R71`, `R76`).
  - Protection features: Under-voltage lockout (UVLO, outputs off below 5.2 V typ), thermal shutdown, overcurrent protection. Faults are reported by pulling the bridge's own open-drain `EN/nFAULT` line low (`PC4`, `PC5`, `PC6`, `PC8`); there is no separate fault pin.
  - Each `EN/nFAULT` line is pulled up on the board, so a bridge is enabled by default and must be held low by the MCU to switch it off.
  - Configured by on-board jumpers (factory default) for dual independent full-bridge mode: per motor one PWM input, one phase (direction) input and one enable/fault line. See `hardware/pinout.md`.
- **Power Terminal & Protection**:
  - Green 2-pin screw terminal marked `48V max` and `GND`.
  - Onboard 15 A blade fuse (`F1`) protecting the battery rail.
  - Large bulk electrolytic capacitor (`330 µF / 50V`) to absorb inductive flyback and maintain bus stability during rapid motor acceleration.
- **Sensors on Board**:
  - `ISM330DHCX`: 6-axis IMU (3D accelerometer + 3D gyroscope).
  - `IIS2MDC`: 3-axis high-accuracy magnetometer.
  - `LPS22HH`: High-precision barometric pressure sensor.
  - `VL53L5CX`: 8x8 multizone Time-of-Flight ranging sensor connector.

---

## 2. Microcontroller: STM32 NUCLEO-L073RZ

- **MCU**: `STM32L073RZT6` (64-pin LQFP)
- **Core**: ARM 32-bit Cortex-M0+ running at 32 MHz with MPU.
- **Memory**:
  - Flash Memory: 192 KB
  - SRAM: 20 KB
  - EEPROM: 6 KB true data EEPROM
- **Key Peripherals Configured**:
  - **TIM3**: 16-bit 4-channel General-Purpose Timer running at 20 kHz ultrasonic PWM for 4 motors.
  - **USART2**: 115200 baud connected to ST-LINK Virtual COM Port for PC console and Python GUI telemetry.
  - **USART1**: 115200 baud connected to PA9/PA10 for external Bluetooth receiver.
  - **SPI1**: 8 MHz bus connected to X-NUCLEO-IDB05A2 BlueNRG-M0 BLE shield.
  - **SysTick**: 1 ms periodic operating system tick for state machine and ramp filters.

---

## 3. Bluetooth Wireless Interfaces

### ST X-NUCLEO-IDB05A2
- Module: `SPBTLE-1S` based on ST BlueNRG-M0 network processor.
- Bluetooth Version: Bluetooth Low Energy (BLE) 4.2 compliant.
- Stack: ST ACI protocol over SPI1.
- Service: Nordic UART Service (NUS) / ST BlueST console profile, compatible with standard Android/iOS BLE terminal apps and the provided Web Bluetooth dashboard.

### Auxiliary UART Bluetooth Module
- Supported: HC-05, HC-06, HM-10, JDY-33, ESP32 BLE UART bridge.
- Interface: USART1 (PA9 TX, PA10 RX) at 115200 baud, 8-N-1.
