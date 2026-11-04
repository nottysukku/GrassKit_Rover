# Wi-Fi SoftAP & Wireless Robot Control Guide
**Project: GrassKit Rover — Wireless Control over Wi-Fi 6 (ST67W611M1)**

---

## 1. SoftAP Network Configuration

The X-NUCLEO-67W61M1 Wi-Fi coprocessor is initialized at boot into **SoftAP (Access Point) Mode** and starts an onboard **TCP Server**.

| Parameter | Configuration Value |
|:---|:---|
| **Wi-Fi Mode** | SoftAP (Access Point Host) |
| **Network Name (SSID)** | `GrassKit_Rover` |
| **Password** | `GrassKit2026` |
| **Security Mode** | WPA2-PSK |
| **Wi-Fi Channel** | 1 (2.4 GHz) |
| **Rover IP Address** | `192.168.4.1` |
| **Subnet Mask** | `255.255.255.0` |
| **TCP Server Port** | `8080` |
| **Max Concurrent Sockets**| 4 clients |

---

## 2. Connecting from Your Smartphone or Laptop

### Option A: From Any Smartphone (Android / iOS)
1. Turn on Wi-Fi on your phone and search for available networks.
2. Select **`GrassKit_Rover`** and enter password **`GrassKit2026`**.
3. Once connected, your phone is automatically assigned an IP address (e.g. `192.168.4.2`).
4. To drive the rover:
   - **Method 1 (Recommended Mobile Web Controller)**: Open the provided [`rover_mobile_controller.html`](../tools/rover_mobile_controller.html) in your phone's browser. It features large touch buttons, a virtual joystick, speed slider, and live telemetry!
   - **Method 2 (TCP Terminal App)**: Download any free TCP Client app (such as *Serial WiFi Terminal* or *TCP Client*):
     - Target Host / IP: `192.168.4.1`
     - Port: `8080`
     - Protocol: TCP
     - Connect and send commands (e.g. `FORWARD`, `STOP`, `THROTTLE:50`).

### Option B: From Your PC or Mac
1. Connect your PC's Wi-Fi to **`GrassKit_Rover`**.
2. Run the included Python teleoperation script:
   ```powershell
   python tools/rover_wifi_controller.py --ip 192.168.4.1 --port 8080
   ```
3. Use the arrow keys (`Up`, `Down`, `Left`, `Right`, `Space` for brake) or an interactive graphical interface.

---

## 3. Communication Protocol Specification

All commands are sent as ASCII strings terminated by `\r\n` or `\n`. The rover responds with status acknowledgments.

### 3.1 Basic Navigation Commands

| Command | Action | Description |
|:---|:---|:---|
| `FORWARD` or `F` | Drive Forward | Both left and right tracks drive forward at default cruise speed (40%) |
| `BACKWARD` or `B`| Drive Backward | Both left and right tracks reverse at default cruise speed (40%) |
| `LEFT` or `L` | Spin Left | Left motors reverse, right motors drive forward (skid-steer zero-radius turn) |
| `RIGHT` or `R` | Spin Right | Left motors drive forward, right motors reverse (skid-steer zero-radius turn) |
| `STOP` or `S` | Brake / Stop | Ramp down PWM to 0% and engage low-side electronic brake |
| `ESTOP` | Emergency Stop | Immediate hard cutoff: pulls all STSPIN948 EN lines LOW |
| `RESET` | Clear Alarm | Re-enables H-bridge drivers after an overcurrent/thermal alarm |

### 3.2 Continuous Proportional Drive Commands

| Command | Format & Range | Example | Description |
|:---|:---|:---|:---|
| `THROTTLE` | `THROTTLE:<val>` (-100 to +100) | `THROTTLE:75` | Sets forward (+75%) or reverse (-75%) thrust |
| `STEER` | `STEER:<val>` (-100 to +100) | `STEER:-30` | Differential bias (+ right, - left) |
| `DRIVE` | `DRIVE:<throttle>,<steer>` | `DRIVE:50,20` | Combined vector: throttle 50%, gentle right turn 20% |
| `MOTOR` | `MOTOR:<id>,<speed>` | `MOTOR:1,40` | Individual wheel test (ID 1-4, speed -100 to +100) |

### 3.3 System & Diagnostics Commands

| Command | Description | Rover Response Example |
|:---|:---|:---|
| `PING` | Communication latency check | `PONG:OK` |
| `STATUS` | Full telemetry snapshot | `STATUS:STATE=RUNNING,THROTTLE=50.0,STEER=0.0,FAULTS=0,UPTIME=12400` |
| `HELP` | Print supported command listing | Command reference help list |

---

## 4. Built-in Failsafe & Watchdog Protection

1. **Wireless Communication Timeout (1500 ms)**:
   - If the controlling phone/PC loses Wi-Fi connection or drops out of range for more than 1.5 seconds, the rover's safety watchdog automatically halts all motors to prevent runaway.
   - Sending `PING` or driving commands periodically resets the watchdog timer.
2. **STSPIN948 Hardware Protection**:
   - The bidirectional `EN/nFAULT` lines are monitored by the MCU continuously.
   - If an overcurrent or thermal shutdown occurs on any motor bridge, the MCU instantly disables the remaining channels, enters `ALARM` state, and logs the faulted bridge ID.
3. **Hardware E-Stop Button**:
   - Pressing the blue user button (`B1` / `PC13`) on the Nucleo board instantly triggers an emergency stop at the physical hardware layer.
