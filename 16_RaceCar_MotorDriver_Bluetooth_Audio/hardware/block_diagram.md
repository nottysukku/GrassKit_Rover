# System Architecture & Block Diagram

This document presents the complete hardware and firmware block diagrams for the STM32 Race Car Demonstrator.

---

## 1. High-Level System Architecture

```text
       ┌────────────────────────────────────────────────────────┐
       │                USER CONTROL INTERFACE                  │
       │   - Android / iOS Phone (Web Bluetooth / BLE App)      │
       │   - PC Control GUI (Python Tkinter / Serial Terminal)  │
       └───────────────────────────┬────────────────────────────┘
                                   │
                                   │ Wireless (BLE / Bluetooth RF)
                                   ▼
       ┌────────────────────────────────────────────────────────┐
       │                WIRELESS INTERFACE LAYER                │
       │   - ST X-NUCLEO-IDB05A2 (SPBTLE-1S BlueNRG BLE Shield) │
       │   - External UART BT Module (HC-05 / HM-10 on USART1)   │
       │   - ST-LINK Virtual COM Port (USART2 on PA2/PA3)       │
       └───────────────────────────┬────────────────────────────┘
                                   │
                                   │ SPI1 / USART1 / USART2
                                   ▼
       ┌────────────────────────────────────────────────────────┐
       │          STM32 NUCLEO-L073RZ (ARM Cortex-M0+)          │
       │                                                        │
       │  ┌──────────────────┐            ┌──────────────────┐  │
       │  │  Command Parser  │            │  Safety Monitor  │  │
       │  │  - DRIVE, GEAR   │            │  - Heartbeat WD  │  │
       │  │  - REV, TURBO    │            │  - Hardware E-Stop│  │
       │  └────────┬─────────┘            └────────┬─────────┘  │
       │           │                               │            │
       │           ▼                               ▼            │
       │  ┌──────────────────┐            ┌──────────────────┐  │
       │  │  Engine Model    │───────────►│  Motor Control   │  │
       │  │  - RPM (900-7000)│ Throttle   │  - TIM3 (20 kHz) │  │
       │  │  - 6-Speed Gear  │ Steer      │  - 4WD Mecanum   │  │
       │  │  - Turbo Boost   │ Strafe     │  - S-Curve Ramp  │  │
       │  └────────┬─────────┘            └────────┬─────────┘  │
       │           │                               │            │
       │           ▼                               │            │
       │  ┌──────────────────┐                     │            │
       │  │ Audio Synthesizer│                     │            │
       │  │  - Combustion Fx │                     │            │
       │  │  - Intake Roar   │                     │            │
       │  │  - Overrun Pops  │                     │            │
       │  │  - I2S2 + DMA1   │                     │            │
       │  └────────┬─────────┘                     │            │
       └───────────┼───────────────────────────────┼────────────┘
                   │                               │
             I2S2  │                               │ 4x PWM + 4x DIR
         (PB12/13/15)                              │ (PB4/5/0/1, PC0-3)
                   ▼                               ▼
       ┌───────────────────────┐       ┌────────────────────────┐
       │ MAX98357A CLASS-D AMP │       │  ST X-STM32MP-RBT01    │
       │ (3.2W Filterless I2S) │       │ (Dual STSPIN948, 48V)  │
       └───────────┬───────────┘       └───────────┬────────────┘
                   │                               │
                   │ Differential Audio            │ 4x Motor Drive (H-Bridge)
                   ▼                               ▼
       ┌───────────────────────┐       ┌────────────────────────┐
       │  LOUDSPEAKER (4-8 Ω)  │       │ 4x MECANUM DC MOTORS   │
       │   V8 Race Car Engine  │       │ FL, FR, RL, RR Drive   │
       │       Sound FX        │       │                        │
       └───────────────────────┘       └────────────────────────┘
```

---

## 2. Power Distribution & Safety Architecture

```text
   BATTERY PACK (7.4V - 12.0V LiPo / NiMH / 18650)
        │
        ├───[Power Switch]───────► X-STM32MP-RBT01 VIN (Screw Terminal)
        │                          │ (Protected by 15A Blade Fuse F1)
        │                          │
        │                          ▼
        │                    STSPIN948 Power Stage ──► 4x Brushed DC Motors
        │
        │
   USB 5V (or Step-Down 5V Regulator)
        │
        ├───► Nucleo-L073RZ (5V Pin)
        │        │
        │        ├───► Nucleo 3.3V LDO ──► STM32L073RZ VDD
        │        │                     ──► X-STM32MP-RBT01 Logic VDD (Pin 1)
        │        │                     ──► X-NUCLEO-IDB05A2 VDD
        │        │
        │        └───► MAX98357A VIN (+5V)
        │
   COMMON GROUND (GND)
        │
        └──────────────────────────────── ALL MODULES TIED TO COMMON GND!
```

> [!CAUTION]
> **ELECTRICAL ISOLATION WARNING**:
> - Never feed motor battery voltage (>5V) to any pin of the Nucleo-L073RZ or MAX98357A!
> - The STSPIN948 power ground and logic ground meet at the X-STM32MP-RBT01 ground plane. Ensure an explicit, heavy ground wire connects the Nucleo GND to the motor driver GND before powering up.
