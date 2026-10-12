# 🏎️ 16_RaceCar_MotorDriver_Bluetooth_Audio

**STM32L073RZ Race Car & Mecanum Rover Demonstrator with Wireless Bluetooth Control**

---

## 📌 1. Project Overview

This project is a complete, real-time embedded demonstrator built around the **STM32 NUCLEO-L073RZ** microcontroller board. It integrates:
1. **Motor Driver Expansion Board**: **ST X-STM32MP-RBT01** (Board code `XMPSRBT01A`), featuring dual **STSPIN948** dual full-bridge drivers to control four independent brushed DC motors on a **Mecanum 4WD chassis**.
2. **Wireless Control**: Dual-mode Bluetooth support featuring the **ST X-NUCLEO-IDB05A2** (BlueNRG-M0 / SPBTLE-1S) expansion shield and standard UART Bluetooth modules (HC-05, HM-10, ESP32).
3. **Mobile & PC Control Interfaces**:
   - **Mobile Phone Web Bluetooth Dashboard** (`tools/web_bluetooth_controller.html`): Open directly in Chrome on Android / iOS (Bluefy) for instant wireless touch driving without installing an app!
   - **PC Desktop Python GUI** (`tools/racecar_controller.py`): Tachometer gauge, throttle sliders, Mecanum strafe pad, and live telemetry log.

---

## 🛠️ 2. Hardware Identification & Specifications

| Component | Exact Model / Order Code | Role & Key Specifications |
|:---|:---|:---|
| **MCU Board** | **ST NUCLEO-L073RZ** | ARM Cortex-M0+ @ 32 MHz, 192 KB Flash, 20 KB SRAM. |
| **Motor Driver Board** | **ST X-STM32MP-RBT01** (`XMPSRBT01A`) | Dual STSPIN948 ICs, 4 Brushed DC Motor outputs, up to 48 V DC, 2.5 A / motor (4.5 A peak), 15 A blade fuse, 330 µF bulk cap. |
| **Bluetooth Shield** | **ST X-NUCLEO-IDB05A2** | Bluetooth Low Energy 4.2 shield (SPBTLE-1S BlueNRG-M0) via SPI1. |
| **Auxiliary Bluetooth** | HC-05 / HM-10 / ESP32 | High-speed UART Bluetooth module via USART1. |
| **Chassis & Wheels** | 4-Wheel Mecanum Rover Chassis | 4 DC gear motors with 45° Mecanum roller wheels. |
| **Battery** | 7.4 V – 12.0 V LiPo / 18650 Pack | Dedicated motor supply connected to X-STM32MP-RBT01 terminal. |

---

## ⚠️ 3. Critical Hardware Safety Rules

> [!CAUTION]
> **READ CAREFULLY BEFORE APPLYING POWER**:
> 1. **SEPARATE VOLTAGES**: NEVER connect the motor battery pack (>5V) to the Nucleo 5V, 3.3V, or any GPIO pin! The motor battery connects ONLY to the 2-pin green screw terminal `CN3` of the X-STM32MP-RBT01.
> 2. **COMMON GROUND**: An explicit, heavy ground wire MUST connect Nucleo `GND` to the X-STM32MP-RBT01 `GND` header before connecting the battery.
> 3. **FAIL-SAFE STARTUP**: The firmware always powers up in `MOTOR_STATE_IDLE` with 0% PWM and holds the four bridge enable lines (`PC4`, `PC5`, `PC6`, `PC8`) LOW until a `START` or drive command is received, and again whenever the car is idle. The RBT01 itself pulls these lines HIGH (bridges enabled), so all four wires must be fitted, and the motor supply should stay off while the Nucleo is being flashed or held in reset.
> 4. **MOTOR TERMINALS**: Each screw block is labelled `2A 1A 1B 2B`. A motor goes on `2A`+`1A` or on `1B`+`2B`, never across `1A`–`1B` or `2A`–`2B`.
> 5. **COMMUNICATION WATCHDOG**: If wireless communication is lost for more than **1.5 seconds**, the firmware automatically executes a safe emergency stop.
> 6. **PHYSICAL EMERGENCY STOP**: Pressing the Blue User Button `B1` (`PC13`) on the Nucleo board instantly triggers a hardware emergency stop.

---

## 🔌 4. Verified Hardware Pinout Table

MCU pins are checked against the STM32L073RZ / Nucleo-64 layout (ST UM1724), and the X-STM32MP-RBT01 header pins against ST UM3482 (Table 11). Full details, including the CN2 pins that must stay unconnected, are in [`hardware/pinout.md`](hardware/pinout.md).

| MCU Pin | Peripheral | Signal Function | Target Board & Pin | Voltage | Direction |
|:---|:---|:---|:---|:---|:---|
| **PB4** | TIM3_CH1 | Motor 1 PWM (FL) | X-STM32MP-RBT01 CN2 Pin 32 | 3.3 V | Output (20 kHz PWM) |
| **PC0** | GPIO_Output | Motor 1 DIR (FL) | X-STM32MP-RBT01 CN2 Pin 23 | 3.3 V | Output |
| **PC4** | GPIO_Output_OD | Motor 1 EN / nFAULT | X-STM32MP-RBT01 CN2 Pin 13 | 3.3 V | Open-drain (LOW = off, read back for fault) |
| **PB5** | TIM3_CH2 | Motor 2 PWM (FR) | X-STM32MP-RBT01 CN2 Pin 33 | 3.3 V | Output (20 kHz PWM) |
| **PC1** | GPIO_Output | Motor 2 DIR (FR) | X-STM32MP-RBT01 CN2 Pin 24 | 3.3 V | Output |
| **PC5** | GPIO_Output_OD | Motor 2 EN / nFAULT | X-STM32MP-RBT01 CN2 Pin 15 | 3.3 V | Open-drain (LOW = off, read back for fault) |
| **PB0** | TIM3_CH3 | Motor 3 PWM (RL) | X-STM32MP-RBT01 CN2 Pin 31 | 3.3 V | Output (20 kHz PWM) |
| **PC2** | GPIO_Output | Motor 3 DIR (RL) | X-STM32MP-RBT01 CN2 Pin 19 | 3.3 V | Output |
| **PC6** | GPIO_Output_OD | Motor 3 EN / nFAULT | X-STM32MP-RBT01 CN2 Pin 16 | 3.3 V | Open-drain (LOW = off, read back for fault) |
| **PB1** | TIM3_CH4 | Motor 4 PWM (RR) | X-STM32MP-RBT01 CN2 Pin 36 | 3.3 V | Output (20 kHz PWM) |
| **PC3** | GPIO_Output | Motor 4 DIR (RR) | X-STM32MP-RBT01 CN2 Pin 21 | 3.3 V | Output |
| **PC8** | GPIO_Output_OD | Motor 4 EN / nFAULT | X-STM32MP-RBT01 CN2 Pin 18 | 3.3 V | Open-drain (LOW = off, read back for fault) |
| **PA2** | USART2_TX | Debug VCP TX | ST-LINK USB Virtual COM Port | 3.3 V | Output (115200 Baud) |
| **PA3** | USART2_RX | Debug VCP RX | ST-LINK USB Virtual COM Port | 3.3 V | Input (115200 Baud) |
| **PA9** | USART1_TX | Aux Bluetooth TX | External BT Module RX | 3.3 V | Output (115200 Baud) |
| **PA10**| USART1_RX | Aux Bluetooth RX | External BT Module TX | 3.3 V | Input (115200 Baud) |
| **PA5** | SPI1_SCK | BlueNRG SPI Clock | X-NUCLEO-IDB05A2 D13 | 3.3 V | Output |
| **PA6** | SPI1_MISO | BlueNRG SPI MISO | X-NUCLEO-IDB05A2 D12 | 3.3 V | Input |
| **PA7** | SPI1_MOSI | BlueNRG SPI MOSI | X-NUCLEO-IDB05A2 D11 | 3.3 V | Output |
| **PA1** | GPIO_Output | BlueNRG Chip Select| X-NUCLEO-IDB05A2 A1 | 3.3 V | Output |
| **PA0** | GPIO_Input | BlueNRG IRQ Line | X-NUCLEO-IDB05A2 A0 | 3.3 V | Input |
| **PA8** | GPIO_Output | BlueNRG Reset Line | X-NUCLEO-IDB05A2 D7 | 3.3 V | Output |
| **PB2** | GPIO_Output | Status Heartbeat LED | Onboard Status Indicator | 3.3 V | Output |
| **PC13**| GPIO_Input | Hardware E-STOP Button | Nucleo Blue Pushbutton B1 | 3.3 V | Input (Active Low) |
| **3V3** | Power Rail | Logic VDD | X-STM32MP-RBT01 Pin 1 | 3.3 V | Power |
| **GND** | Ground Rail | Common Ground | All Boards | 0.0 V | Common Return |

---

## 📱 5. Wireless Mobile Phone & PC Control

### Option 1: Mobile Phone Control (Web Bluetooth Dashboard)
No app installation required!
1. Open Google Chrome on your Android phone, tablet, or PC (or the "Bluefy" browser on iOS).
2. Open [`tools/web_bluetooth_controller.html`](file:///c:/Users/choprasu/Desktop/GrassKit_Rover_RandD/16_RaceCar_MotorDriver_Bluetooth_Audio/tools/web_bluetooth_controller.html).
3. Tap **CONNECT** and pair with the car.
4. Drive using the virtual on-screen touch pad (Forward, Reverse, Strafe Left, Strafe Right, Rotate Left, Rotate Right), change gears, rev the engine, or trigger turbo boost!

### Option 2: Mobile Terminal App (Android / iOS)
Use apps such as **Serial Bluetooth Terminal** or **nRF Connect**:
- Connect to your Bluetooth module at **115200 baud**.
- Send commands: `START`, `THROTTLE:50`, `STRAFE:60`, `REV`, `STATUS`, `ESTOP`.

### Option 3: Desktop Python Controller
1. Connect via USB or Bluetooth COM port.
2. Run:
   ```bash
   python tools/racecar_controller.py
   ```
3. Use the graphical tachometer, sliders, and directional pad.

---

## 💻 6. Building & Flashing Firmware

### Project Structure
```text
16_RaceCar_MotorDriver_Bluetooth_Audio/
├── README.md
├── docs/                      # Architectural specifications
│   ├── hardware.md
│   ├── wiring.md
│   ├── bluetooth_protocol.md
│   └── motor_control.md
├── hardware/                  # Physical connection diagrams & pinout
│   ├── pinout.md
│   ├── block_diagram.md
│   └── wiring_diagram.md
├── firmware/                  # Complete STM32CubeIDE project
│   ├── Core/
│   │   ├── Inc/               # Application & driver headers
│   │   ├── Src/               # Modular C implementations
│   │   └── Startup/           # startup_stm32l073rztx.s
│   ├── Drivers/               # STM32L0xx HAL and CMSIS
│   ├── STM32CubeMX/           # CubeMX .ioc configuration
│   ├── STM32L073RZTx_FLASH.ld # Linker script
│   └── .project / .cproject   # Eclipse / STM32CubeIDE project files
├── bluetooth/                 # Protocol docs & test commands
└── tools/                     # Python GUI and Web Bluetooth app
```

### Option A: Open in STM32CubeIDE
1. Launch **STM32CubeIDE**.
2. Click **File -> Open Projects from File System...**
3. Select the folder: `16_RaceCar_MotorDriver_Bluetooth_Audio/firmware`.
4. Click **Build Project** (Hammer icon).
5. Click **Run / Debug** to flash to your Nucleo-L073RZ.

### Option B: Command-Line GCC Compilation
Using the installed ARM GNU toolchain in STM32CubeIDE:
```powershell
$gcc = (Get-ChildItem -Path "C:\ST\STM32CubeIDE_2.2.0" -Recurse -Filter "arm-none-eabi-gcc.exe").FullName | Select -First 1
$fw = "c:\Users\choprasu\Desktop\GrassKit_Rover_RandD\16_RaceCar_MotorDriver_Bluetooth_Audio\firmware"
& "$gcc" -mcpu=cortex-m0plus -mthumb -O2 -I"$fw\Core\Inc" -I"$fw\Drivers\STM32L0xx_HAL_Driver\Inc" -I"$fw\Drivers\CMSIS\Device\ST\STM32L0xx\Include" -I"$fw\Drivers\CMSIS\Include" -DSTM32L073xx -DUSE_HAL_DRIVER -T"$fw\STM32L073RZTx_FLASH.ld" "$fw\Core\Src\*.c" "$fw\Core\Startup\startup_stm32l073rztx.s" "$fw\Drivers\STM32L0xx_HAL_Driver\Src\*.c" -o "$fw\racecar_demo.elf"
```

---

## 🧪 7. Staged 15-Step Test Procedure

| Step | Action | Expected Result | Pass/Fail |
|:---:|:---|:---|:---:|
| **1** | Power Nucleo via USB | ST-LINK LED turns GREEN; Status LED (PB2) flashes at 1 Hz | [ ] |
| **2** | Open Serial Terminal @ 115200 | Boot banner and `STATUS:BOOT_OK` printed | [ ] |
| **3** | Pair Bluetooth module | Terminal or Web app connects successfully | [ ] |
| **4** | Send `HELP` and `STATUS` | Valid command list and telemetry returned | [ ] |
| **5** | Send `REV`, then `STATUS` | Reported `RPM` jumps above the 900 idle value | [ ] |
| **6** | Verify TIM3 PWM with multimeter/oscilloscope | PB4, PB5, PB0, PB1 show 20 kHz PWM proportional to throttle | [ ] |
| **7** | Place car on test stand (wheels off ground) | Chassis secure and free from mechanical obstruction | [ ] |
| **8** | Connect Motor Battery to RBT01 `CN3` | No wheel moves at any supply voltage while idle; no smoke or excessive heat | [ ] |
| **8a**| Send `MOTOR:1,25`, `MOTOR:2,25`, `MOTOR:3,25`, `MOTOR:4,25` one at a time (`STOP` in between) | Only FL, then FR, then RL, then RR turns | [ ] |
| **9** | Send `START` then `THROTTLE:20` | All 4 wheels spin forward at low speed | [ ] |
| **10**| Send `THROTTLE:0` | Wheels gracefully decelerate to a stop | [ ] |
| **11**| Test Steering: `STEER:40` | Left wheels spin faster than right wheels | [ ] |
| **12**| Test Mecanum Strafe: `MODE:MECANUM`, `STRAFE:50` | Wheels spin in diagonal pairs (lateral motion) | [ ] |
| **13**| Test Shifting: `GEAR:UP`, `GEAR:DOWN` | `STATUS` reports the new `GEAR` value | [ ] |
| **14**| Test Communication Timeout: Stop sending commands for 2 s | Firmware executes `WARN:COMM_TIMEOUT_AUTO_STOP` | [ ] |
| **15**| Test Emergency Stop: Press Blue Button B1 or send `ESTOP` | Motors instantly halt, PWM zeroes, driver disables | [ ] |

---

## ❓ 8. Known Limitations & Hardware Checklist

- **Board Jumpers**: The mode jumpers J6 / J7 / J8 (driver U2) and J11 / J12 / J13 (driver U3) must be at their factory positions `1-2 / 2-3 / 1-2` (dual independent full-bridge mode). The firmware's PWM + direction + enable scheme only matches that mode.
- **Current Limit Trimmers**: Set R51 (M1), R55 (M2), R71 (M3) and R76 (M4) so the current limit matches your DC motor stall current rating: $I_{\text{peak}} = V_{\text{REF}} / 0.5\ \Omega$.
- **Step 6 needs the motor supply**: With the RBT01 connected but its motor supply off, the drivers sit in under-voltage lockout and report a fault as soon as the bridges are enabled (`ALARM:DRIVER_FAULT`), which zeroes the PWM outputs.
- **Not yet bench-verified**: The pin map above was derived from ST's UM3482 / DS14271 documents and compiles cleanly, but has not been run on the hardware. Do steps 7–9 with the wheels off the ground.
- **Mecanum Wheel Orientation**: Ensure wheels are mounted in the standard Mecanum "O-pattern" (top rollers point toward the center of the chassis) for correct omnidirectional motion.
