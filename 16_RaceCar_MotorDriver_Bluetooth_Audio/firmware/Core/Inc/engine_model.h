/**
  ******************************************************************************
  * @file           : engine_model.h
  * @brief          : Virtual race car engine dynamics, 6-speed gearbox, and
  *                   turbocharger physics simulation.
  ******************************************************************************
  */

#ifndef __ENGINE_MODEL_H
#define __ENGINE_MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hardware_config.h"

typedef struct {
    float current_rpm;
    float target_rpm;
    float throttle_input;     /* 0.0f to 100.0f */
    uint8_t current_gear;     /* 1 to 6 */
    bool auto_gearbox;
    bool turbo_enabled;
    float boost_pressure_bar; /* 0.0f to 1.8f bar */
    bool is_shifting;
    uint32_t shift_timer_ms;
    bool rev_limiter_active;
    bool blow_off_valve_fired;
} EngineModel_t;

void Engine_Init(void);
void Engine_Update(uint32_t delta_ms, float physical_throttle);

void Engine_SetThrottle(float throttle);
void Engine_SetGear(uint8_t gear);
void Engine_ShiftUp(void);
void Engine_ShiftDown(void);
void Engine_SetAutoGear(bool enable);
void Engine_SetTurbo(bool enable);
void Engine_RevThrottle(float rev_burst);

float Engine_GetRPM(void);
uint8_t Engine_GetGear(void);
float Engine_GetBoost(void);
bool Engine_IsShifting(void);
bool Engine_IsRevLimiter(void);
bool Engine_ConsumeBOVTrigger(void);

#ifdef __cplusplus
}
#endif

#endif /* __ENGINE_MODEL_H */
