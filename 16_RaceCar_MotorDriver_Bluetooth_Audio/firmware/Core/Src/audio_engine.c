/**
  ******************************************************************************
  * @file           : audio_engine.c
  * @brief          : Real-time procedural engine sound synthesizer and I2S DMA
  *                   streamer for MAX98357A Class-D audio amplifier.
  ******************************************************************************
  */

#include "audio_engine.h"
#include <string.h>
#include <stdlib.h>

static AudioEngine_t g_audio;
static I2S_HandleTypeDef hi2s2_audio;
DMA_HandleTypeDef hdma_spi2_tx;

/* Audio DMA Ping-Pong Buffer (Stereo 16-bit samples) */
#define DMA_HALF_BLOCK_SAMPLES    128
#define DMA_TOTAL_SAMPLES         (DMA_HALF_BLOCK_SAMPLES * 2 * AUDIO_NUM_CHANNELS)
static int16_t g_i2s_tx_buffer[DMA_TOTAL_SAMPLES];

/* Synthesis State */
static uint32_t s_phase_acc1 = 0;   /* Fundamental combustion frequency */
static uint32_t s_phase_acc2 = 0;   /* 2nd harmonic (crankshaft rotational) */
static uint32_t s_phase_acc3 = 0;   /* 3rd harmonic (exhaust manifold resonance) */
static uint32_t s_noise_seed = 0x12345678;
static bool s_is_shifting = false;
static bool s_is_rev_limit = false;
static uint32_t s_rev_limit_tick = 0;

/* Precomputed 256-point exhaust expansion pulse wave (Rich asymmetric racing waveform) */
static const int16_t k_engine_pulse_lut[256] = {
       0,   450,  1200,  2400,  4100,  6200,  8800, 11500,
   14200, 16800, 19000, 20800, 22000, 22500, 22200, 21200,
   19500, 17200, 14500, 11500,  8400,  5400,  2600,   200,
   -1800, -3400, -4600, -5300, -5600, -5500, -5100, -4500,
   -3800, -3000, -2300, -1600, -1000,  -500,  -100,   200,
     400,   500,   550,   500,   400,   300,   150,     0,
    -150,  -300,  -400,  -500,  -550,  -500,  -400,  -250,
    -100,     0,   100,   200,   300,   350,   300,   200,
     100,     0,  -100,  -200,  -250,  -200,  -100,     0,
      50,   100,   150,   150,   100,    50,     0,   -50,
    -100,  -120,  -100,   -50,     0,    40,    70,    80,
      60,    30,     0,   -30,   -50,   -60,   -40,   -20,
       0,    20,    30,    30,    20,    10,     0,   -10,
     -20,   -20,   -10,     0,    10,    15,    15,    10,
       5,     0,    -5,   -10,   -10,    -5,     0,     5,
       8,     8,     5,     2,     0,    -2,    -5,    -5,
      -3,     0,     2,     4,     4,     2,     1,     0,
      -1,    -2,    -2,    -1,     0,     1,     2,     2,
       1,     0,     0,    -1,    -1,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0
};

static inline int16_t FastRandomNoise(void)
{
    s_noise_seed = s_noise_seed * 1103515245 + 12345;
    return (int16_t)((s_noise_seed >> 16) & 0xFFFF);
}

static void Audio_Hardware_Init(void);
static void Audio_SynthesizeBlock(int16_t *p_out, uint16_t frame_count);

void Audio_Init(void)
{
    memset(&g_audio, 0, sizeof(g_audio));
    g_audio.is_active = false;
    g_audio.is_muted = false;
    g_audio.volume_percent = 80; /* Default 80% safe volume */
    g_audio.sample_rate = AUDIO_SAMPLE_RATE_HZ;
    g_audio.current_rpm = ENGINE_IDLE_RPM;
    g_audio.current_throttle = 0.0f;

    memset(g_i2s_tx_buffer, 0, sizeof(g_i2s_tx_buffer));

    Audio_Hardware_Init();
}

static void Audio_Hardware_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable Clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    AUDIO_I2S_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* Configure PB12 (WS), PB13 (CK), PB15 (SD) for SPI2 / I2S2 Alternate Function */
    GPIO_InitStruct.Pin = AUDIO_I2S_WS_PIN | AUDIO_I2S_CK_PIN | AUDIO_I2S_SD_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Configure I2S2 Peripheral: Master Transmit, 16 kHz, 16-bit standard */
    hi2s2_audio.Instance = AUDIO_I2S_INSTANCE;
    hi2s2_audio.Init.Mode = I2S_MODE_MASTER_TX;
    hi2s2_audio.Init.Standard = I2S_STANDARD_PHILIPS;
    hi2s2_audio.Init.DataFormat = I2S_DATAFORMAT_16B;
    hi2s2_audio.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
    hi2s2_audio.Init.AudioFreq = I2S_AUDIOFREQ_16K;
    hi2s2_audio.Init.CPOL = I2S_CPOL_LOW;
    HAL_I2S_Init(&hi2s2_audio);

    /* Configure DMA1 Channel 5 for SPI2_TX */
    hdma_spi2_tx.Instance = DMA1_Channel5;
    hdma_spi2_tx.Init.Request = DMA_REQUEST_2; /* SPI2_TX */
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_spi2_tx.Init.Mode = DMA_CIRCULAR;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_spi2_tx);

    __HAL_LINKDMA(&hi2s2_audio, hdmatx, hdma_spi2_tx);

    /* Enable DMA IRQ */
    HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);
}

void Audio_Start(void)
{
    if (!g_audio.is_active) {
        /* Pre-fill initial buffer */
        Audio_SynthesizeBlock(g_i2s_tx_buffer, DMA_TOTAL_SAMPLES / AUDIO_NUM_CHANNELS);

        /* Start DMA Transmission in Circular Mode */
        HAL_I2S_Transmit_DMA(&hi2s2_audio, (uint16_t *)g_i2s_tx_buffer, DMA_TOTAL_SAMPLES);
        g_audio.is_active = true;
    }
}

void Audio_Stop(void)
{
    if (g_audio.is_active) {
        HAL_I2S_DMAStop(&hi2s2_audio);
        g_audio.is_active = false;
        memset(g_i2s_tx_buffer, 0, sizeof(g_i2s_tx_buffer));
    }
}

void Audio_SetVolume(uint8_t volume)
{
    if (volume > 100) volume = 100;
    g_audio.volume_percent = volume;
}

void Audio_SetMute(bool mute)
{
    g_audio.is_muted = mute;
}

void Audio_UpdateParameters(float rpm, float throttle, bool is_shifting, bool is_rev_limiter, bool bov_trigger)
{
    g_audio.current_rpm = rpm;
    g_audio.current_throttle = throttle;
    s_is_shifting = is_shifting;
    s_is_rev_limit = is_rev_limiter;

    if (bov_trigger) {
        g_audio.bov_active = true;
        g_audio.bov_decay = 2400; /* ~150 ms BOV hiss */
    }
}

void Audio_TriggerOverrunPop(int16_t intensity)
{
    g_audio.pop_sample_counter = 450; /* ~28 ms transient crackle */
    g_audio.pop_amplitude = intensity;
}

/**
 * @brief  Core Procedural Sound Synthesizer (Integer Fixed-Point Cortex-M0+ Optimized)
 */
static void Audio_SynthesizeBlock(int16_t *p_out, uint16_t frame_count)
{
    if (g_audio.is_muted || !g_audio.is_active) {
        memset(p_out, 0, frame_count * AUDIO_NUM_CHANNELS * sizeof(int16_t));
        return;
    }

    /* V8 Engine Firing Frequency: 4 combustion strokes per crankshaft rev */
    /* F_hz = (RPM / 60) * 4. At 900 RPM = 60 Hz. At 6000 RPM = 400 Hz. */
    float firing_freq = (g_audio.current_rpm / 60.0f) * 4.0f;
    if (firing_freq < 20.0f) firing_freq = 20.0f;
    if (firing_freq > 800.0f) firing_freq = 800.0f;

    /* Fixed-point phase increment: (Freq * 2^24) / SampleRate */
    uint32_t phase_inc1 = (uint32_t)((firing_freq * 16777216.0f) / (float)AUDIO_SAMPLE_RATE_HZ);
    uint32_t phase_inc2 = phase_inc1 >> 1; /* Half-order subharmonic (crankshaft base) */
    uint32_t phase_inc3 = phase_inc1 * 2;  /* Double-order acoustic overtone */

    /* Throttle growl mix (0 to 128) */
    int32_t load_growl = (int32_t)((g_audio.current_throttle / 100.0f) * 128.0f);

    /* Rev limiter ignition chop: 16 Hz staccato gate */
    s_rev_limit_tick += frame_count;
    bool limiter_cut = s_is_rev_limit && ((s_rev_limit_tick % 1000) < 500);

    /* Volume multiplier (0 to 256) */
    int32_t master_vol = ((int32_t)g_audio.volume_percent * 256) / 100;
    if (s_is_shifting) {
        master_vol >>= 2; /* 75% volume dip during gear change ignition cut */
    }

    for (uint16_t i = 0; i < frame_count; i++) {
        if (limiter_cut) {
            /* Zero output during limiter cut */
            *p_out++ = 0;
            *p_out++ = 0;
            continue;
        }

        s_phase_acc1 += phase_inc1;
        s_phase_acc2 += phase_inc2;
        s_phase_acc3 += phase_inc3;

        /* Wave table lookups using high 8 bits */
        uint8_t lut_idx1 = (uint8_t)(s_phase_acc1 >> 16);
        uint8_t lut_idx2 = (uint8_t)(s_phase_acc2 >> 16);
        uint8_t lut_idx3 = (uint8_t)(s_phase_acc3 >> 16);

        int32_t sample = (int32_t)k_engine_pulse_lut[lut_idx1];
        sample += ((int32_t)k_engine_pulse_lut[lut_idx2] * 3) / 8; /* Subharmonic rumble */
        sample += ((int32_t)k_engine_pulse_lut[lut_idx3]) / 4;     /* Overtone */

        /* Add intake induction noise modulated by throttle */
        if (load_growl > 0) {
            int16_t noise = FastRandomNoise();
            sample += ((int32_t)noise * load_growl) >> 9;
        }

        /* Turbo Blow-Off Valve (BOV) hiss overlay */
        if (g_audio.bov_active) {
            int16_t bov_noise = FastRandomNoise();
            sample += ((int32_t)bov_noise * (int32_t)g_audio.bov_decay) >> 12;
            if (g_audio.bov_decay > 0) {
                g_audio.bov_decay--;
            } else {
                g_audio.bov_active = false;
            }
        }

        /* Overrun exhaust pop / burble crackle */
        if (g_audio.pop_sample_counter > 0) {
            int16_t pop_noise = FastRandomNoise();
            sample += ((int32_t)pop_noise * (int32_t)g_audio.pop_amplitude) >> 8;
            g_audio.pop_sample_counter--;
        }

        /* Apply master volume */
        sample = (sample * master_vol) >> 8;

        /* Hard clamp to 16-bit signed range */
        if (sample > 32767) sample = 32767;
        if (sample < -32768) sample = -32768;

        int16_t final_sample = (int16_t)sample;

        /* Write Stereo Channels (L and R identical for MAX98357A mono summing) */
        *p_out++ = final_sample;
        *p_out++ = final_sample;
    }
}

/* DMA Transfer Callbacks */
void Audio_HalfTransfer_Callback(void)
{
    /* Synthesize first half of ping-pong buffer */
    Audio_SynthesizeBlock(&g_i2s_tx_buffer[0], DMA_HALF_BLOCK_SAMPLES);
}

void Audio_FullTransfer_Callback(void)
{
    /* Synthesize second half of ping-pong buffer */
    Audio_SynthesizeBlock(&g_i2s_tx_buffer[DMA_HALF_BLOCK_SAMPLES * AUDIO_NUM_CHANNELS], DMA_HALF_BLOCK_SAMPLES);
}

/* HAL I2S Callbacks wired to Audio Engine */
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == AUDIO_I2S_INSTANCE) {
        Audio_HalfTransfer_Callback();
    }
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == AUDIO_I2S_INSTANCE) {
        Audio_FullTransfer_Callback();
    }
}
