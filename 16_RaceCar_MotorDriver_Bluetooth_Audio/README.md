# 🏎️ 16_RaceCar_MotorDriver_Bluetooth_Audio

**STM32L073RZ Race Car & Mecanum Rover Demonstrator with Real-Time Procedural V8 Engine Audio Synthesis and Wireless Bluetooth Control**

---

## 📌 1. Project Overview

This project is a complete, real-time embedded demonstrator built around the **STM32 NUCLEO-L073RZ** microcontroller board. It integrates:
1. **Motor Driver Expansion Board**: **ST X-STM32MP-RBT01** (Board code `XMPSRBT01A`), featuring dual **STSPIN948** dual full-bridge drivers to control four independent brushed DC motors on a **Mecanum 4WD chassis**.
2. **Audio System**: **MAX98357A** I2S Class-D mono amplifier driving a loudspeaker with a **real-time procedural V8 racing engine sound synthesizer** (idle, acceleration roar, deceleration burble, 6-speed gearbox shifts, turbocharger blow-off valve hiss, and rev limiter bounces).
3. **Wireless Control**: Dual-mode Bluetooth support featuring the **ST X-NUCLEO-IDB05A2** (BlueNRG-M0 / SPBTLE-1S) expansion shield and standard UART Bluetooth modules (HC-05, HM-10, ESP32).
4. **Mobile & PC Control Interfaces**:
   - **Mobile Phone Web Bluetooth Dashboard** (`tools/web_bluetooth_controller.html`): Open directly in Chrome on Android / iOS (Bluefy) for instant wireless touch driving without installing an app!
   - **PC Desktop Python GUI** (`tools/racecar_controller.py`): Tachometer gauge, throttle sliders, Mecanum strafe pad, and live telemetry log.

---

## 🛠️ 2. Hardware Identification & Specifications

| Component | Exact Model / Order Code | Role & Key Specifications |
|:---|:---|:---|
| **MCU Board** | **ST NUCLEO-L073RZ** | ARM Cortex-M0+ @ 32 MHz, 192 KB Flash, 20 KB SRAM. |
| **Motor Driver Board** | **ST X-STM32MP-RBT01** (`XMPSRBT01A`) | Dual STSPIN948 ICs, 4 Brushed DC Motor outputs, up to 48 V DC, 2.5 A / motor (4.5 A peak), 15 A blade fuse, 330 µF bulk cap. |
| **Audio Amplifier** | **Maxim MAX98357A** | 3.2 W Class-D I2S Digital Audio Amplifier, filterless BTL output, 5 V supply. |
| **Loudspeaker** | 4 Ω or 8 Ω (3 W to 5 W) | Connected to MAX98357A BTL output terminals. |
| **Bluetooth Shield** | **ST X-NUCLEO-IDB05A2** | Bluetooth Low Energy 4.2 shield (SPBTLE-1S BlueNRG-M0) via SPI1. |
| **Auxiliary Bluetooth** | HC-05 / HM-10 / ESP32 | High-speed UART Bluetooth module via USART1. |
| **Chassis & Wheels** | 4-Wheel Mecanum Rover Chassis | 4 DC gear motors with 45° Mecanum roller wheels. |
| **Battery** | 7.4 V – 12.0 V LiPo / 18650 Pack | Dedicated motor supply connected to X-STM32MP-RBT01 terminal. |

---

## ⚠️ 3. Critical Hardware Safety Rules

> [!CAUTION]
> **READ CAREFULLY BEFORE APPLYING POWER**:
> 1. **SEPARATE VOLTAGES**: NEVER connect the motor battery pack (>5V) to the Nucleo 5V, 3.3V, or any GPIO pin! The motor battery connects ONLY to the bottom-right green terminal of the X-STM32MP-RBT01.
> 2. **COMMON GROUND**: An explicit, heavy ground wire MUST connect Nucleo `GND` to the X-STM32MP-RBT01 `GND` header before connecting the battery.
> 3. **SPEAKER CONNECTIONS**: Never ground either speaker terminal of the MAX98357A. It uses a high-frequency differential Bridge-Tied Load (BTL).
> 4. **FAIL-SAFE STARTUP**: The firmware always powers up in `MOTOR_STATE_IDLE` with 0% PWM. The motor driver enable line (`PC4`) is held LOW until an explicit `START` command is received.
> 5. **COMMUNICATION WATCHDOG**: If wireless communication is lost for more than **1.5 seconds**, the firmware automatically executes a safe emergency stop.
> 6. **PHYSICAL EMERGENCY STOP**: Pressing the Blue User Button `B1` (`PC13`) on the Nucleo board instantly triggers a hardware emergency stop.

---

## 🔌 4. Verified Hardware Pinout Table

All pins are conflict-free and verified against the STM32L073RZ datasheet and Nucleo-64 layout:

| MCU Pin | Peripheral | Signal Function | Target Board & Pin | Voltage | Direction |
|:---|:---|:---|:---|:---|:---|
| **PB4** | TIM3_CH1 | Motor 1 PWM (FL) | X-STM32MP-RBT01 Header Pin 12 | 3.3 V | Output (20 kHz PWM) |
| **PC0** | GPIO_Output | Motor 1 DIR (FL) | X-STM32MP-RBT01 Header Pin 11 | 3.3 V | Output |
| **PB5** | TIM3_CH2 | Motor 2 PWM (FR) | X-STM32MP-RBT01 Header Pin 33 | 3.3 V | Output (20 kHz PWM) |
| **PC1** | GPIO_Output | Motor 2 DIR (FR) | X-STM32MP-RBT01 Header Pin 13 | 3.3 V | Output |
| **PB0** | TIM3_CH3 | Motor 3 PWM (RL) | X-STM32MP-RBT01 Header Pin 35 | 3.3 V | Output (20 kHz PWM) |
| **PC2** | GPIO_Output | Motor 3 DIR (RL) | X-STM32MP-RBT01 Header Pin 15 | 3.3 V | Output |
| **PB1** | TIM3_CH4 | Motor 4 PWM (RR) | X-STM32MP-RBT01 Header Pin 32 | 3.3 V | Output (20 kHz PWM) |
| **PC3** | GPIO_Output | Motor 4 DIR (RR) | X-STM32MP-RBT01 Header Pin 16 | 3.3 V | Output |
| **PC4** | GPIO_Output | Driver Enable (EN) | X-STM32MP-RBT01 Header Pin 7 | 3.3 V | Output (Active High) |
| **PC5** | GPIO_Input | Driver Fault Line | X-STM32MP-RBT01 Header Pin 29 | 3.3 V | Input (Active Low) |
| **PB12**| SPI2_NSS | I2S2_WS (Word Select) | MAX98357A LRC / WSEL | 3.3 V | Output (16 kHz Frame) |
| **PB13**| SPI2_SCK | I2S2_CK (Bit Clock) | MAX98357A BCLK | 3.3 V | Output (512 kHz Clock) |
| **PB15**| SPI2_MOSI| I2S2_SD (Serial Data)| MAX98357A DIN | 3.3 V | Output (16-bit PCM) |
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
| **5V**  | Power Rail | Main 5V Rail | MAX98357A VIN | 5.0 V | Power |
| **3V3** | Power Rail | Logic VDD | X-STM32MP-RBT01 Pin 1 | 3.3 V | Power |
| **GND** | Ground Rail | Common Ground | All Boards | 0.0 V | Common Return |

---

## 🔊 5. Real-Time Race Car Audio Synthesis

Rather than playing static audio clips, the STM32L073RZ executes a lightweight, **real-time procedural synthesis algorithm** optimized for the Cortex-M0+ core using fixed-point math:

1. **Combustion Firing Frequency**:
   $$F_c = \frac{\text{RPM}}{60} \times 4 \quad (60 \text{ Hz @ 900 RPM Idle } \longrightarrow 480 \text{ Hz @ 7200 RPM Redline})$$
2. **Asymmetric Exhaust Pressure Pulse**: Generated from a 256-point lookup table modeling supersonic exhaust valve shockwaves and manifold expansion.
3. **Crankshaft & Manifold Harmonics**: Sub-bass rumble ($0.5 \times F_c$) and intake manifold resonance ($2.0 \times F_c$) stacked in real time.
4. **Intake Induction Growl**: Modulated pseudorandom turbulence blended under heavy throttle load.
5. **Overrun Pops & Backfire Crackles**: Random transient bursts triggered during engine braking and throttle release.
6. **Turbo Blow-Off Valve**: High-frequency hiss envelope fired when throttle is released under high boost.
7. **Rev Limiter Chop**: 16 Hz staccato ignition cut when bouncing off the 7000 RPM redline.
8. **I2S DMA Streaming**: Transmitted over I2S2 to MAX98357A via a 1024-byte double-buffer with **< 3.5% CPU utilization**.

---

## 📱 6. Wireless Mobile Phone & PC Control

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

## 💻 7. Building & Flashing Firmware

### Project Structure
```text
16_RaceCar_MotorDriver_Bluetooth_Audio/
├── README.md
├── docs/                      # Architectural specifications
│   ├── hardware.md
│   ├── wiring.md
│   ├── bluetooth_protocol.md
│   ├── audio_architecture.md
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

## 🧪 8. Staged 15-Step Test Procedure

| Step | Action | Expected Result | Pass/Fail |
|:---:|:---|:---|:---:|
| **1** | Power Nucleo via USB | ST-LINK LED turns GREEN; Status LED (PB2) flashes at 1 Hz | [ ] |
| **2** | Open Serial Terminal @ 115200 | Boot banner and `STATUS:BOOT_OK` printed | [ ] |
| **3** | Pair Bluetooth module | Terminal or Web app connects successfully | [ ] |
| **4** | Send `HELP` and `STATUS` | Valid command list and telemetry returned | [ ] |
| **5** | Send `REV` without motor battery | Speaker emits throaty engine rev surge | [ ] |
| **6** | Verify TIM3 PWM with multimeter/oscilloscope | PB4, PB5, PB0, PB1 show 20 kHz PWM proportional to throttle | [ ] |
| **7** | Place car on test stand (wheels off ground) | Chassis secure and free from mechanical obstruction | [ ] |
| **8** | Connect Motor Battery to RBT01 VIN | Power LED on RBT01 lights up; no smoke or excessive heat | [ ] |
| **9** | Send `START` then `THROTTLE:20` | All 4 wheels spin forward at low speed | [ ] |
| **10**| Send `THROTTLE:0` | Wheels gracefully decelerate to a stop | [ ] |
| **11**| Test Steering: `STEER:40` | Left wheels spin faster than right wheels | [ ] |
| **12**| Test Mecanum Strafe: `MODE:MECANUM`, `STRAFE:50` | Wheels spin in diagonal pairs (lateral motion) | [ ] |
| **13**| Test Shifting: `GEAR:UP`, `GEAR:DOWN` | Audio dips momentarily during shift with exhaust pop | [ ] |
| **14**| Test Communication Timeout: Stop sending commands for 2 s | Firmware executes `WARN:COMM_TIMEOUT_AUTO_STOP` | [ ] |
| **15**| Test Emergency Stop: Press Blue Button B1 or send `ESTOP` | Motors instantly halt, PWM zeroes, driver disables | [ ] |

---

## ❓ 9. Known Limitations & Hardware Checklist

- **`[VERIFY HARDWARE]` Board Jumpers**: Ensure jumpers J1 and J2 on the X-STM32MP-RBT01 are set for standard independent H-bridge drive rather than parallel single-motor configuration.
- **`[VERIFY HARDWARE]` Current Limit Trimpots**: Check trimpots R70 and R71 on the RBT01 board to ensure the current limit reference ($V_{\text{REF}}$) is matched to your specific DC motor stall current rating.
- **Mecanum Wheel Orientation**: Ensure wheels are mounted in the standard Mecanum "O-pattern" (top rollers point toward the center of the chassis) for correct omnidirectional motion.
