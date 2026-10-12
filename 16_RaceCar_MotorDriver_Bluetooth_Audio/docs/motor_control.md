# Motor Control & Mecanum Kinematics Architecture

This document details the motor control subsystem, STSPIN948 driver interface, and 4-wheel Mecanum omnidirectional kinematic equations.

---

## 1. STSPIN948 Dual Full-Bridge Motor Driver

The **X-STM32MP-RBT01** expansion board features two **STSPIN948** ICs:
- **U3 (Left IC)**: Drives Motor 1 (Front-Left) and Motor 2 (Front-Right) via Screw Terminal J1.
- **U2 (Right IC)**: Drives Motor 3 (Rear-Left) and Motor 4 (Rear-Right) via Screw Terminal J4.

### Control Lines per Motor
Each motor channel receives:
1. **PWM Speed Input**: Configured at **20 kHz** ultrasonic frequency via TIM3 Channels 1–4 (`PB4`, `PB5`, `PB0`, `PB1`). At 20 kHz, switching noise is beyond human hearing, eliminating annoying high-pitched motor whine.
2. **Direction GPIO Input**: Digital outputs (`PC0`, `PC1`, `PC2`, `PC3`) controlling H-bridge polarity.
3. **Global Driver Enable / Standby**: Pin `PC4`. Held LOW on startup (high-impedance safe state). Driven HIGH by `Motor_Start()`.
4. **Hardware Fault Monitor**: Pin `PC5`. Open-drain active-low input tied to internal driver diagnostics (over-temperature, overcurrent, UVLO).

### Current Limit Configuration (Trimpots R70 & R71)
The board includes onboard trimpots `R70` and `R71` to set the internal current comparator reference voltage ($V_{\text{REF}}$):

$$I_{\text{peak}} = \frac{V_{\text{REF}}}{R_{\text{sense}}}$$

With $R_{\text{sense}} = 50\text{ m}\Omega$ (`R050`), adjust trimpots so current does not exceed the continuous stall rating of your chassis motors (typically 1.0 A – 2.0 A per motor).

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
