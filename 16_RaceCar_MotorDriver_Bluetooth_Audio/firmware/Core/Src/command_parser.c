/**
  ******************************************************************************
  * @file           : command_parser.c
  * @brief          : Human-readable text protocol implementation for phone & PC.
  ******************************************************************************
  */

#include "command_parser.h"
#include "motor_control.h"
#include "engine_model.h"
#include "safety.h"
#include "bluetooth.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static void ToUpperString(char *s)
{
    while (*s) {
        *s = (char)toupper((unsigned char)*s);
        s++;
    }
}

void Command_Init(void)
{
    /* Ready */
}

void Command_Parse(char *cmd_str)
{
    if (cmd_str == NULL || strlen(cmd_str) == 0) return;

    /* Trim leading spaces */
    while (isspace((unsigned char)*cmd_str)) cmd_str++;

    char buffer[64];
    strncpy(buffer, cmd_str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    ToUpperString(buffer);

    /* Cleared for unknown or malformed lines so they cannot keep the car alive */
    bool valid = true;

    if (strcmp(buffer, "START") == 0) {
        Motor_Start();
        Bluetooth_Send("OK:START\r\n");
    }
    else if (strcmp(buffer, "STOP") == 0) {
        Motor_Stop();
        Engine_SetThrottle(0.0f);
        Bluetooth_Send("OK:STOP\r\n");
    }
    else if (strcmp(buffer, "BRAKE") == 0) {
        Motor_Brake();
        Engine_SetThrottle(0.0f);
        Bluetooth_Send("OK:BRAKE\r\n");
    }
    else if (strcmp(buffer, "ESTOP") == 0) {
        Safety_TriggerEmergencyStop();
        Bluetooth_Send("ALARM:EMERGENCY_STOP_TRIGGERED\r\n");
    }
    else if (strcmp(buffer, "RESET") == 0) {
        Safety_ResetEmergencyStop();
        Bluetooth_Send("OK:RESET\r\n");
    }
    else if (strncmp(buffer, "THROTTLE:", 9) == 0) {
        float val = (float)atof(buffer + 9);
        if (val < 0.0f) val = 0.0f;
        if (val > 100.0f) val = 100.0f;
        Motor_SetDrive(val, 0.0f, 0.0f);
        Engine_SetThrottle(val);
        Bluetooth_Send("OK:THROTTLE\r\n");
    }
    else if (strncmp(buffer, "STEER:", 6) == 0) {
        float val = (float)atof(buffer + 6);
        Motor_SetDrive(Motor_GetThrottle(), val, 0.0f);
        Bluetooth_Send("OK:STEER\r\n");
    }
    else if (strncmp(buffer, "STRAFE:", 7) == 0) {
        float val = (float)atof(buffer + 7);
        Motor_SetDrive(Motor_GetThrottle(), 0.0f, val);
        Bluetooth_Send("OK:STRAFE\r\n");
    }
    else if (strncmp(buffer, "DRIVE:", 6) == 0) {
        /* Format: DRIVE:<throttle>,<steer>,<strafe> */
        char *p = buffer + 6;
        float th = (float)atof(p);
        float st = 0.0f;
        float strf = 0.0f;
        char *comma1 = strchr(p, ',');
        if (comma1) {
            st = (float)atof(comma1 + 1);
            char *comma2 = strchr(comma1 + 1, ',');
            if (comma2) {
                strf = (float)atof(comma2 + 1);
            }
        }
        Motor_SetDrive(th, st, strf);
        Engine_SetThrottle(th);
        Bluetooth_Send("OK:DRIVE\r\n");
    }
    else if (strncmp(buffer, "GEAR:", 5) == 0) {
        char *arg = buffer + 5;
        if (strcmp(arg, "UP") == 0) {
            Engine_ShiftUp();
            Bluetooth_Send("OK:GEAR_UP\r\n");
        } else if (strcmp(arg, "DOWN") == 0) {
            Engine_ShiftDown();
            Bluetooth_Send("OK:GEAR_DOWN\r\n");
        } else if (strcmp(arg, "AUTO") == 0) {
            Engine_SetAutoGear(true);
            Bluetooth_Send("OK:GEAR_AUTO\r\n");
        } else {
            int g = atoi(arg);
            if (g >= 1 && g <= 6) {
                Engine_SetAutoGear(false);
                Engine_SetGear((uint8_t)g);
                Bluetooth_Send("OK:GEAR\r\n");
            } else {
                valid = false;
                Bluetooth_Send("ERR:INVALID_GEAR\r\n");
            }
        }
    }
    else if (strncmp(buffer, "TURBO:", 6) == 0) {
        if (strcmp(buffer + 6, "ON") == 0) {
            Engine_SetTurbo(true);
            Bluetooth_Send("OK:TURBO_ON\r\n");
        } else {
            Engine_SetTurbo(false);
            Bluetooth_Send("OK:TURBO_OFF\r\n");
        }
    }
    else if (strncmp(buffer, "REV", 3) == 0) {
        Engine_RevThrottle(1800.0f);
        Bluetooth_Send("OK:REV\r\n");
    }
    else if (strncmp(buffer, "MODE:", 5) == 0) {
        if (strcmp(buffer + 5, "MECANUM") == 0) {
            Motor_SetMode(DRIVE_MODE_MECANUM_OMNI);
            Bluetooth_Send("OK:MODE_MECANUM\r\n");
        } else if (strcmp(buffer + 5, "RACECAR") == 0) {
            Motor_SetMode(DRIVE_MODE_RACECAR);
            Bluetooth_Send("OK:MODE_RACECAR\r\n");
        } else {
            valid = false;
            Bluetooth_Send("ERR:INVALID_MODE\r\n");
        }
    }
    else if (strncmp(buffer, "MOTOR:", 6) == 0) {
        /* Format: MOTOR:<1-4>,<speed%> e.g. MOTOR:1,30 or MOTOR:4,25 */
        char *p = buffer + 6;
        int m_id = atoi(p);
        float spd = 0.0f;
        char *comma = strchr(p, ',');
        if (comma) {
            spd = (float)atof(comma + 1);
        }
        if (m_id >= 1 && m_id <= 4) {
            Motor_Start();
            Motor_SetIndividual((uint8_t)(m_id - 1), spd);
            char resp[32];
            snprintf(resp, sizeof(resp), "OK:MOTOR_%d\r\n", m_id);
            Bluetooth_Send(resp);
        } else {
            valid = false;
            Bluetooth_Send("ERR:INVALID_MOTOR_ID\r\n");
        }
    }
    else if (strcmp(buffer, "STATUS") == 0) {
        Command_SendStatus();
    }
    else if (strcmp(buffer, "BLE_TEST") == 0) {
        char test_buf[128];
        Bluetooth_TestSPI(test_buf, sizeof(test_buf));
        Bluetooth_Send(test_buf);
    }
    else if (strcmp(buffer, "BLE_ADV") == 0) {
        Bluetooth_RestartAdvertising();
    }
    else if (strcmp(buffer, "HELP") == 0) {
        Command_SendHelp();
    }
    else {
        valid = false;
        Bluetooth_Send("ERR:UNKNOWN_COMMAND\r\n");
    }

    /* Reset communication watchdog timer upon valid packet */
    if (valid) {
        Safety_KickWatchdog();
    }
}

void Command_SendStatus(void)
{
    char msg[128];
    const char *state_str = "IDLE";
    MotorState_t st = Motor_GetState();

    switch (st) {
        case MOTOR_STATE_RUNNING: state_str = "RUNNING"; break;
        case MOTOR_STATE_BRAKING: state_str = "BRAKING"; break;
        case MOTOR_STATE_FAULT:   state_str = "FAULT"; break;
        case MOTOR_STATE_ESTOP:   state_str = "ESTOP"; break;
        default:                  state_str = "IDLE"; break;
    }

    snprintf(msg, sizeof(msg),
             "STATUS:STATE=%s,RPM=%.0f,THROTTLE=%.0f,GEAR=%u,BOOST=%.2f,M1=%.0f,M2=%.0f,M3=%.0f,M4=%.0f\r\n",
             state_str,
             Engine_GetRPM(),
             Motor_GetThrottle(),
             Engine_GetGear(),
             Engine_GetBoost(),
             Motor_GetMotorSpeed(0),
             Motor_GetMotorSpeed(1),
             Motor_GetMotorSpeed(2),
             Motor_GetMotorSpeed(3));

    Bluetooth_Send(msg);
}

void Command_SendHelp(void)
{
    Bluetooth_Send("--- RACECAR COMMAND LIST ---\r\n"
                   "START, STOP, BRAKE, ESTOP, RESET\r\n"
                   "THROTTLE:<0..100>\r\n"
                   "STEER:<-100..100>\r\n"
                   "STRAFE:<-100..100>\r\n"
                   "DRIVE:<th,st,strf>\r\n"
                   "GEAR:<1..6|AUTO|UP|DOWN>\r\n"
                   "TURBO:<ON|OFF>\r\n"
                   "REV\r\n"
                   "MODE:<RACECAR|MECANUM>\r\n"
                   "STATUS\r\n");
}
