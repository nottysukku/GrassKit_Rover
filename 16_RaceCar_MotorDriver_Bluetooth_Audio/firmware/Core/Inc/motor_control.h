/**
  ******************************************************************************
  * @file           : motor_control.h
  * @brief          : Motor control abstraction and Mecanum kinematics solver
  *                   for STSPIN948 on ST X-STM32MP-RBT01.
  ******************************************************************************
  */

#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hardware_config.h"

typedef enum {
    MOTOR_STATE_DISABLED = 0,
    MOTOR_STATE_IDLE,
    MOTOR_STATE_RUNNING,
    MOTOR_STATE_BRAKING,
    MOTOR_STATE_FAULT,
    MOTOR_STATE_ESTOP
} MotorState_t;

typedef enum {
    DRIVE_MODE_RACECAR = 0,   /* Throttle (-100%..100%) + Steering (-100%..100%) */
    DRIVE_MODE_MECANUM_OMNI   /* Throttle, Strafe, and Rotation holonomic control */
} DriveMode_t;

typedef struct {
    float target_speed;       /* Target speed percentage: -100.0f to +100.0f */
    float current_speed;      /* Current ramped speed percentage */
    uint16_t pwm_duty;        /* Applied timer PWM duty cycle ticks */
    uint8_t direction;        /* 1 = Forward, 0 = Reverse */
} MotorChannel_t;

typedef struct {
    MotorState_t state;
    DriveMode_t drive_mode;
    MotorChannel_t motors[4]; /* Index 0: FL, 1: FR, 2: RL, 3: RR */
    float overall_throttle;   /* 0.0f to 100.0f */
    float overall_steer;      /* -100.0f to 100.0f */
    float overall_strafe;     /* -100.0f to 100.0f */
    bool fault_detected;
    bool estop_active;
    bool bridges_enabled;     /* EN/nFAULT lines released (power stages active) */
    uint8_t fault_mask;       /* Bit n set = driver fault on motor n+1 */
    uint8_t fault_samples[4]; /* Low-sample counters for EN/nFAULT debounce */
    uint32_t fault_blanking_ms;
} MotorControl_t;

/* Public API */
void Motor_Init(void);
void Motor_Start(void);
void Motor_Stop(void);
void Motor_Brake(void);
void Motor_EmergencyStop(void);
void Motor_ResetFault(void);

void Motor_SetDrive(float throttle, float steer, float strafe);
void Motor_SetIndividual(uint8_t motor_id, float speed_percent);
void Motor_SetMode(DriveMode_t mode);

void Motor_UpdateRamp(uint32_t delta_ms);

MotorState_t Motor_GetState(void);
float Motor_GetThrottle(void);
float Motor_GetMotorSpeed(uint8_t motor_id);
bool Motor_IsFaultActive(void);
uint8_t Motor_GetFaultMask(void);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_CONTROL_H */
