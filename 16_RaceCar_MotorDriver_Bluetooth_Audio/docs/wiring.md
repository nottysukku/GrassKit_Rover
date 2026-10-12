# Detailed Step-by-Step Wiring & Setup Guide

Follow these steps sequentially to assemble and power up the race car hardware safely.

---

## Step 1: Pre-Power Ground and Voltage Inspection

1. **Verify Battery Voltage**:
   - Ensure the motor battery is rated between **7.4 V (2S LiPo)** and **12.0 V (3S LiPo or SLA)**.
   - Do NOT connect battery yet!
2. **Tie Common Grounds**:
   - Connect a heavy jumper from `GND` on the Nucleo board to `GND` on the X-STM32MP-RBT01 40-pin header CN2 (Pin 6).
   - **Verification**: Use a digital multimeter in continuity/resistance mode to verify that all GND pins read < 0.2 Ω between boards.

---

## Step 2: X-STM32MP-RBT01 Motor Driver Signal Wiring

Do this with the motor supply disconnected. Connect the logic control lines from
the Nucleo to the 40-pin header **CN2** of the X-STM32MP-RBT01 (pin 1 has the
square solder pad; odd pins share its row). Pin functions are from ST UM3482
Table 11:

| Signal | Nucleo Pin | Nucleo Connector | RBT01 CN2 Pin |
|:---|:---|:---|:---|
| **3.3V Logic Supply** | Nucleo 3V3 | CN6.4 | Pin 1 (3V3) |
| **Common GND** | Nucleo GND | CN6.6 | Pin 6 (GND) |
| **Motor 1 PWM** | PB4 | CN10.27 / D5 | Pin 32 (`STSPIN948_1_PWM1A`) |
| **Motor 1 DIR** | PC0 | CN7.38 / A5 | Pin 23 (`STSPIN948_1_GPIOA`) |
| **Motor 1 EN/FAULT** | PC4 | CN10.34 | Pin 13 (`STSPIN948_1_ENA_nFAULTA`) |
| **Motor 2 PWM** | PB5 | CN10.29 / D4 | Pin 33 (`STSPIN948_1_PWM1B`) |
| **Motor 2 DIR** | PC1 | CN7.36 / A4 | Pin 24 (`STSPIN948_1_GPIOB`) |
| **Motor 2 EN/FAULT** | PC5 | CN10.6 | Pin 15 (`STSPIN948_1_ENA_nFAULTB`) |
| **Motor 3 PWM** | PB0 | CN7.34 / A3 | Pin 31 (`STSPIN948_2_PWM1A`) |
| **Motor 3 DIR** | PC2 | CN7.35 | Pin 19 (`STSPIN948_2_GPIOA`) |
| **Motor 3 EN/FAULT** | PC6 | CN10.4 | Pin 16 (`STSPIN948_2_ENA_nFAULTA`) |
| **Motor 4 PWM** | PB1 | CN10.24 | Pin 36 (`STSPIN948_2_PWM1B`) |
| **Motor 4 DIR** | PC3 | CN7.37 | Pin 21 (`STSPIN948_2_GPIOB`) |
| **Motor 4 EN/FAULT** | PC8 | CN10.2 | Pin 18 (`STSPIN948_2_ENA_nFAULTB`) |

Nothing else on CN2 gets a wire. Pins 7, 8, 10, 11 and 35 are sensor interrupt
outputs: an MCU output on any of them is a short between two outputs.

All four EN/FAULT wires are required. Each line is pulled high on the RBT01, so
a bridge whose wire is missing stays enabled and cannot be shut off.

---

## Step 3: Motor Terminal Wiring

Connect the 4 DC motors to the two 4-position screw terminals of the
X-STM32MP-RBT01. Both blocks are labelled `2A  1A  1B  2B`:
- **Terminal CN4 (driver U2)**:
  - `2A` & `1A` (pins 1 & 2): Front-Left Motor
  - `1B` & `2B` (pins 3 & 4): Front-Right Motor
- **Terminal CN5 (driver U3)**:
  - `2A` & `1A` (pins 1 & 2): Rear-Left Motor
  - `1B` & `2B` (pins 3 & 4): Rear-Right Motor

Never put one motor across `1A`–`1B` or `2A`–`2B`: those are two different
bridges.

---

## Step 4: Power Sequence Checklist

1. Connect the Nucleo-L073RZ to your PC using a USB cable.
2. Verify that the ST-LINK LED turns GREEN and the Status Heartbeat LED (PB2) flashes at 1 Hz.
3. Verify that the PC serial port opens at 115200 baud and outputs the boot banner.
4. With the car wheels elevated (off the ground on a test stand), connect the motor battery to the 2-pin screw terminal `CN3` of the X-STM32MP-RBT01, observing the +/− marking. No wheel may move at any supply voltage while the firmware is idle.
5. Test one wheel at a time (`MOTOR:1,25` … `MOTOR:4,25`, then `STOP`) to confirm each channel reaches the intended wheel, then send `START` and `THROTTLE:20` to confirm motor directions before placing the car on the floor.

If `ALARM:DRIVER_FAULT:M1=..,M2=..,M3=..,M4=..` appears, the listed bridges are
holding their `EN/nFAULT` line low. All four at once almost always means the
motor supply is missing or below 5.2 V, or the RBT01 has no 3V3 / the EN wires
are not connected. Fix the cause, then send `RESET`.
