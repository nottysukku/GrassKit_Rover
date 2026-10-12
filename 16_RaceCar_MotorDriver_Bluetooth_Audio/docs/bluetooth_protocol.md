# Bluetooth & Serial Command Protocol Specification

The system uses an ASCII text-based, line-delimited (`\r\n` or `\n`) protocol that can be driven from:
- Android / iOS Web Bluetooth browser app (`tools/web_bluetooth_controller.html`)
- Android "Serial Bluetooth Terminal" app
- iOS "Bluefy" or "nRF Connect"
- PC Python Controller GUI (`tools/racecar_controller.py`)
- Standard serial terminals (PuTTY, Tera Term, minicom) @ 115200 baud, 8-N-1

---

## 1. System Control Commands

| Command | Arguments | Description | Response | Example |
|:---|:---|:---|:---|:---|
| `START` | None | Enables motor driver power stage and starts audio engine | `OK:START\r\n` | `START` |
| `STOP` | None | Gracefully ramps motor speed to 0 and returns engine to idle | `OK:STOP\r\n` | `STOP` |
| `BRAKE` | None | Immediately zeros motor PWM and triggers exhaust pop sound | `OK:BRAKE\r\n` | `BRAKE` |
| `ESTOP` | None | Emergency Stop: Kills PWM, shuts down STBY line, latches e-stop | `ALARM:EMERGENCY_STOP_TRIGGERED\r\n` | `ESTOP` |
| `RESET` | None | Clears emergency stop latch and resets fault status | `OK:RESET\r\n` | `RESET` |
| `STATUS`| None | Queries full vehicle state telemetry | `STATUS:STATE=...,RPM=...` | `STATUS` |
| `HELP`  | None | Returns summary of supported commands | Multi-line text | `HELP` |

---

## 2. Motion & Driving Commands

| Command | Argument Range | Description | Example |
|:---|:---|:---|:---|
| `THROTTLE:<val>` | `0` to `100` | Sets forward drive throttle percentage | `THROTTLE:45` |
| `STEER:<val>` | `-100` to `100` | Differential steering yaw (-100 = full left, +100 = full right) | `STEER:25` |
| `STRAFE:<val>` | `-100` to `100` | Lateral Mecanum strafe (-100 = strafe left, +100 = strafe right) | `STRAFE:-60` |
| `DRIVE:<th,st,strf>` | `0..100, -100..100, -100..100` | Combined single-packet vector update for joystick control | `DRIVE:50,15,-20` |
| `MODE:<mode>` | `RACECAR` or `MECANUM` | Switches between differential car steering and omnidirectional Mecanum kinematics | `MODE:MECANUM` |

---

## 3. Engine & Sound Effects Commands

| Command | Arguments | Description | Example |
|:---|:---|:---|:---|
| `GEAR:<val>` | `1` to `6` | Sets manual virtual gear (1st to 6th) | `GEAR:3` |
| `GEAR:AUTO` | None | Enables automatic transmission mode | `GEAR:AUTO` |
| `GEAR:UP` | None | Shifts up one gear with ignition cut effect | `GEAR:UP` |
| `GEAR:DOWN`| None | Shifts down one gear with rev-match effect | `GEAR:DOWN` |
| `TURBO:<val>`| `ON` or `OFF` | Enables/disables turbocharger boost buildup | `TURBO:ON` |
| `REV` | None | Blips throttle in neutral, producing sudden RPM surge & overrun crackle | `REV` |
| `VOLUME:<val>`| `0` to `100` | Sets master digital audio gain percentage | `VOLUME:85` |

---

## 4. Telemetry Format

### Response to `STATUS`:
```text
STATUS:STATE=RUNNING,RPM=3450,THROTTLE=45,GEAR=3,BOOST=0.60,M1=45,M2=45,M3=45,M4=45
```

### Periodic Telemetry Stream (Every 200 ms when connected):
```text
TELE:RPM=3450,THROT=45%,GEAR=3,BOOST=0.60,M1=45,M2=45,M3=45,M4=45
```

---

## 5. Communication Watchdog & Safety Protocol

1. **Heartbeat Timeout**: If no valid command packet is received within **1500 ms** while the car is in `RUNNING` state, the safety supervisor automatically executes a graceful stop (`Motor_Stop()`) and emits:
   ```text
   WARN:COMM_TIMEOUT_AUTO_STOP\r\n
   ```
2. **Periodic Keep-Alive**: When driving with joystick controls, the mobile app or PC controller should send a `DRIVE:...` packet every 100 ms to satisfy the watchdog.
