/**
  ******************************************************************************
  * @file           : app.h
  * @brief          : Master application state machine and task executive.
  ******************************************************************************
  */

#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hardware_config.h"

typedef enum {
    APP_STATE_BOOT = 0,
    APP_STATE_INIT,
    APP_STATE_IDLE,
    APP_STATE_RUNNING,
    APP_STATE_GEAR_SHIFT,
    APP_STATE_BRAKING,
    APP_STATE_FAULT,
    APP_STATE_ESTOP
} AppState_t;

void App_Init(void);
void App_Run(void);
AppState_t App_GetState(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_H */
