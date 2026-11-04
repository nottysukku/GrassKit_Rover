/**
  ******************************************************************************
  * @file           : command_parser.c
  * @brief          : Human-readable text protocol implementation for phone & PC
  *                   controlling the rover over Wi-Fi.
  ******************************************************************************
  */

#include "command_parser.h"
#include "motor_control.h"
#include "safety.h"
#include "wifi_st67w61.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#define DEFAULT_CRUISE_THROTTLE       45.0f
#define DEFAULT_SPIN_STEER            50.0f

static void ToUpperString(char *s)
{
    while (*s) {
        *s = (char)toupper((unsigned char)*s);
        s++;
    }
}

static void SendReply(const char *msg)
{
    Debug_Log("%s", msg);
    WiFi_BroadcastTelemetry(msg, (uint16_t)strlen(msg));
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

    /* Trim trailing carriage returns, newlines, spaces */
    char *end = cmd_str + strlen(cmd_str) - 1;
    while (end >= cmd_str && (*end == '\r' || *end == '\n' || isspace((unsigned char)*end))) {
        *end = '\0';
        end--;
    }

    if (strlen(cmd_str) == 0) return;

    char buffer[64];
    strncpy(buffer, cmd_str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    ToUpperString(buffer);

    bool valid = true;

    /* Navigation Commands */
    if (strcmp(buffer, "FORWARD") == 0 || strcmp(buffer, "F") == 0) {
        Motor_SetDrive(DEFAULT_CRUISE_THROTTLE, 0.0f, 0.0f);
        SendReply("OK:FORWARD\r\n");
    }
    else if (strcmp(buffer, "BACKWARD") == 0 || strcmp(buffer, "B") == 0) {
        Motor_SetDrive(-DEFAULT_CRUISE_THROTTLE, 0.0f, 0.0f);
        SendReply("OK:BACKWARD\r\n");
    }
    else if (strcmp(buffer, "LEFT") == 0 || strcmp(buffer, "L") == 0) {
        /* Skid-steer spin left */
        Motor_SetDrive(0.0f, -DEFAULT_SPIN_STEER, 0.0f);
        SendReply("OK:LEFT\r\n");
    }
    else if (strcmp(buffer, "RIGHT") == 0 || strcmp(buffer, "R") == 0) {
        /* Skid-steer spin right */
        Motor_SetDrive(0.0f, DEFAULT_SPIN_STEER, 0.0f);
        SendReply("OK:RIGHT\r\n");
    }
    else if (strcmp(buffer, "STOP") == 0 || strcmp(buffer, "S") == 0) {
        Motor_Stop();
        SendReply("OK:STOP\r\n");
    }
    else if (strcmp(buffer, "BRAKE") == 0) {
        Motor_Brake();
        SendReply("OK:BRAKE\r\n");
    }
    else if (strcmp(buffer, "START") == 0) {
        Motor_Start();
        SendReply("OK:START\r\n");
    }
    else if (strcmp(buffer, "ESTOP") == 0) {
        Safety_TriggerEmergencyStop();
        SendReply("ALARM:EMERGENCY_STOP_TRIGGERED\r\n");
    }
    else if (strcmp(buffer, "RESET") == 0) {
        Safety_ResetEmergencyStop();
        SendReply("OK:RESET\r\n");
    }
    /* Proportional Drive Commands */
    else if (strncmp(buffer, "THROTTLE:", 9) == 0) {
        float val = (float)atof(buffer + 9);
        Motor_SetDrive(val, 0.0f, 0.0f);
        SendReply("OK:THROTTLE\r\n");
    }
    else if (strncmp(buffer, "STEER:", 6) == 0) {
        float val = (float)atof(buffer + 6);
        Motor_SetDrive(Motor_GetThrottle(), val, 0.0f);
        SendReply("OK:STEER\r\n");
    }
    else if (strncmp(buffer, "STRAFE:", 7) == 0) {
        float val = (float)atof(buffer + 7);
        Motor_SetDrive(Motor_GetThrottle(), 0.0f, val);
        SendReply("OK:STRAFE\r\n");
    }
    else if (strncmp(buffer, "DRIVE:", 6) == 0) {
        /* Format: DRIVE:<throttle>,<steer> or DRIVE:<throttle>,<steer>,<strafe> */
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
        SendReply("OK:DRIVE\r\n");
    }
    else if (strncmp(buffer, "MOTOR:", 6) == 0) {
        /* Format: MOTOR:<id>,<speed> */
        char *p = buffer + 6;
        int id = atoi(p);
        float spd = 0.0f;
        char *comma = strchr(p, ',');
        if (comma) {
            spd = (float)atof(comma + 1);
        }
        Motor_SetIndividual((uint8_t)id, spd);
        SendReply("OK:MOTOR\r\n");
    }
    else if (strncmp(buffer, "WHEELS:", 7) == 0) {
        /* Format: WHEELS:<FL>,<FR>,<RL>,<RR> in percent, + = wheel forward */
        float w[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        char *p = buffer + 7;
        for (int i = 0; i < 4 && p != NULL; i++) {
            w[i] = (float)atof(p);
            p = strchr(p, ',');
            if (p != NULL) p++;
        }
        Motor_SetWheels(w);
        SendReply("OK:WHEELS\r\n");
    }
    else if (strncmp(buffer, "MODE:", 5) == 0) {
        char *m = buffer + 5;
        if (strstr(m, "MEC") != NULL || strstr(m, "OMNI") != NULL) {
            Motor_SetMode(DRIVE_MODE_MECANUM_OMNI);
            SendReply("OK:MODE:MECANUM\r\n");
        } else {
            Motor_SetMode(DRIVE_MODE_DIFFERENTIAL);
            SendReply("OK:MODE:TANK\r\n");
        }
    }
    else if (strncmp(buffer, "INVERT:", 7) == 0) {
        int id = atoi(buffer + 7);
        if (id >= 1 && id <= 4) {
            bool inv = !Motor_GetInvert((uint8_t)id);
            Motor_SetInvert((uint8_t)id, inv);
            SendReply(inv ? "OK:INVERT:ON\r\n" : "OK:INVERT:OFF\r\n");
        } else {
            SendReply("ERR:INVERT_ID\r\n");
        }
    }
    else if (strncmp(buffer, "FLIP:", 5) == 0) {
        /* Format: FLIP:<id>,<0|1> */
        int id = atoi(buffer + 5);
        char *comma = strchr(buffer + 5, ',');
        if (id >= 1 && id <= 4 && comma != NULL) {
            Motor_SetInvert((uint8_t)id, atoi(comma + 1) != 0);
            if (Motor_GetLinkSides()) {
                SendReply((id == 1 || id == 3) ? "OK:FLIP Front left + Rear left\r\n"
                                               : "OK:FLIP Front right + Rear right\r\n");
            } else {
                SendReply("OK:FLIP\r\n");
            }
        } else {
            SendReply("ERR:FLIP\r\n");
        }
    }
    else if (strncmp(buffer, "TRIM:", 5) == 0) {
        /* Format: TRIM:<id>,<50..150> */
        int id = atoi(buffer + 5);
        char *comma = strchr(buffer + 5, ',');
        if (id >= 1 && id <= 4 && comma != NULL) {
            int pct = atoi(comma + 1);
            if (pct < 0) pct = 0;
            if (pct > 255) pct = 255;
            Motor_SetTrim((uint8_t)id, (uint8_t)pct);
            SendReply("OK:TRIM\r\n");
        } else {
            SendReply("ERR:TRIM\r\n");
        }
    }
    else if (strncmp(buffer, "SFLIP:", 6) == 0) {
        Motor_SetStrafeFlip(atoi(buffer + 6) != 0);
        SendReply("OK:SFLIP\r\n");
    }
    else if (strncmp(buffer, "LINK:", 5) == 0) {
        /* LINK:1 = front wheels follow the rear wheel on the same side */
        bool link = atoi(buffer + 5) != 0;
        Motor_SetLinkSides(link);
        SendReply(link ? "OK:LINK:ON (fronts follow rears)\r\n" : "OK:LINK:OFF\r\n");
    }
    else if (strcmp(buffer, "SAVE") == 0) {
        SendReply(Motor_SaveSettings() ? "OK:SAVED\r\n" : "ERR:SAVE_FAILED\r\n");
    }
    else if (strcmp(buffer, "DEFAULTS") == 0) {
        Motor_LoadDefaults();
        SendReply("OK:DEFAULTS (not saved yet)\r\n");
    }
    else if (strncmp(buffer, "GEAR:", 5) == 0 || strncmp(buffer, "TURBO:", 6) == 0 || strcmp(buffer, "REV") == 0) {
        SendReply("OK:FX\r\n");
    }
    else if (strcmp(buffer, "PING") == 0) {
        SendReply("PONG:OK\r\n");
    }
    else if (strcmp(buffer, "WIFI") == 0) {
        WiFi_Init();
        SendReply("OK:WIFI_RESTART\r\n");
    }
    else if (strcmp(buffer, "WIFIDIAG") == 0) {
        WiFi_RunDiagnostic();
        SendReply("OK:WIFIDIAG_DONE\r\n");
    }
    else if (strcmp(buffer, "STATUS") == 0) {
        Command_SendStatus();
    }
    else if (strcmp(buffer, "HELP") == 0) {
        Command_SendHelp();
    }
    else {
        valid = false;
        SendReply("ERR:UNKNOWN_COMMAND\r\n");
    }

    if (valid) {
        Safety_FeedCommunicationWatchdog();
    }
}

void Command_SendStatus(void)
{
    char status_str[192];
    const char *state_str = "UNKNOWN";

    switch (Motor_GetState()) {
        case MOTOR_STATE_DISABLED: state_str = "DISABLED"; break;
        case MOTOR_STATE_IDLE:     state_str = "IDLE"; break;
        case MOTOR_STATE_RUNNING:  state_str = "RUNNING"; break;
        case MOTOR_STATE_BRAKING:  state_str = "BRAKING"; break;
        case MOTOR_STATE_FAULT:    state_str = "FAULT"; break;
        case MOTOR_STATE_ESTOP:    state_str = "ESTOP"; break;
    }

    float m1 = Motor_GetMotorSpeed(1);
    float m2 = Motor_GetMotorSpeed(2);
    float m3 = Motor_GetMotorSpeed(3);
    float m4 = Motor_GetMotorSpeed(4);
    float avg_spd = (fabsf(m1) + fabsf(m2) + fabsf(m3) + fabsf(m4)) * 0.25f;
    float virtual_rpm = 900.0f + (avg_spd * 61.0f);

    snprintf(status_str, sizeof(status_str),
             "STATUS:STATE=%s,RPM=%.0f,THROTTLE=%.0f,GEAR=1,BOOST=0.00,M1=%.0f,M2=%.0f,M3=%.0f,M4=%.0f,FAULTS=%u,UPTIME=%lu\r\n",
             state_str,
             virtual_rpm,
             fabsf(Motor_GetThrottle()),
             m1, m2, m3, m4,
             Motor_GetFaultMask(),
             HAL_GetTick());

    SendReply(status_str);
}

static const char *StateName(void)
{
    switch (Motor_GetState()) {
        case MOTOR_STATE_DISABLED: return "DISABLED";
        case MOTOR_STATE_IDLE:     return "IDLE";
        case MOTOR_STATE_RUNNING:  return "RUNNING";
        case MOTOR_STATE_BRAKING:  return "BRAKING";
        case MOTOR_STATE_FAULT:    return "FAULT";
        case MOTOR_STATE_ESTOP:    return "ESTOP";
        default:                   return "UNKNOWN";
    }
}

/* Compact status line for the web page:
 * W:st=<state>;f=<fault mask>;md=<M|T>;sf=<0|1>;lk=<0|1>;w=<speed x4>;d=<duty % x4>;ph=<pin x4>;fl=<flip x4>;tr=<trim x4> */
void Command_SendWebStatus(void)
{
    char s[200];
    int n = snprintf(s, sizeof(s), "W:st=%s;f=%u;md=%c;sf=%u;lk=%u;w=", StateName(), Motor_GetFaultMask(),
                     (Motor_GetMode() == DRIVE_MODE_MECANUM_OMNI) ? 'M' : 'T', Motor_GetStrafeFlip() ? 1U : 0U,
                     Motor_GetLinkSides() ? 1U : 0U);
    for (uint8_t i = 1; i <= 4 && n > 0 && n < (int)sizeof(s); i++) {
        n += snprintf(s + n, sizeof(s) - (size_t)n, "%d%s", (int)lroundf(Motor_GetMotorSpeed(i)), (i < 4) ? "," : ";d=");
    }
    for (uint8_t i = 1; i <= 4 && n > 0 && n < (int)sizeof(s); i++) {
        n += snprintf(s + n, sizeof(s) - (size_t)n, "%u%s", Motor_GetDutyPercent(i), (i < 4) ? "," : ";ph=");
    }
    for (uint8_t i = 1; i <= 4 && n > 0 && n < (int)sizeof(s); i++) {
        n += snprintf(s + n, sizeof(s) - (size_t)n, "%u%s", Motor_GetDirPin(i), (i < 4) ? "," : ";fl=");
    }
    for (uint8_t i = 1; i <= 4 && n > 0 && n < (int)sizeof(s); i++) {
        n += snprintf(s + n, sizeof(s) - (size_t)n, "%u%s", Motor_GetInvert(i) ? 1U : 0U, (i < 4) ? "," : ";tr=");
    }
    for (uint8_t i = 1; i <= 4 && n > 0 && n < (int)sizeof(s); i++) {
        n += snprintf(s + n, sizeof(s) - (size_t)n, "%u%s", Motor_GetTrim(i), (i < 4) ? "," : "\r\n");
    }
    if (n > 0) {
        WiFi_BroadcastTelemetry(s, (uint16_t)strlen(s));
    }
}

void Command_SendHelp(void)
{
    SendReply("HELP: FORWARD, BACKWARD, LEFT, RIGHT, STOP, BRAKE, DRIVE:<th,turn,strafe>, WHEELS:<fl,fr,rl,rr>, "
              "MOTOR:<id,spd>, MODE:<MECANUM|TANK>, FLIP:<id,0|1>, LINK:<0|1>, TRIM:<id,50..150>, SFLIP:<0|1>, SAVE, DEFAULTS, "
              "ESTOP, RESET, STATUS, PING\r\n");
}
