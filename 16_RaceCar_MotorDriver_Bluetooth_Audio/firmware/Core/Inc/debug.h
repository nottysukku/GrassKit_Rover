/**
  ******************************************************************************
  * @file           : debug.h
  * @brief          : Serial console logging and periodic telemetry manager.
  ******************************************************************************
  */

#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hardware_config.h"

void Debug_Init(void);
void Debug_PrintBanner(void);
void Debug_Log(const char *fmt, ...);
void Debug_UpdatePeriodic(uint32_t delta_ms);

#ifdef __cplusplus
}
#endif

#endif /* __DEBUG_H */
