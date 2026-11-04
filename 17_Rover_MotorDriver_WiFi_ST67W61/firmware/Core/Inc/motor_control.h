/**
  ******************************************************************************
  * @file           : motor_control.h
  * @brief          : Motor control abstraction and differential/skid-steer kinematics
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
    DRIVE_MODE_DIFFERENTIAL = 0, /* Differential / Skid-Steer Tank Drive */
    DRIVE_MODE_MECANUM_OMNI      /* 4-Wheel Mecanum Omnidirectional */
} DriveMode_t;

#define MOTOR_TRIM_MIN_PCT   50U
#define MOTOR_TRIM_MAX_PCT   150U

typedef struct {
    float target_speed;          /* Target speed percentage: -100.0f to +100.0f (+ = wheel rolls forward) */
    float current_speed;         /* Current ramped speed percentage */
    uint16_t pwm_duty;           /* Applied timer PWM duty cycle ticks */
    uint8_t direction;           /* Level written to the PH (direction) pin */
    bool invert_dir;             /* true: forward = PH low */
    uint8_t trim_pct;            /* Drive-command scale for this wheel, 50..150 % */
} MotorChannel_t;

typedef struct {
    MotorState_t state;
    DriveMode_t drive_mode;
    bool strafe_flip;            /* Mirror sideways moves (mecanum rollers mounted the other way) */
    bool link_sides;             /* Front wheel direction follows the rear wheel on the same side */
    MotorChannel_t motors[4];    /* Index 0: FL, 1: FR, 2: RL, 3: RR */
    float overall_throttle;      /* -100.0f to 100.0f */
    float overall_steer;         /* -100.0f to 100.0f */
    float overall_strafe;        /* -100.0f to 100.0f */
    bool fault_detected;
    bool estop_active;
    bool bridges_enabled;        /* EN/nFAULT lines released (power stages active) */
    uint8_t fault_mask;          /* Bit n set = driver fault on motor n+1 */
    uint8_t fault_samples[4];    /* Low-sample counters for EN/nFAULT debounce */
    uint32_t fault_blanking_ms;
} MotorControl_t;

/* Public API */
void Motor_Init(void);
void Motor_Start(void);
void Motor_Stop(void);
void Motor_Brake(void);
void Motor_EmergencyStop(void);
void Motor_ResetFault(void);

/* throttle: + forward; turn: + clockwise (seen from above); strafe: + to the right */
void Motor_SetDrive(float throttle, float turn, float strafe);
void Motor_SetIndividual(uint8_t motor_id, float speed_percent);
void Motor_SetWheels(const float speed_percent[4]);   /* FL, FR, RL, RR; no trim applied */
void Motor_SetMode(DriveMode_t mode);
DriveMode_t Motor_GetMode(void);
void Motor_SetInvert(uint8_t motor_id, bool invert);
bool Motor_GetInvert(uint8_t motor_id);
void Motor_SetTrim(uint8_t motor_id, uint8_t trim_pct);
uint8_t Motor_GetTrim(uint8_t motor_id);
void Motor_SetStrafeFlip(bool flip);
bool Motor_GetStrafeFlip(void);
/* true: FL and RL share one direction setting, FR and RR share another.
 * Turning it on copies each rear wheel's setting to the front wheel beside it. */
void Motor_SetLinkSides(bool link);
bool Motor_GetLinkSides(void);
uint8_t Motor_GetDutyPercent(uint8_t motor_id);
uint8_t Motor_GetDirPin(uint8_t motor_id);

/* Wheel settings (flip, trim, mode, strafe flip, side link) kept in the MCU data EEPROM */
void Motor_LoadDefaults(void);
bool Motor_SaveSettings(void);

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
