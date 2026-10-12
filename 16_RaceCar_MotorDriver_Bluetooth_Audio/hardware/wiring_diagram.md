# Physical Wiring & Interconnection Guide

This document describes the exact wire-by-wire interconnections required to build and bench-test the race car demonstrator.

---

## 1. Power Supply Wiring

| Source | Wire Color | Connected Destination | Specification / Notes |
|:---|:---|:---|:---|
| **Motor Battery (+)** | RED (Heavy gauge, 18-20 AWG) | X-STM32MP-RBT01 2-pin Screw Terminal `CN3` (+) | 7.4 V – 12.0 V (Do NOT connect to Nucleo!) |
| **Motor Battery (-)** | BLACK (Heavy gauge, 18-20 AWG) | X-STM32MP-RBT01 2-pin Screw Terminal `CN3` (−) | Main power return |
| **Nucleo 3V3** | ORANGE / RED | X-STM32MP-RBT01 CN2 Pin 1 | Supplies 3.3V to the motor driver logic and sensors |
| **Nucleo GND** | BLACK | X-STM32MP-RBT01 CN2 Pin 6 | **ESSENTIAL COMMON GROUND** |

---

## 2. Motor Output Wiring (To 4 DC Motors on Mecanum Chassis)

The X-STM32MP-RBT01 board has two green 4-position screw terminal blocks,
`CN4` (driver U2) and `CN5` (driver U3). On **both** blocks the terminals are
labelled, in order, `2A  1A  1B  2B`.

Each STSPIN948 contains full-bridge **A** (outputs 1A + 2A) and full-bridge **B**
(outputs 1B + 2B). One motor goes on each bridge, so on each block one motor
takes the first two terminals and the other motor takes the last two:

### Terminal Block CN4 (driver U2): Motors 1 and 2
- **Pin 1 (`2A`)** + **Pin 2 (`1A`)**: Front-Left Motor (M1, bridge A)
- **Pin 3 (`1B`)** + **Pin 4 (`2B`)**: Front-Right Motor (M2, bridge B)

### Terminal Block CN5 (driver U3): Motors 3 and 4
- **Pin 1 (`2A`)** + **Pin 2 (`1A`)**: Rear-Left Motor (M3, bridge A)
- **Pin 3 (`1B`)** + **Pin 4 (`2B`)**: Rear-Right Motor (M4, bridge B)

> [!CAUTION]
> Do **not** wire a motor between `1A` and `1B` (the two middle terminals) or
> between `2A` and `2B` (the two outer terminals). Those pairs belong to two
> different bridges, so the motor would be driven by the *difference* between
> two unrelated motor channels.

If a wheel turns the wrong way during the bench test, swap that motor's two
leads within its own pair.

---

## 3. Control Signal Wiring (Nucleo to X-STM32MP-RBT01)

Connect Dupont jumper wires from the Nucleo Morpho/Arduino pins to the 40-pin
header `CN2` on the X-STM32MP-RBT01. Find CN2 pin 1 by its square solder pad;
odd pins share its row. Nucleo connector positions are from ST UM1724 (Table 30).

| Signal | Nucleo-L073RZ Pin | X-STM32MP-RBT01 CN2 Pin | Wire Label |
|:---|:---|:---|:---|
| **Motor 1 PWM** | PB4 (Morpho CN10.27 / Arduino D5) | Pin 32 (`STSPIN948_1_PWM1A`) | M1_PWM |
| **Motor 1 DIR** | PC0 (Morpho CN7.38 / Arduino A5) | Pin 23 (`STSPIN948_1_GPIOA`) | M1_DIR |
| **Motor 1 EN/FAULT** | PC4 (Morpho CN10.34) | Pin 13 (`STSPIN948_1_ENA_nFAULTA`) | M1_EN |
| **Motor 2 PWM** | PB5 (Morpho CN10.29 / Arduino D4) | Pin 33 (`STSPIN948_1_PWM1B`) | M2_PWM |
| **Motor 2 DIR** | PC1 (Morpho CN7.36 / Arduino A4) | Pin 24 (`STSPIN948_1_GPIOB`) | M2_DIR |
| **Motor 2 EN/FAULT** | PC5 (Morpho CN10.6) | Pin 15 (`STSPIN948_1_ENA_nFAULTB`) | M2_EN |
| **Motor 3 PWM** | PB0 (Morpho CN7.34 / Arduino A3) | Pin 31 (`STSPIN948_2_PWM1A`) | M3_PWM |
| **Motor 3 DIR** | PC2 (Morpho CN7.35) | Pin 19 (`STSPIN948_2_GPIOA`) | M3_DIR |
| **Motor 3 EN/FAULT** | PC6 (Morpho CN10.4) | Pin 16 (`STSPIN948_2_ENA_nFAULTA`) | M3_EN |
| **Motor 4 PWM** | PB1 (Morpho CN10.24) | Pin 36 (`STSPIN948_2_PWM1B`) | M4_PWM |
| **Motor 4 DIR** | PC3 (Morpho CN7.37) | Pin 21 (`STSPIN948_2_GPIOB`) | M4_DIR |
| **Motor 4 EN/FAULT** | PC8 (Morpho CN10.2) | Pin 18 (`STSPIN948_2_ENA_nFAULTB`) | M4_EN |

Leave every other CN2 signal pin unconnected. In particular pins 7, 8, 10, 11
and 35 are sensor interrupt outputs and must never be driven by the Nucleo
(see `hardware/pinout.md`).

---

## 4. Bluetooth Wiring Options

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
