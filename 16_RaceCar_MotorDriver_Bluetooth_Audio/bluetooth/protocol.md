# Bluetooth Command Protocol Quick Reference

See `docs/bluetooth_protocol.md` for complete specification.

## Core Commands Quick Table:
- `START` - Enable motor driver and sound
- `STOP` - Graceful stop
- `BRAKE` - Electric active brake
- `ESTOP` - Emergency stop (requires `RESET`)
- `RESET` - Clear e-stop latch
- `THROTTLE:<0..100>` - Drive forward
- `STEER:<-100..100>` - Steering (-100 Left .. +100 Right)
- `STRAFE:<-100..100>` - Mecanum strafe (-100 Left .. +100 Right)
- `DRIVE:<th,st,strf>` - Full motion vector
- `GEAR:<1..6|AUTO|UP|DOWN>` - Sequential gearbox
- `TURBO:<ON|OFF>` - Turbo boost
- `REV` - Throttle blip
- `MODE:<RACECAR|MECANUM>` - Drive kinematics mode
- `STATUS` - Request telemetry
- `HELP` - Command summary
