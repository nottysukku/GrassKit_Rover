/**
  ******************************************************************************
  * @file           : bluetooth.h
  * @brief          : Bluetooth communications layer (UART Ring-Buffer + BlueNRG BLE)
  *                   for mobile phone and PC wireless control.
  ******************************************************************************
  */

#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hardware_config.h"

#define BT_RX_BUFFER_SIZE             256
#define BT_CMD_MAX_LEN                64

typedef struct {
    uint8_t rx_ring_buffer[BT_RX_BUFFER_SIZE];
    volatile uint16_t rx_head;
    volatile uint16_t rx_tail;
    char current_line[BT_CMD_MAX_LEN];
    uint8_t line_index;
    bool is_connected;
    uint32_t last_rx_timestamp_ms;
} BluetoothManager_t;

void Bluetooth_Init(void);
void Bluetooth_Process(void);
void Bluetooth_Send(const char *msg);
void Bluetooth_SendStatus(void);
bool Bluetooth_IsConnected(void);
uint32_t Bluetooth_GetTimeSinceLastCommand(void);

/* Interrupt Handlers */
void Bluetooth_UART_RxCallback(UART_HandleTypeDef *huart);

/* BlueNRG Diagnostics */
bool Bluetooth_TestSPI(char *out_buf, uint16_t out_max);
void Bluetooth_RestartAdvertising(void);

#ifdef __cplusplus
}
#endif

#endif /* __BLUETOOTH_H */
