/**
  ******************************************************************************
  * @file           : safety.h
  * @brief          : Hardware safety supervisor, heartbeat watchdog, and
  *                   emergency-stop management.
  ******************************************************************************
  */

#ifndef __SAFETY_H
#define __SAFETY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hardware_config.h"

typedef struct {
    uint32_t last_heartbeat_ms;
    uint32_t timeout_limit_ms;
    bool estop_latched;
    bool comm_timeout_occurred;
} SafetySupervisor_t;

void Safety_Init(void);
void Safety_Update(uint32_t delta_ms);
void Safety_KickWatchdog(void);

void Safety_TriggerEmergencyStop(void);
void Safety_ResetEmergencyStop(void);

bool Safety_IsEmergencyStopActive(void);
bool Safety_IsTimeoutActive(void);

#ifdef __cplusplus
}
#endif

#endif /* __SAFETY_H */
