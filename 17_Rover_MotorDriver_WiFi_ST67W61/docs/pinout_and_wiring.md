# Hardware Pinout & Wiring Specification
**Project: GrassKit Rover — STM32 NUCLEO-L073RG + X-NUCLEO-67W61M1 + X-STM32MP-RBT01**

---

## 1. System Architecture Overview

The system consists of three hardware layers:

```
+------------------------------------------------------------------------+
|                      X-NUCLEO-67W61M1 Shield                           |
|       (ST67W611M1 Wi-Fi 6 + BLE 5.4 Network Coprocessor)               |
+------------------------------------------------------------------------+
                                  || Plugs directly into Arduino Headers
+------------------------------------------------------------------------+
|                       NUCLEO-L073RG Baseboard                          |
|             (STM32L073RZT6 Cortex-M0+ 32 MHz, 192KB Flash)             |
+------------------------------------------------------------------------+
          || Jumper Wires from Morpho Headers CN7 & CN10
          \/
+------------------------------------------------------------------------+
|                     X-STM32MP-RBT01 Motor Driver                       |
|         (Dual STSPIN948 Drivers, 4x Brushed DC Motors, CN2 Header)     |
+------------------------------------------------------------------------+
```

---

## 2. Shield Integration & Conflict Resolution

### 2.1 What the X-NUCLEO-67W61M1 Uses
When the `X-NUCLEO-67W61M1` shield is seated onto the NUCLEO-L073RG Arduino Uno V3 sockets, it connects to the following MCU pins:

| Shield Signal | Arduino Pin | Nucleo-64 Pin | Description |
|:---|:---|:---|:---|
| **SPI_SCK** | D13 | **PA5** | SPI1 Serial Clock (up to 32 MHz) |
| **SPI_MISO** | D12 | **PA6** | SPI1 Master In Slave Out |
| **SPI_MOSI** | D11 | **PA7** | SPI1 Master Out Slave In |
| **SPI_CS** | D10 | **PB6** | SPI1 Chip Select (NSS, active LOW) |
| **SPI_RDY** | D3 | **PB3** | Coprocessor Data Ready / Interrupt Handshake (EXTI3) |
| **CHIP_EN** | D5 | **PB4** | Coprocessor Power / Hardware Enable (Active HIGH) |
| **BOOT** | D6 | **PB10** | Coprocessor Boot Mode Select (Logic LOW = Normal Run) |
| **+3V3 / +5V / GND** | Power | CN6 Header | Power Supply Rails from Nucleo |

### 2.2 Pin Conflict Identified & Solved!
> [!WARNING]
> In typical legacy setups, `PB4` (Arduino D5) was assigned to `Motor 1 PWM`. Because the `X-NUCLEO-67W61M1` shield physically hardwires `D5` (`PB4`) to the coprocessor's `CHIP_EN` line, pulsing PWM on PB4 would reset or kill power to the Wi-Fi coprocessor while giving errant signals to the motor driver.

**The Solution:**
1. **Motor 1 PWM is remapped to `PC6`** using `TIM3_CH1` Alternate Function `AF2`. On the NUCLEO-L073RG, `PC6` is conveniently located on Morpho connector **`CN10 pin 4`**.
2. **Motor 3 EN/FAULT is remapped to `PC9`** on Morpho connector **`CN10 pin 1`**.
3. All four motor PWM channels run synchronously on **`TIM3` (20 kHz ultrasonic PWM)** with zero audible whine and **zero shield conflicts**.

---

## 3. Wiring Table: NUCLEO-L073RG to RBT01 (CN2)

Connect standard female-to-female jumper wires from the **outer Morpho headers (CN7 and CN10)** of the Nucleo-L073RG to the **40-pin header (CN2)** on the `X-STM32MP-RBT01`.

> [!NOTE]
> The outer Morpho double rows remain completely accessible on both sides even with the X-NUCLEO-67W61M1 shield plugged firmly into the inner Arduino sockets.

| Signal Function | Nucleo Pin | Nucleo Morpho Pin | RBT01 CN2 Pin | RBT01 Net Name | Notes |
|:---|:---|:---|:---|:---|:---|
| **Logic Supply (3.3V)** | `3V3` | **CN6.4** or **CN7.16** | **Pin 1** | `+3V3` | Powers RBT01 logic pull-ups |
| **Digital Ground** | `GND` | **CN6.6** or **CN7.20** | **Pin 6** | `GND` | **Mandatory common ground** |
| **Motor 1 PWM** (FL) | `PC6` | **CN10.4** | **Pin 32** | `STSPIN948_1_PWM1A` | TIM3_CH1 (20 kHz PWM) |
| **Motor 1 DIR** (FL) | `PC0` | **CN7.38** | **Pin 23** | `STSPIN948_1_GPIOA` | GPIO Output (Direction) |
| **Motor 1 EN/FAULT** | `PC4` | **CN10.34** | **Pin 13** | `STSPIN948_1_ENA_nFAULTA` | Open-Drain Bidirectional |
| **Motor 2 PWM** (FR) | `PB5` | **CN10.29** (or D4) | **Pin 33** | `STSPIN948_1_PWM1B` | TIM3_CH2 (20 kHz PWM) |
| **Motor 2 DIR** (FR) | `PC1` | **CN7.36** | **Pin 24** | `STSPIN948_1_GPIOB` | GPIO Output (Direction) |
| **Motor 2 EN/FAULT** | `PC5` | **CN10.6** | **Pin 15** | `STSPIN948_1_ENA_nFAULTB` | Open-Drain Bidirectional |
| **Motor 3 PWM** (RL) | `PB0` | **CN7.34** (or A3) | **Pin 31** | `STSPIN948_2_PWM1A` | TIM3_CH3 (20 kHz PWM) |
| **Motor 3 DIR** (RL) | `PC2` | **CN7.35** | **Pin 19** | `STSPIN948_2_GPIOA` | GPIO Output (Direction) |
| **Motor 3 EN/FAULT** | `PC9` | **CN10.1** | **Pin 16** | `STSPIN948_2_ENA_nFAULTA` | Open-Drain Bidirectional |
| **Motor 4 PWM** (RR) | `PB1` | **CN10.24** | **Pin 36** | `STSPIN948_2_PWM1B` | TIM3_CH4 (20 kHz PWM) |
| **Motor 4 DIR** (RR) | `PC3** | **CN7.37** | **Pin 21** | `STSPIN948_2_GPIOB` | GPIO Output (Direction) |
| **Motor 4 EN/FAULT** | `PC8` | **CN10.2** | **Pin 18** | `STSPIN948_2_ENA_nFAULTB` | Open-Drain Bidirectional |

> [!IMPORTANT]
> **Do not connect any other pins on CN2!**
> Pins 7, 8, 10, 11, and 35 on CN2 carry onboard sensor interrupts from the IMU/magnetometer. Connecting MCU outputs there will cause bus contention and short circuits.

---

## 4. RBT01 Motor Screw Terminals (CN4 & CN5)

The X-STM32MP-RBT01 has two 4-position screw terminals for 4 DC brushed motors. Each terminal block is labelled `2A  1A  1B  2B`:

### CN4 (Driver IC U2 - Front Axle):
- **Front-Left Motor (M1)**: Connect to terminals **`2A`** (Pin 1) & **`1A`** (Pin 2).
- **Front-Right Motor (M2)**: Connect to terminals **`1B`** (Pin 3) & **`2B`** (Pin 4).

### CN5 (Driver IC U3 - Rear Axle):
- **Rear-Left Motor (M3)**: Connect to terminals **`2A`** (Pin 1) & **`1A`** (Pin 2).
- **Rear-Right Motor (M4)**: Connect to terminals **`1B`** (Pin 3) & **`2B`** (Pin 4).

> [!CAUTION]
> Never connect a motor across `1A` and `1B` or across `2A` and `2B`. Those terminals belong to two completely separate H-bridges. Doing so can permanently damage the STSPIN948 IC!

---

## 5. Power Supply & Safe Power Sequence

### 5.1 Power Connections
- **Motor Power Supply (CN3)**: 2-pin green screw terminal marked `48V max` and `GND`.
  - Connect your robot battery: **7.4 V (2S LiPo)** to **12.0 V (3S LiPo or SLA)**.
  - Observe polarity (+ to `48V max`, - to `GND`).
- **MCU & Shield Power**: Powered via the ST-LINK USB cable from PC or 5V Power Bank into Nucleo USB connector `CN1`.

### 5.2 Safe Startup Sequence
1. Elevate the rover wheels off the ground on a bench stand.
2. Plug the USB cable into the Nucleo-L073RG board.
3. Open serial console at 115200 baud (ST-LINK VCP) on `PA2/PA3` to verify the boot banner and Wi-Fi initialization.
4. Verify the Status LED (PB2) starts blinking.
5. With the firmware running in `IDLE` state, connect the motor battery to CN3. No wheel should turn while idle.
6. Connect your phone or laptop to the Wi-Fi AP `GrassKit_Rover`, connect to TCP port `8080`, and begin controlling the vehicle.
