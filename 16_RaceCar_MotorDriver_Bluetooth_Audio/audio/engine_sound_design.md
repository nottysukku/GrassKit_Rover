# Race Car Engine Sound Design & Waveform Modeling

This document details the acoustic synthesis techniques used to recreate an aggressive V8 racing engine on resource-constrained embedded hardware.

---

## 1. Cylinder Firing Pulse Waveform

A simple sine wave sounds like a sterile electronic test tone, while a triangle wave sounds like an electric buzzer. Real race car exhaust sounds are produced by steep supersonic shock waves formed when the exhaust valve opens against high cylinder pressure, followed by gas inertia expansion and acoustic ringing in the manifold runner.

To capture this character, our engine uses a custom 256-point lookup table (`k_engine_pulse_lut`) modeled after real exhaust pressure transducer data:
- **Phase 0% – 12%**: Rapid steep compression rise (+22500 amplitude peak)
- **Phase 12% – 25%**: Fast pressure relief dropping into vacuum (-5600 trough)
- **Phase 25% – 50%**: Manifold acoustic resonance oscillation
- **Phase 50% – 100%**: Valve closed exponential decay to baseline

---

## 2. Multi-Harmonic Stacking

In addition to the fundamental combustion pulse ($F_c$), real racing engines have prominent rotational mechanical resonances:
- **Rotational Fundamental ($0.5 \times F_c$)**: Produced by crank counterweights and flywheel rotation. Blended in at 37.5% gain to add low-frequency chest-thumping sub-bass.
- **Header Manifold Harmonic ($2.0 \times F_c$)**: Produced by primary header tube reflections. Blended in at 25% gain to give brightness and metallic bite at high RPM.

---

## 3. Dynamic Load & Overrun Modulation

- **High Load (WOT - Wide Open Throttle)**: Adds intake air turbulence (noise generator) proportional to throttle.
- **Engine Braking (Deceleration)**: Reduces low-end rumble while allowing transient exhaust backfire pops.
- **Sequential Gear Shift**: Dips output volume by 75% for 120 ms to simulate ignition cut / power shift, followed by an immediate recovery pop.
