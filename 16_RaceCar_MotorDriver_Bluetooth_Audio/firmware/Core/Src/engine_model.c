/**
  ******************************************************************************
  * @file           : engine_model.c
  * @brief          : Physics simulation for race car engine RPM, gearbox,
  *                   and turbocharger boost dynamics.
  ******************************************************************************
  */

#include "engine_model.h"
#include <string.h>

static EngineModel_t g_engine;

/* 6-Speed Gear Ratios */
static const float k_gear_ratios[6] = {
    3.60f,  /* 1st Gear */
    2.40f,  /* 2nd Gear */
    1.75f,  /* 3rd Gear */
    1.30f,  /* 4th Gear */
    1.00f,  /* 5th Gear */
    0.80f   /* 6th Gear */
};

/* Upshift and Downshift RPM thresholds for Automatic Mode */
static const float k_auto_upshift_rpm   = 6200.0f;
static const float k_auto_downshift_rpm = 2400.0f;

void Engine_Init(void)
{
    memset(&g_engine, 0, sizeof(g_engine));
    g_engine.current_rpm = ENGINE_IDLE_RPM;
    g_engine.target_rpm = ENGINE_IDLE_RPM;
    g_engine.current_gear = 1;
    g_engine.auto_gearbox = true;
    g_engine.turbo_enabled = true;
    g_engine.boost_pressure_bar = 0.0f;
    g_engine.rev_limiter_active = false;
    g_engine.is_shifting = false;
    g_engine.shift_timer_ms = 0;
}

void Engine_Update(uint32_t delta_ms, float physical_throttle)
{
    if (delta_ms == 0) return;

    /* Handle Gear Shift in progress */
    if (g_engine.is_shifting) {
        if (g_engine.shift_timer_ms > delta_ms) {
            g_engine.shift_timer_ms -= delta_ms;
            /* During shift, RPM decays rapidly towards new gear match */
            g_engine.current_rpm -= 8.0f * (float)delta_ms;
            if (g_engine.current_rpm < ENGINE_IDLE_RPM) {
                g_engine.current_rpm = ENGINE_IDLE_RPM;
            }
            return;
        } else {
            g_engine.is_shifting = false;
            g_engine.shift_timer_ms = 0;
        }
    }

    g_engine.throttle_input = physical_throttle;

    /* Calculate Target RPM based on throttle and gear load */
    float throttle_norm = g_engine.throttle_input / 100.0f;
    if (throttle_norm < 0.0f) throttle_norm = 0.0f;
    if (throttle_norm > 1.0f) throttle_norm = 1.0f;

    g_engine.target_rpm = ENGINE_IDLE_RPM + throttle_norm * (ENGINE_MAX_RPM - ENGINE_IDLE_RPM);

    /* Dynamic Inertia Response:
     * High acceleration rate under load (quicker in lower gears).
     * Slower natural deceleration rate on throttle lift (engine braking). */
    float gear_ratio = k_gear_ratios[g_engine.current_gear - 1];
    float accel_rate = (14.0f * gear_ratio) * (float)delta_ms;
    float decel_rate = 4.5f * (float)delta_ms;

    if (g_engine.rev_limiter_active) {
        /* Fuel cut bounce effect: drop 350 RPM then re-enable */
        g_engine.current_rpm -= 15.0f * (float)delta_ms;
        if (g_engine.current_rpm <= (ENGINE_REV_LIMIT_RPM - 350.0f)) {
            g_engine.rev_limiter_active = false;
        }
    } else if (g_engine.target_rpm > g_engine.current_rpm) {
        g_engine.current_rpm += accel_rate;
        if (g_engine.current_rpm >= ENGINE_REV_LIMIT_RPM) {
            g_engine.current_rpm = ENGINE_REV_LIMIT_RPM;
            g_engine.rev_limiter_active = true; /* Hard rev limit bounce */
        }
    } else {
        g_engine.current_rpm -= decel_rate;
        if (g_engine.current_rpm < ENGINE_IDLE_RPM) {
            g_engine.current_rpm = ENGINE_IDLE_RPM;
        }
    }

    /* Turbocharger physics */
    if (g_engine.turbo_enabled) {
        if (g_engine.throttle_input > 40.0f && g_engine.current_rpm > 2800.0f) {
            float target_boost = ((g_engine.throttle_input - 40.0f) / 60.0f) * 1.5f;
            g_engine.boost_pressure_bar += 0.002f * (float)delta_ms;
            if (g_engine.boost_pressure_bar > target_boost) {
                g_engine.boost_pressure_bar = target_boost;
            }
        } else {
            g_engine.boost_pressure_bar -= 0.005f * (float)delta_ms;
            if (g_engine.boost_pressure_bar < 0.0f) {
                g_engine.boost_pressure_bar = 0.0f;
            }
        }
    } else {
        g_engine.boost_pressure_bar = 0.0f;
    }

    /* Automatic Gearbox Management */
    if (g_engine.auto_gearbox && !g_engine.is_shifting) {
        if (g_engine.current_rpm >= k_auto_upshift_rpm && g_engine.current_gear < 6) {
            Engine_ShiftUp();
        } else if (g_engine.current_rpm <= k_auto_downshift_rpm && g_engine.current_gear > 1 && g_engine.throttle_input < 30.0f) {
            Engine_ShiftDown();
        }
    }
}

void Engine_SetThrottle(float throttle)
{
    if (throttle < 0.0f) throttle = 0.0f;
    if (throttle > 100.0f) throttle = 100.0f;
    g_engine.throttle_input = throttle;
}

void Engine_SetGear(uint8_t gear)
{
    if (gear >= 1 && gear <= 6 && gear != g_engine.current_gear) {
        g_engine.current_gear = gear;
        g_engine.is_shifting = true;
        g_engine.shift_timer_ms = 130; /* 130 ms shift cut */
    }
}

void Engine_ShiftUp(void)
{
    if (g_engine.current_gear < 6) {
        Engine_SetGear(g_engine.current_gear + 1);
    }
}

void Engine_ShiftDown(void)
{
    if (g_engine.current_gear > 1) {
        Engine_SetGear(g_engine.current_gear - 1);
    }
}

void Engine_SetAutoGear(bool enable)
{
    g_engine.auto_gearbox = enable;
}

void Engine_SetTurbo(bool enable)
{
    g_engine.turbo_enabled = enable;
}

void Engine_RevThrottle(float rev_burst)
{
    g_engine.current_rpm += rev_burst;
    if (g_engine.current_rpm > ENGINE_REV_LIMIT_RPM) {
        g_engine.current_rpm = ENGINE_REV_LIMIT_RPM;
        g_engine.rev_limiter_active = true;
    }
}

float Engine_GetRPM(void)
{
    return g_engine.current_rpm;
}

uint8_t Engine_GetGear(void)
{
    return g_engine.current_gear;
}

float Engine_GetBoost(void)
{
    return g_engine.boost_pressure_bar;
}

bool Engine_IsShifting(void)
{
    return g_engine.is_shifting;
}

bool Engine_IsRevLimiter(void)
{
    return g_engine.rev_limiter_active;
}
