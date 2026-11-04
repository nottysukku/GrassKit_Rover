/**
  ******************************************************************************
  * @file           : wifi_st67w61.h
  * @brief          : Header file for ST67W611M1 Wi-Fi 6 Coprocessor Driver
  *                   (X-NUCLEO-67W61M1) over SPI1 with SoftAP & TCP Server.
  ******************************************************************************
  */

#ifndef __WIFI_ST67W61_H
#define __WIFI_ST67W61_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l0xx_hal.h"
#include "hardware_config.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    WIFI_STATE_UNINITIALIZED = 0,
    WIFI_STATE_RESETTING,
    WIFI_STATE_WAIT_RDY,
    WIFI_STATE_INIT_AT,
    WIFI_STATE_CONFIG_SOFTAP,
    WIFI_STATE_START_SERVER,
    WIFI_STATE_SERVER_READY,
    WIFI_STATE_CLIENT_CONNECTED,
    WIFI_STATE_ERROR
} WiFi_State_t;

typedef struct {
    WiFi_State_t state;
    char ssid[32];
    char password[32];
    uint8_t channel;
    uint16_t port;
    char ip_addr[16];
    uint8_t connected_clients;
    uint8_t active_link_id;
    uint32_t rx_packet_count;
    uint32_t tx_packet_count;
    uint32_t last_rx_time_ms;
    uint32_t state_entry_time_ms;
} WiFi_Manager_t;

/* Public API */
void WiFi_Init(void);
void WiFi_Process(void);
bool WiFi_IsClientConnected(void);
bool WiFi_SendPacket(uint8_t link_id, const char *data, uint16_t length);
void WiFi_BroadcastTelemetry(const char *data, uint16_t length);
const char* WiFi_GetStateString(void);
WiFi_State_t WiFi_GetState(void);
void WiFi_RunDiagnostic(void);

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_ST67W61_H */
