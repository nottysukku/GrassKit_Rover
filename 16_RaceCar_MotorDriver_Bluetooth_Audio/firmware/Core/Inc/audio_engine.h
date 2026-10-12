/**
  ******************************************************************************
  * @file           : audio_engine.h
  * @brief          : Procedural race car engine sound synthesizer and I2S DMA
  *                   driver for MAX98357A Class-D amplifier on STM32L073RZ.
  ******************************************************************************
  */

#ifndef __AUDIO_ENGINE_H
#define __AUDIO_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hardware_config.h"

typedef struct {
    bool is_active;
    bool is_muted;
    uint8_t volume_percent;    /* 0 to 100% */
    uint32_t sample_rate;
    float current_rpm;
    float current_throttle;
    bool bov_active;
    uint16_t bov_decay;
    uint32_t pop_cooldown_ms;
    uint16_t pop_sample_counter;
    int16_t pop_amplitude;
} AudioEngine_t;

void Audio_Init(void);
void Audio_Start(void);
void Audio_Stop(void);
void Audio_SetVolume(uint8_t volume);
void Audio_SetMute(bool mute);

void Audio_UpdateParameters(float rpm, float throttle, bool is_shifting, bool is_rev_limiter, bool bov_trigger);
void Audio_TriggerOverrunPop(int16_t intensity);

/* DMA Callbacks */
void Audio_HalfTransfer_Callback(void);
void Audio_FullTransfer_Callback(void);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_ENGINE_H */
