# GrassKit Rover: STM32L073RG + X-NUCLEO-67W61M1 + X-STM32MP-RBT01

Autonomous / Teleoperated 4WD Robot Car running on an **STM32 NUCLEO-L073RG** baseboard, stacked with an **X-NUCLEO-67W61M1** Wi-Fi 6 expansion board, and driving an **X-STM32MP-RBT01** (dual STSPIN948) motor driver board over ultrasonic 20 kHz PWM.

The rover functions as a **Wi-Fi SoftAP Host** (`GrassKit_Rover`), allowing any smartphone, tablet, or laptop to connect directly to its private Wi-Fi network and steer the car from the control page the rover serves at `http://192.168.4.1/`, or over a plain TCP socket on port 80.

---

## 1. Hardware Stack

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

## 2. Pin Conflict Resolution & Full Wiring Table

### 2.1 Critical Shield Pin Conflict Solved
When the `X-NUCLEO-67W61M1` shield is seated into the Arduino headers, it physically connects:
- **`D5` (`PB4`)** to the coprocessor **`CHIP_EN`** power enable line.
- **`D3` (`PB3`)** to **`SPI_RDY`** (data ready handshake).
- **`D6` (`PB10`)** to **`BOOT`** (run mode).
- **`D10` (`PB6`)** to **`SPI1_CS`** (chip select).
- **`D11`, `D12`, `D13` (`PA7`, `PA6`, `PA5`)** to **`SPI1`** bus lines.

> [!IMPORTANT]
> Because `PB4` (`D5`) is hardwired to the Wi-Fi `CHIP_EN` line on the shield, it **cannot be used for Motor 1 PWM**.
> 
> **Resolved Configuration:**
> - **Motor 1 PWM** is remapped to **`PC6`** (`TIM3_CH1`, Alternate Function `AF2`), located on Morpho **`CN10 pin 4`**.
> - **Motor 3 EN/FAULT** is remapped to **`PC9`**, located on Morpho **`CN10 pin 1`**.
> - All 4 motors are driven synchronously by **`TIM3` at 20 kHz ultrasonic PWM** with zero audible coil whine.

### 2.2 Wire Connection Table (Nucleo Morpho to RBT01 CN2)

Connect standard jumper wires from the **outer Morpho headers (CN7 & CN10)** to the **40-pin header (CN2)** on the RBT01:

| Function | Nucleo Pin | Nucleo Header Pin | RBT01 CN2 Pin | RBT01 Signal Net | Notes |
|:---|:---|:---|:---|:---|:---|
| **Logic 3.3V** | `3V3` | **CN7.16** (or CN6.4) | **Pin 1** | `+3V3` | Logic reference pull-up |
| **Common GND** | `GND` | **CN7.20** (or CN6.6) | **Pin 6** | `GND` | **Mandatory Ground** |
| **Motor 1 PWM** (FL) | `PC6` | **CN10.4** | **Pin 32** | `STSPIN948_1_PWM1A` | TIM3_CH1 (20 kHz PWM) |
| **Motor 1 DIR** (FL) | `PC0` | **CN7.38** | **Pin 23** | `STSPIN948_1_GPIOA` | Direction |
| **Motor 1 EN/FAULT** | `PC4` | **CN10.34** | **Pin 13** | `STSPIN948_1_ENA_nFAULTA` | Open-Drain Bidirectional |
| **Motor 2 PWM** (FR) | `PB5` | **CN10.29** | **Pin 33** | `STSPIN948_1_PWM1B` | TIM3_CH2 (20 kHz PWM) |
| **Motor 2 DIR** (FR) | `PC1` | **CN7.36** | **Pin 24** | `STSPIN948_1_GPIOB` | Direction |
| **Motor 2 EN/FAULT** | `PC5` | **CN10.6** | **Pin 15** | `STSPIN948_1_ENA_nFAULTB` | Open-Drain Bidirectional |
| **Motor 3 PWM** (RL) | `PB0` | **CN7.34** | **Pin 31** | `STSPIN948_2_PWM1A` | TIM3_CH3 (20 kHz PWM) |
| **Motor 3 DIR** (RL) | `PC2` | **CN7.35** | **Pin 19** | `STSPIN948_2_GPIOA` | Direction |
| **Motor 3 EN/FAULT** | `PC9` | **CN10.1** | **Pin 16** | `STSPIN948_2_ENA_nFAULTA` | Open-Drain Bidirectional |
| **Motor 4 PWM** (RR) | `PB1` | **CN10.24** | **Pin 36** | `STSPIN948_2_PWM1B` | TIM3_CH4 (20 kHz PWM) |
| **Motor 4 DIR** (RR) | `PC3` | **CN7.37** | **Pin 21** | `STSPIN948_2_GPIOB` | Direction |
| **Motor 4 EN/FAULT** | `PC8` | **CN10.2** | **Pin 18** | `STSPIN948_2_ENA_nFAULTB` | Open-Drain Bidirectional |

> [!CAUTION]
> Do NOT connect wires to any other pins on CN2 (pins 7, 8, 10, 11, and 35 are sensor interrupt lines).

---

## 3. Motor Terminal Wiring (CN4 & CN5)

Connect your 4 DC motors to screw terminals **CN4** and **CN5**:
- **CN4 (Front Axle)**:
  - `2A` & `1A` (Pins 1 & 2): **Front-Left Motor (M1)**
  - `1B` & `2B` (Pins 3 & 4): **Front-Right Motor (M2)**
- **CN5 (Rear Axle)**:
  - `2A` & `1A` (Pins 1 & 2): **Rear-Left Motor (M3)**
  - `1B` & `2B` (Pins 3 & 4): **Rear-Right Motor (M4)**

Connect the motor battery (7.4V - 12V) to **CN3** (`48V max` & `GND`).

---

## 4. Wi-Fi SoftAP Specifications

| Item | Setting |
|:---|:---|
| **Role** | Wi-Fi Access Point (SoftAP Host) |
| **SSID** | `GrassKit_Rover` |
| **Password** | `GrassKit2026` |
| **Security** | WPA2-PSK |
| **Rover IP** | Read from the module at boot (`AT+CIPAP?`) and printed on the USB serial log |
| **Web / TCP port** | `80`: the rover serves its own control page; plain TCP clients can also send one command per line |
| **Timeout Failsafe** | 1.5 seconds (auto-stops motors if commands stop arriving) |

> [!IMPORTANT]
> The ST67W611M1 module must run ST's **mission profile T01** firmware, v2.0.106 (matching X-CUBE-ST67W61 v1.3.0). A module on its earliest factory binary answers SPI with header magic `0x5501` (`hdr=[01 55 ...]` in the boot log) and never answers `AT`. See [`tools/ncp_bridge/README.md`](tools/ncp_bridge/README.md) to update it from this Nucleo.
>
> The driver (`firmware/Core/Src/wifi_st67w61.c`) follows ST's reference host driver: CS is active high; the host waits for SPI_RDY after raising CS, and keeps CS high until SPI_RDY drops; SoftAP uses `AT+CWMODE=3,0` + `AT+CWSAP`; data is received in passive mode (`+IPD:<id>,<len>` then `AT+CIPRECVDATA`).

---

## 5. How to Build & Flash

### 5.1 Compile Firmware
Run the PowerShell build script:
```powershell
cd 17_Rover_MotorDriver_WiFi_ST67W61\firmware
powershell -ExecutionPolicy Bypass -File build_firmware.ps1
```
Output binary generated at: `17_Rover_MotorDriver_WiFi_ST67W61\firmware\build\rover_wifi.bin`

### 5.2 Flash to STM32L073RG
Connect the Nucleo board via USB and run:
```powershell
powershell -ExecutionPolicy Bypass -File tools\flash_firmware.ps1
```

---

## 6. How to Drive the Rover

### Method A: From Any Smartphone (web page served by the rover)
1. Turn mobile data off, so the phone does not try to reach the rover over the cellular network.
2. Connect to the Wi-Fi network **`GrassKit_Rover`** (password `GrassKit2026`). If the phone warns there is no internet, choose to stay connected.
3. Open `http://192.168.4.1/` in the phone browser (the IP is also printed on the USB serial log when the SoftAP comes up).
4. **Drive** tab: hold a button to move, release to stop. The slider sets speed (10–100 %).
   - Move: forward, back, sideways ◀ ▶ and the four diagonals (mecanum mode only).
   - Turn: spin in place, curve forward or back, and pivot on one side.
   - **Mecanum / Tank** switches the mixing; Tank disables sideways moves for plain wheels.
   - **Wheels now** shows each wheel's live speed and direction.
   - On a laptop: arrows or W/S drive and spin, A/D sideways, Q/E/Z/C diagonals, space stops.
   - **EMERGENCY STOP** latches until **Reset e-stop**.
5. **Wheel lab** tab: set each wheel's PWM (−100…+100 %, + = forward) and tap **Run**. Each wheel shows its PWM, its direction pin level (PH) and an RPM estimate. The motors have no encoders, so RPM = PWM × the rated RPM you enter. **Reverse** flips a wheel, **Trim** (50–150 %) slows or speeds one wheel during drive moves, **Mirror sideways moves** fixes ◀ ▶ if the rollers are mounted the other way. **Pair wheels by side** (on by default) makes Reverse change both wheels on that side, because each front motor is wired like the rear motor behind it. **Save to rover** keeps these settings in the MCU's EEPROM.

All four wheels start together and ramp in step: each wheel covers the same share of its speed change at every tick, so they reach their new speeds at the same moment and the rover keeps its heading while speeding up and slowing down. (Earlier builds started the wheels 35 ms apart, which kicked the rover sideways at every start.)

Each held button re-sends its command every 200 ms; if the phone or Wi-Fi drops, the firmware stops the motors after 1.5 s. The page source is [`firmware/web/index.html`](firmware/web/index.html): after editing it, run `python firmware/web/make_page.py` to regenerate `Core/Src/web_page.c`, and `python firmware/web/mock_server.py` to try it on a PC at `http://localhost:8765/` without the rover. (The old `tools/rover_mobile_controller.html` used a WebSocket, which the firmware does not implement; it is superseded by this page.)

### Wheel directions (bench tests, 2026-10-30 and 2026-11-02)
- Both rear wheels turn **backwards when their PH (direction) pin is high**, so the firmware treats PH low as forward. Front left is wired like rear left and front right like rear right, so the fronts use the same setting (side pairing, see above).
- The front wheels turn forwards whatever their PH pin does, even with the wiring table above followed exactly. Checked over SWD on 2026-11-02:
  - PC0 and PC1 output a full 3.3 V when Reverse is ticked;
  - all four direction lines read high with the MCU pull-up and low with the pull-down, so none is shorted to GND or 3.3 V, and the Wi-Fi shield (which also connects to PC0/PC1 through A5/A4) does not load them.
- On the RBT01, CN2 pin 23 → **R23** (0 Ω) → U2 pin 15 (PHA), and CN2 pin 24 → **R22** (0 Ω) → U2 pin 17 (PHB) (UM3482 schematic sheet 3/4). In every STSPIN948 mode that drives the motor, direction comes only from PHx (DS14271, Table 8/9), so firmware cannot reverse a front wheel until its PH pin sees the signal. Both fronts fail together and share U2, so check, with Reverse ticked on the front wheels (PH should be 3.3 V):
  1. U2 mode jumpers **J6 = 1-2, J7 = 2-3, J8 = 1-2**, the same as J11–J13 on the rear driver, which works (UM3482, Table 1). J7 on 1-2 puts U2 in a reserved mode;
  2. 3.3 V on **both ends of R23 and R22** (to GND, e.g. CN2 pin 25). 3.3 V on the CN2 side but 0 V on the U2 side means an open link; 3.3 V at U2 with the wheel still not reversing means U2 itself.
- Then use the Wheel lab to run each front wheel at +40 % and −40 %. Tick **Reverse** on any wheel that turns the wrong way (with pairing on, its partner follows), and **Save to rover**.

### Method B: From Any Smartphone via TCP Terminal App
1. Connect to **`GrassKit_Rover`** Wi-Fi.
2. Open any free TCP terminal app (e.g. *Serial WiFi Terminal* or *TCP Client*).
3. Connect to the rover IP on port `80`.
4. Send text commands:
   - `FORWARD` or `F`: Drive forward
   - `BACKWARD` or `B`: Drive backward
   - `LEFT` or `L`: Spin left in place
   - `RIGHT` or `R`: Spin right in place
   - `STOP` or `S`: Brake / stop
   - `THROTTLE:60`: Set cruise throttle to 60%
   - `DRIVE:<throttle>,<turn>,<strafe>`: each −100…100; turn + = clockwise, strafe + = right
   - `WHEELS:<FL>,<FR>,<RL>,<RR>`: set each wheel's PWM directly (+ = forward)
   - `FLIP:<1-4>,<0|1>`, `LINK:<0|1>` (side pairing), `TRIM:<1-4>,<50-150>`, `SFLIP:<0|1>`, `MODE:MECANUM` / `MODE:TANK`
   - `SAVE` (keep wheel settings after power-off), `DEFAULTS`
   - `ESTOP`: Emergency Stop
   - `STATUS`: Query live telemetry

### Method C: From PC / Laptop
Run the Python GUI/CLI teleoperation app:
```powershell
python tools\rover_wifi_controller.py --ip <rover IP> --port 80
```
Use the arrow keys (`Up`, `Down`, `Left`, `Right`, `Space` for brake).
