# Physical Wiring & Interconnection Guide

This document describes the exact wire-by-wire interconnections required to build and bench-test the race car demonstrator.

---

## 1. Power Supply Wiring

| Source | Wire Color | Connected Destination | Specification / Notes |
|:---|:---|:---|:---|
| **Motor Battery (+)** | RED (Heavy gauge, 18-20 AWG) | X-STM32MP-RBT01 Green Screw Terminal `VIN` | 7.4 V – 12.0 V (Do NOT connect to Nucleo!) |
| **Motor Battery (-)** | BLACK (Heavy gauge, 18-20 AWG) | X-STM32MP-RBT01 Green Screw Terminal `GND` | Main power return |
| **Nucleo 5V** | RED (Standard jumper) | MAX98357A `VIN` Pin | Supplies 5V to Class-D amplifier |
| **Nucleo 3V3** | ORANGE / RED | X-STM32MP-RBT01 40-pin Header Pin 1 | Supplies 3.3V reference to motor driver logic |
| **Nucleo GND** | BLACK | MAX98357A `GND` Pin | Signal ground |
| **Nucleo GND** | BLACK | X-STM32MP-RBT01 40-pin Header Pin 6 | **ESSENTIAL COMMON GROUND** |

---

## 2. Motor Output Wiring (To 4 DC Motors on Mecanum Chassis)

The X-STM32MP-RBT01 board has two green 4-position screw terminal blocks at the top edge:

### Terminal Block J1 (Left): Motors 1 and 2
- **Pin 1 (M1+)**: Front-Left Motor Red Wire
- **Pin 2 (M1-)**: Front-Left Motor Black Wire
- **Pin 3 (M2+)**: Front-Right Motor Red Wire
- **Pin 4 (M2-)**: Front-Right Motor Black Wire

### Terminal Block J4 (Right): Motors 3 and 4
- **Pin 1 (M3+)**: Rear-Left Motor Red Wire
- **Pin 2 (M3-)**: Rear-Left Motor Black Wire
- **Pin 3 (M4+)**: Rear-Right Motor Red Wire
- **Pin 4 (M4-)**: Rear-Right Motor Black Wire

---

## 3. Control Signal Wiring (Nucleo to X-STM32MP-RBT01)

Connect female-to-female (or female-to-male) Dupont jumper wires from Nucleo Morpho/Arduino pins to the 40-pin header on X-STM32MP-RBT01:

| Signal | Nucleo-L073RZ Pin | X-STM32MP-RBT01 Pin | Wire Label |
|:---|:---|:---|:---|
| **Motor 1 PWM** | PB4 (Morpho CN7.19) | 40-Pin Header Pin 12 (GPIO18) | M1_PWM |
| **Motor 1 DIR** | PC0 (Morpho CN7.38) | 40-Pin Header Pin 11 (GPIO17) | M1_DIR |
| **Motor 2 PWM** | PB5 (Morpho CN7.13) | 40-Pin Header Pin 33 (GPIO13) | M2_PWM |
| **Motor 2 DIR** | PC1 (Morpho CN7.36) | 40-Pin Header Pin 13 (GPIO27) | M2_DIR |
| **Motor 3 PWM** | PB0 (Morpho CN10.31)| 40-Pin Header Pin 35 (GPIO19) | M3_PWM |
| **Motor 3 DIR** | PC2 (Morpho CN7.34) | 40-Pin Header Pin 15 (GPIO22) | M3_DIR |
| **Motor 4 PWM** | PB1 (Morpho CN10.29)| 40-Pin Header Pin 32 (GPIO12) | M4_PWM |
| **Motor 4 DIR** | PC3 (Morpho CN7.32) | 40-Pin Header Pin 16 (GPIO23) | M4_DIR |
| **Driver Enable** | PC4 (Morpho CN10.34)| 40-Pin Header Pin 7 (GPIO4)   | DRV_EN |
| **Driver Fault**  | PC5 (Morpho CN10.6) | 40-Pin Header Pin 29 (GPIO5)  | DRV_FAULT |

---

## 4. Audio Wiring (Nucleo to MAX98357A & Speaker)

| MAX98357A Pin | Nucleo-L073RZ Pin | Notes |
|:---|:---|:---|
| **VIN** | Nucleo 5V (CN6.5) | Power rail |
| **GND** | Nucleo GND (CN6.6) | Common ground |
| **BCLK** | PB13 (CN10.30) | I2S Bit Clock |
| **LRC** | PB12 (CN10.16) | I2S Word Select / Left-Right Clock |
| **DIN** | PB15 (CN10.26) | I2S Serial PCM Data |
| **GAIN** | Nucleo GND | Configures 9 dB amplifier gain |
| **SD_MODE** | Unconnected | Default stereo downmix to mono |
| **SPK (+)** | Speaker (+) Terminal | 4 Ω or 8 Ω (3 W) Speaker |
| **SPK (-)** | Speaker (-) Terminal | 4 Ω or 8 Ω (3 W) Speaker |

> [!WARNING]
> Never connect either speaker terminal (`+` or `-`) to ground or power. The MAX98357A uses a Bridge-Tied Load (BTL) output that switches both speaker lines at high frequency.

---

## 5. Bluetooth Wiring Options

### Option A: ST X-NUCLEO-IDB05A2 Expansion Shield
- Simply stack the X-NUCLEO-IDB05A2 board directly on top of the Nucleo-L073RZ board via the Arduino UNO R3 headers.
- SPI1 and control lines (D13, D12, D11, A1, A0, D7) align automatically.

### Option B: External UART Bluetooth Module (HC-05, HM-10, ESP32)
| BT Module Pin | Nucleo-L073RZ Pin | Notes |
|:---|:---|:---|
| **VCC** | Nucleo 5V (or 3V3) | Check module voltage requirement (HC-05 needs 5V) |
| **GND** | Nucleo GND | Common Ground |
| **TXD** | PA10 (USART1_RX, CN10.33) | BT Transmit -> STM32 Receive |
| **RXD** | PA9 (USART1_TX, CN10.21)  | STM32 Transmit -> BT Receive (3.3V safe) |
