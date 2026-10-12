# Real-Time Audio Generation Implementation

Details on the STM32 I2S peripheral and DMA streaming pipeline.

---

## 1. I2S Peripheral & DMA Configuration

- **Peripheral**: STM32L073 SPI2 operating in I2S Master Transmit mode.
- **Clocking**: Driven by SYSCLK (32 MHz).
- **Audio Frequency**: 16,000 Hz.
- **Data Format**: 16-bit extended on 32-bit frame (or 16-bit data on 16-bit frame).
- **Bit Clock (BCLK)**: $16000 \times 16 \times 2 = 512,000 \text{ Hz}$ on pin `PB13`.
- **Word Select (LRC)**: 16,000 Hz square wave on pin `PB12`.
- **Serial Data (DIN)**: Continuous PCM data on pin `PB15`.

---

## 2. Ping-Pong Buffer Memory Layout

```text
g_i2s_tx_buffer (512 total halfwords = 1024 bytes)
├── [0 .. 255]: Half-Buffer 0 (128 Stereo L/R Samples)
│    └── Synthesized in Audio_HalfTransfer_Callback() while DMA transmits Half-Buffer 1
└── [256 .. 511]: Half-Buffer 1 (128 Stereo L/R Samples)
     └── Synthesized in Audio_FullTransfer_Callback() while DMA transmits Half-Buffer 0
```

This guarantees 100% glitch-free audio streaming without audio underruns or buffer starvation.

---

## 3. Connecting to MAX98357A

- Solder or connect headers to `VIN` (+5V), `GND`, `BCLK` (PB13), `LRC` (PB12), and `DIN` (PB15).
- Connect `GAIN` to `GND` to select 9 dB gain.
- Connect an 4 Ω or 8 Ω speaker (3 Watt rating) to the speaker output terminals.
- When the STM32 boots, the speaker will immediately emit the realistic V8 engine idle sound at 900 RPM.
