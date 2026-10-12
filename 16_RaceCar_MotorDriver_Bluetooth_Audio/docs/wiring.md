# Detailed Step-by-Step Wiring & Setup Guide

Follow these steps sequentially to assemble and power up the race car hardware safely.

---

## Step 1: Pre-Power Ground and Voltage Inspection

1. **Verify Battery Voltage**:
   - Ensure the motor battery is rated between **7.4 V (2S LiPo)** and **12.0 V (3S LiPo or SLA)**.
   - Do NOT connect battery yet!
2. **Tie Common Grounds**:
   - Connect a heavy jumper from `GND` on the Nucleo board to `GND` on the X-STM32MP-RBT01 40-pin header (Pin 6).
   - Connect a second ground wire from Nucleo `GND` to MAX98357A `GND`.
   - **Verification**: Use a digital multimeter in continuity/resistance mode to verify that all GND pins read < 0.2 Ω between boards.

---

## Step 2: MAX98357A Amplifier Wiring

1. Connect **VIN** on MAX98357A to **5V** on Nucleo (Arduino header CN6 pin 5).
2. Connect **GND** on MAX98357A to **GND** on Nucleo.
3. Connect **BCLK** to Nucleo **PB13** (Morpho CN10 pin 30).
4. Connect **LRC** (WSEL) to Nucleo **PB12** (Morpho CN10 pin 16).
5. Connect **DIN** to Nucleo **PB15** (Morpho CN10 pin 26).
6. Connect **GAIN** to Nucleo **GND** (sets safe 9 dB gain).
7. Connect the two speaker terminals (`+` and `-`) to an 4 Ω or 8 Ω 3W speaker.

---

## Step 3: X-STM32MP-RBT01 Motor Driver Signal Wiring

Connect the logic control lines from the Nucleo Morpho headers to the 40-pin header of the X-STM32MP-RBT01:

| Signal | Nucleo Pin | RBT01 40-pin Header |
|:---|:---|:---|
| **3.3V Logic Ref** | Nucleo 3V3 | Pin 1 (3V3) |
| **Common GND** | Nucleo GND | Pin 6 (GND) |
| **Driver Enable** | PC4 | Pin 7 (GPIO4) |
| **Motor 1 PWM** | PB4 | Pin 12 (GPIO18) |
| **Motor 1 DIR** | PC0 | Pin 11 (GPIO17) |
| **Motor 2 PWM** | PB5 | Pin 33 (GPIO13) |
| **Motor 2 DIR** | PC1 | Pin 13 (GPIO27) |
| **Motor 3 PWM** | PB0 | Pin 35 (GPIO19) |
| **Motor 3 DIR** | PC2 | Pin 15 (GPIO22) |
| **Motor 4 PWM** | PB1 | Pin 32 (GPIO12) |
| **Motor 4 DIR** | PC3 | Pin 16 (GPIO23) |
| **Fault Line** | PC5 | Pin 29 (GPIO5) |

---

## Step 4: Motor Terminal Wiring

Connect the 4 DC motors to the top screw terminals of the X-STM32MP-RBT01:
- **Terminal J1 (Left)**:
  - Pins 1 & 2: Front-Left Motor
  - Pins 3 & 4: Front-Right Motor
- **Terminal J4 (Right)**:
  - Pins 1 & 2: Rear-Left Motor
  - Pins 3 & 4: Rear-Right Motor

---

## Step 5: Power Sequence Checklist

1. Connect the Nucleo-L073RZ to your PC using a USB cable.
2. Verify that the ST-LINK LED turns GREEN and the Status Heartbeat LED (PB2) flashes at 1 Hz.
3. Verify that the PC serial port opens at 115200 baud and outputs the boot banner.
4. With the car wheels elevated (off the ground on a test stand), connect the motor battery to the bottom-right green terminal of the X-STM32MP-RBT01.
5. Send `START` and test low-speed commands (`THROTTLE:20`) to confirm motor directions before placing the car on the floor.
