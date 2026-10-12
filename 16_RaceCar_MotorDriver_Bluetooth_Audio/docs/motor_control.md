# Motor Control & Mecanum Kinematics Architecture

This document details the motor control subsystem, STSPIN948 driver interface, and 4-wheel Mecanum omnidirectional kinematic equations.

---

## 1. STSPIN948 Dual Full-Bridge Motor Driver

The **X-STM32MP-RBT01** expansion board features two **STSPIN948** ICs, each
containing two independent full-bridges (A and B):
- **U2** (`STSPIN948_1`): Drives Motor 1 (Front-Left, bridge A) and Motor 2 (Front-Right, bridge B) via Screw Terminal CN4.
- **U3** (`STSPIN948_2`): Drives Motor 3 (Rear-Left, bridge A) and Motor 4 (Rear-Right, bridge B) via Screw Terminal CN5.

Both ICs are strapped (jumpers J6/J7/J8 and J11/J12/J13, factory default) for
**dual independent full-bridge mode with fixed OFF-time current limiting**.

### Control Lines per Motor
Each motor channel has three lines (see `hardware/pinout.md` for the CN2 pins):
1. **PWM Speed Input (`PWM1x`)**: Configured at **20 kHz** ultrasonic frequency via TIM3 Channels 1–4 (`PB4`, `PB5`, `PB0`, `PB1`). At 20 kHz, switching noise is beyond human hearing, eliminating annoying high-pitched motor whine. While `PWM1x` is low an enabled bridge brakes the motor (both low-side MOSFETs on).
2. **Direction Input (`PHx`)**: Digital outputs (`PC0`, `PC1`, `PC2`, `PC3`) controlling H-bridge polarity.
3. **Enable / Fault (`EN/nFAULTx`)**: Open-drain lines `PC4`, `PC5`, `PC6`, `PC8`, one per motor. The RBT01 pulls each line up to 3V3 through 39 kΩ, so a bridge is **enabled unless the MCU holds its line low**. The firmware:
   - drives all four lines low at power-up and whenever the car is idle (bridge outputs high-impedance, wheels cannot be driven);
   - releases them in `Motor_Start()` and pulls them low again once the wheels have ramped to zero after `Motor_Stop()`, and immediately on any emergency stop;
   - reads the lines back while they are released. The STSPIN948 pulls a line low on under-voltage lockout (motor supply below 5.2 V typ), overcurrent or thermal shutdown. After a 10 ms settling time, 3 low samples latch a fault, disable all bridges and send `ALARM:DRIVER_FAULT:M1=..,M2=..,M3=..,M4=..`. Clear it with `RESET`.

There is no global enable/standby pin and no separate fault pin on the 40-pin
connector: `nSTDBY` is tied high on the board.

### Current Limit Configuration (Trimmers R51, R55, R71, R76)
Each bridge has its own trimmer setting the current-limiter reference voltage
($V_{\text{REF}}$): `R51` (M1), `R55` (M2), `R71` (M3), `R76` (M4). The shunt
voltage is amplified by the STSPIN948's internal amplifier ($A_{\text{CL}} = 10$)
before it is compared with $V_{\text{REF}}$:

$$I_{\text{peak}} = \frac{V_{\text{REF}}}{A_{\text{CL}} \times R_{\text{sense}}} = \frac{V_{\text{REF}}}{0.5\ \Omega}$$

With $R_{\text{sense}} = 50\text{ m}\Omega$ (`R050`), every 0.5 V of $V_{\text{REF}}$ is 1 A of peak current. Adjust the trimmers so current does not exceed the continuous stall rating of your chassis motors (typically 1.0 A – 2.0 A per motor, i.e. 0.5 V – 1.0 V).

---

## 2. Mecanum 4-Wheel Kinematic Equations

A Mecanum wheel chassis uses 4 wheels with passive rollers oriented at 45° to the wheel plane. By varying the individual rotation speed and direction of each wheel, the chassis can translate in any 2D direction (forward, backward, strafe sideways, diagonal) while simultaneously rotating (holonomic motion).

```text
               FRONT
        M1 (FL) //     \\ M2 (FR)
          ┌───────────────┐
          │   ROVER       │
          │   CHASSIS     │
          └───────────────┘
        M3 (RL) \\     // M4 (RR)
                REAR
```

### Inverse Kinematic Equations
Given desired translation velocities $V_x$ (forward/back), $V_y$ (strafe right/left), and rotational velocity $\omega$ (yaw clockwise):

$$\begin{aligned}
V_{\text{FL}} &= V_x + V_y + \omega \\
V_{\text{FR}} &= V_x - V_y - \omega \\
V_{\text{RL}} &= V_x - V_y + \omega \\
V_{\text{RR}} &= V_x + V_y - \omega
\end{aligned}$$

### Speed Normalization
If the calculation for any wheel exceeds 100% duty cycle, all four wheel velocities are divided by the maximum magnitude:

$$V_{\text{max}} = \max(|V_{\text{FL}}|, |V_{\text{FR}}|, |V_{\text{RL}}|, |V_{\text{RR}}|)$$

$$\text{If } V_{\text{max}} > 100\% \implies V_i = V_i \times \frac{100\%}{V_{\text{max}}}$$

This guarantees that the vehicle moves precisely along the commanded trajectory vector without direction distortion caused by motor clipping.

---

## 3. Acceleration / Deceleration Ramping

Sudden steps in motor duty cycle create massive back-EMF spikes, high inrush currents that can brown out the microcontroller logic rail, and severe mechanical gear wear.

To eliminate this:
- The firmware maintains `target_speed` and `current_speed` for each motor.
- In `Motor_UpdateRamp(delta_ms)`, `current_speed` moves toward `target_speed` at a rate of:
  $$\Delta V \le \text{RAMP\_RATE} \times \Delta t \quad (2.0\% \text{ per ms})$$
- Full acceleration from 0% to 100% takes a smooth, controlled 50 milliseconds.
- A deadband of 4.0% is applied so motors do not stall or buzz at sub-threshold PWM levels.
