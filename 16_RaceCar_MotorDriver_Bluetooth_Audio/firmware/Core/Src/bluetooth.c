/**
******************************************************************************
* @file           : bluetooth.c
* @brief          : Bluetooth communications implementation with non-blocking
*                   ring-buffer UART RX and BlueNRG-M0 / SPBTLE-1S ACI SPI engine.
******************************************************************************
*/

#include "bluetooth.h"
#include "command_parser.h"
#include <string.h>
#include <stdio.h>

static BluetoothManager_t g_bt;

UART_HandleTypeDef huart1_bt;     /* Aux / External Bluetooth Module (PA9/PA10) */
UART_HandleTypeDef huart2_debug;  /* ST-LINK VCP Debug / PC Control (PA2/PA3) */
static SPI_HandleTypeDef  hspi1_bluenrg; /* X-NUCLEO-IDB05A2 BlueNRG SPI (PA5/6/7) */

static uint8_t s_rx_byte_uart1 = 0;
static uint8_t s_rx_byte_uart2 = 0;
static uint8_t s_st_reset = 0xFF, s_st_gatt = 0xFF, s_st_gap = 0xFF, s_st_adv = 0xFF;

/* Hardware Initializations */
static void Bluetooth_UART1_Init(void);
static void Bluetooth_UART2_Init(void);
static void Bluetooth_BlueNRG_SPI_Init(void);
static void Bluetooth_BlueNRG_Start_Advertising(void);
static void Bluetooth_BlueNRG_Poll(void);

void Bluetooth_Init(void)
{
memset(&g_bt, 0, sizeof(g_bt));
g_bt.last_rx_timestamp_ms = HAL_GetTick();
g_bt.is_connected = false;

Bluetooth_UART2_Init();
Bluetooth_UART1_Init();
Bluetooth_BlueNRG_SPI_Init();

/* Start non-blocking 1-byte interrupt receptions */
HAL_UART_Receive_IT(&huart2_debug, &s_rx_byte_uart2, 1);
HAL_UART_Receive_IT(&huart1_bt, &s_rx_byte_uart1, 1);

/* Start BlueNRG Advertising */
Bluetooth_BlueNRG_Start_Advertising();
}

static void Bluetooth_UART1_Init(void)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};

BT_UART_CLK_ENABLE();
__HAL_RCC_GPIOA_CLK_ENABLE();

/* PA9 -> USART1_TX, PA10 -> USART1_RX */
GPIO_InitStruct.Pin = BT_UART_TX_PIN | BT_UART_RX_PIN;
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
GPIO_InitStruct.Pull = GPIO_PULLUP;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
GPIO_InitStruct.Alternate = GPIO_AF4_USART1;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

huart1_bt.Instance = BT_UART_INSTANCE;
huart1_bt.Init.BaudRate = BT_UART_BAUDRATE;
huart1_bt.Init.WordLength = UART_WORDLENGTH_8B;
huart1_bt.Init.StopBits = UART_STOPBITS_1;
huart1_bt.Init.Parity = UART_PARITY_NONE;
huart1_bt.Init.Mode = UART_MODE_TX_RX;
huart1_bt.Init.HwFlowCtl = UART_HWCONTROL_NONE;
huart1_bt.Init.OverSampling = UART_OVERSAMPLING_16;
/* Overrun detection off: on an overrun the HAL aborts reception for good */
huart1_bt.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
huart1_bt.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
HAL_UART_Init(&huart1_bt);

HAL_NVIC_SetPriority(USART1_IRQn, 2, 0);
HAL_NVIC_EnableIRQ(USART1_IRQn);
}

static void Bluetooth_UART2_Init(void)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};

DEBUG_UART_CLK_ENABLE();
__HAL_RCC_GPIOA_CLK_ENABLE();

/* PA2 -> USART2_TX, PA3 -> USART2_RX */
GPIO_InitStruct.Pin = DEBUG_UART_TX_PIN | DEBUG_UART_RX_PIN;
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
GPIO_InitStruct.Pull = GPIO_PULLUP;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

huart2_debug.Instance = DEBUG_UART_INSTANCE;
huart2_debug.Init.BaudRate = DEBUG_UART_BAUDRATE;
huart2_debug.Init.WordLength = UART_WORDLENGTH_8B;
huart2_debug.Init.StopBits = UART_STOPBITS_1;
huart2_debug.Init.Parity = UART_PARITY_NONE;
huart2_debug.Init.Mode = UART_MODE_TX_RX;
huart2_debug.Init.HwFlowCtl = UART_HWCONTROL_NONE;
huart2_debug.Init.OverSampling = UART_OVERSAMPLING_16;
/* Overrun detection off: on an overrun the HAL aborts reception for good */
huart2_debug.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
huart2_debug.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
HAL_UART_Init(&huart2_debug);

HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
HAL_NVIC_EnableIRQ(USART2_IRQn);
}

static void Bluetooth_BlueNRG_SPI_Init(void)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};

IDB05A2_SPI_CLK_ENABLE();
__HAL_RCC_GPIOA_CLK_ENABLE();
__HAL_RCC_GPIOB_CLK_ENABLE();
__HAL_RCC_GPIOC_CLK_ENABLE();

/* SPI1 Pins on GPIOA: PA5 (SCK alt), PA6 (MISO), PA7 (MOSI) */
GPIO_InitStruct.Pin = IDB05A2_SCK_PIN | IDB05A2_MISO_PIN | IDB05A2_MOSI_PIN;
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* SPI1 SCK on GPIOB: PB3 (Arduino D3 - default factory clock pin for IDB05A1/IDB05A2!) */
GPIO_InitStruct.Pin = GPIO_PIN_3;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* BlueNRG Primary CS (PA1 / A1) and Secondary CS (PB6 / D10) */
GPIO_InitStruct.Pin = IDB05A2_CS_PIN;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
HAL_GPIO_WritePin(GPIOA, IDB05A2_CS_PIN, GPIO_PIN_SET);

GPIO_InitStruct.Pin = GPIO_PIN_6; /* PB6 (Arduino D10) */
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

/* BlueNRG RST (PA8 / D7) */
GPIO_InitStruct.Pin = IDB05A2_RST_PIN;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
HAL_GPIO_WritePin(GPIOA, IDB05A2_RST_PIN, GPIO_PIN_SET);

/* BlueNRG IRQ: PA0 (A0) and PC7 (D9) Inputs */
GPIO_InitStruct.Pin = IDB05A2_IRQ_PIN;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = GPIO_PULLDOWN;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

GPIO_InitStruct.Pin = GPIO_PIN_7; /* PC7 (D9) */
HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

/* Configure SPI1 Peripheral for BlueNRG */
hspi1_bluenrg.Instance = IDB05A2_SPI_INSTANCE;
hspi1_bluenrg.Init.Mode = SPI_MODE_MASTER;
hspi1_bluenrg.Init.Direction = SPI_DIRECTION_2LINES;
hspi1_bluenrg.Init.DataSize = SPI_DATASIZE_8BIT;
hspi1_bluenrg.Init.CLKPolarity = SPI_POLARITY_LOW;
hspi1_bluenrg.Init.CLKPhase = SPI_PHASE_1EDGE;
hspi1_bluenrg.Init.NSS = SPI_NSS_SOFT;
hspi1_bluenrg.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16; /* 32/16 = 2 MHz (ultra-stable) */
hspi1_bluenrg.Init.FirstBit = SPI_FIRSTBIT_MSB;
HAL_SPI_Init(&hspi1_bluenrg);
}

static inline void BlueNRG_CS_Low(void)
{
HAL_GPIO_WritePin(GPIOA, IDB05A2_CS_PIN, GPIO_PIN_RESET);
}

static inline void BlueNRG_CS_High(void)
{
HAL_GPIO_WritePin(GPIOA, IDB05A2_CS_PIN, GPIO_PIN_SET);
}

static inline void RingBuffer_Push(uint8_t byte)
{
uint16_t next_head = (g_bt.rx_head + 1) % BT_RX_BUFFER_SIZE;
if (next_head != g_bt.rx_tail) {
g_bt.rx_ring_buffer[g_bt.rx_head] = byte;
g_bt.rx_head = next_head;
}
}

static inline bool RingBuffer_Pop(uint8_t *p_byte)
{
if (g_bt.rx_head == g_bt.rx_tail) {
return false;
}
*p_byte = g_bt.rx_ring_buffer[g_bt.rx_tail];
g_bt.rx_tail = (g_bt.rx_tail + 1) % BT_RX_BUFFER_SIZE;
return true;
}

static uint8_t s_dbg_rd_c0 = 0, s_dbg_rd_c1 = 0, s_dbg_rd_c3 = 0;
static uint8_t s_dbg_wr_c0 = 0, s_dbg_wr_c1 = 0, s_dbg_wr_c3 = 0;

static bool BlueNRG_Read_Packet(uint8_t *p_buf, uint8_t max_len, uint8_t *p_rx_len)
{
uint8_t hdr_tx[5] = {0x0B, 0x00, 0x00, 0x00, 0x00};
uint8_t hdr_rx[5] = {0};

if (p_rx_len) *p_rx_len = 0;

for (int retry = 0; retry < 50; retry++) {
BlueNRG_CS_Low();
for (volatile int d = 0; d < 800; d++); /* >50us CS setup delay */

HAL_SPI_TransmitReceive(&hspi1_bluenrg, hdr_tx, hdr_rx, 5, 20);

s_dbg_rd_c0 = hdr_rx[0];
s_dbg_rd_c1 = hdr_rx[1];
s_dbg_rd_c3 = hdr_rx[3];

if (hdr_rx[0] == 0x02) {
uint16_t total_len = ((uint16_t)hdr_rx[4] << 8) | hdr_rx[3];
if (total_len > 0) {
if (total_len > 128) total_len = 128;
uint8_t dummy_tx[128] = {0};
uint8_t rx_data[128] = {0};

HAL_SPI_TransmitReceive(&hspi1_bluenrg, dummy_tx, rx_data, (uint16_t)total_len, 30);

uint8_t copy_len = (total_len > max_len) ? max_len : (uint8_t)total_len;
if (p_buf && copy_len > 0) {
memcpy(p_buf, rx_data, copy_len);
}
if (p_rx_len) {
*p_rx_len = copy_len;
}
for (volatile int d = 0; d < 300; d++);
BlueNRG_CS_High();
for (volatile int d = 0; d < 500; d++);
return true;
} else {
/* BlueNRG is ready but 0 bytes available */
for (volatile int d = 0; d < 300; d++);
BlueNRG_CS_High();
for (volatile int d = 0; d < 500; d++);
return false;
}
}

BlueNRG_CS_High();
for (volatile int d = 0; d < 800; d++);
HAL_Delay(2);
}

return false;
}

static bool BlueNRG_Send_Packet(const uint8_t *p_data, uint16_t len)
{
uint8_t hdr_tx[5] = {0x0A, 0x00, 0x00, 0x00, 0x00};
uint8_t hdr_rx[5] = {0};

/* If IRQ is set, read pending events first to free BlueNRG buffer */
for (int d = 0; d < 5; d++) {
if (HAL_GPIO_ReadPin(GPIOA, IDB05A2_IRQ_PIN) != GPIO_PIN_SET) break;
uint8_t drain[64];
uint8_t dlen = 0;
BlueNRG_Read_Packet(drain, sizeof(drain), &dlen);
HAL_Delay(2);
}

for (int retry = 0; retry < 50; retry++) {
BlueNRG_CS_Low();
for (volatile int d = 0; d < 800; d++); /* >50us CS setup delay */

HAL_SPI_TransmitReceive(&hspi1_bluenrg, hdr_tx, hdr_rx, 5, 20);

s_dbg_wr_c0 = hdr_rx[0];
s_dbg_wr_c1 = hdr_rx[1];
s_dbg_wr_c3 = hdr_rx[3];

if (hdr_rx[0] == 0x02 && hdr_rx[1] >= len) {
HAL_SPI_Transmit(&hspi1_bluenrg, (uint8_t *)p_data, len, 50);
for (volatile int d = 0; d < 300; d++);
BlueNRG_CS_High();
for (volatile int d = 0; d < 500; d++);
return true;
}

BlueNRG_CS_High();
for (volatile int d = 0; d < 800; d++);

if (HAL_GPIO_ReadPin(GPIOA, IDB05A2_IRQ_PIN) == GPIO_PIN_SET) {
uint8_t drain[64];
uint8_t dlen = 0;
BlueNRG_Read_Packet(drain, sizeof(drain), &dlen);
}

HAL_Delay(2);
}
return false;
}

static bool BlueNRG_Execute_Command(const uint8_t *cmd, uint16_t len, uint8_t *p_status)
{
if (!BlueNRG_Send_Packet(cmd, len)) {
if (p_status) *p_status = 0xFF;
return false;
}

/* Wait up to 250ms for IRQ to go HIGH indicating response event */
uint32_t t0 = HAL_GetTick();
while (HAL_GPIO_ReadPin(GPIOA, IDB05A2_IRQ_PIN) != GPIO_PIN_SET) {
if ((HAL_GetTick() - t0) > 250) {
if (p_status) *p_status = 0xFD;
return false;
}
HAL_Delay(2);
}

/* Drain events until we get the response */
bool found = false;
uint8_t status_val = 0xFE;
while (HAL_GPIO_ReadPin(GPIOA, IDB05A2_IRQ_PIN) == GPIO_PIN_SET) {
uint8_t evt[64] = {0};
uint8_t evt_len = 0;
if (!BlueNRG_Read_Packet(evt, sizeof(evt), &evt_len)) {
break;
}
/* Check for Command Complete (0x0E) */
if (evt_len >= 7 && evt[0] == 0x04 && evt[1] == 0x0E) {
status_val = evt[6];
found = true;
}
/* Check for Command Status (0x0F) */
else if (evt_len >= 6 && evt[0] == 0x04 && evt[1] == 0x0F) {
status_val = evt[2];
found = true;
}
}

if (p_status) *p_status = status_val;
return (found && status_val == 0x00);
}

static void Bluetooth_BlueNRG_Start_Advertising(void)
{
/* 1. Hardware Reset: Toggle RST pin LOW then HIGH */
HAL_GPIO_WritePin(GPIOA, IDB05A2_RST_PIN, GPIO_PIN_RESET);
HAL_Delay(25);
HAL_GPIO_WritePin(GPIOA, IDB05A2_RST_PIN, GPIO_PIN_SET);
HAL_Delay(120);

/* 2. Wait up to 300ms for boot event (IRQ -> HIGH) */
uint32_t t0 = HAL_GetTick();
while (HAL_GPIO_ReadPin(GPIOA, IDB05A2_IRQ_PIN) != GPIO_PIN_SET) {
if ((HAL_GetTick() - t0) > 300) break;
HAL_Delay(5);
}

/* Drain initial boot event(s) */
while (HAL_GPIO_ReadPin(GPIOA, IDB05A2_IRQ_PIN) == GPIO_PIN_SET) {
uint8_t boot_evt[32] = {0};
uint8_t boot_len = 0;
if (!BlueNRG_Read_Packet(boot_evt, sizeof(boot_evt), &boot_len)) break;
}

/* 3. Command: HCI_RESET */
const uint8_t hci_reset[] = {0x01, 0x03, 0x0C, 0x00};
BlueNRG_Execute_Command(hci_reset, sizeof(hci_reset), &s_st_reset);
HAL_Delay(30);

/* Drain any leftover events */
while (HAL_GPIO_ReadPin(GPIOA, IDB05A2_IRQ_PIN) == GPIO_PIN_SET) {
uint8_t evt[32] = {0};
uint8_t evt_len = 0;
if (!BlueNRG_Read_Packet(evt, sizeof(evt), &evt_len)) break;
}

/* 4. Command: ACI_GATT_INIT */
const uint8_t aci_gatt_init[] = {0x01, 0x01, 0xFD, 0x00};
BlueNRG_Execute_Command(aci_gatt_init, sizeof(aci_gatt_init), &s_st_gatt);
HAL_Delay(20);

/* 5. Command: ACI_GAP_INIT (Opcode 0xFC8A) */
const uint8_t aci_gap_init[] = {0x01, 0x8A, 0xFC, 0x03, 0x01, 0x00, 0x0C};
BlueNRG_Execute_Command(aci_gap_init, sizeof(aci_gap_init), &s_st_gap);
HAL_Delay(20);

/* 6. Command: ACI_GAP_SET_DISCOVERABLE (Opcode 0xFC83) */
const uint8_t aci_set_discoverable[] = {
0x01, 0x83, 0xFC, 0x18,
0x00,                   /* ADV_IND */
0xA0, 0x00,             /* Min Interval 100 ms */
0xA0, 0x00,             /* Max Interval 100 ms */
0x00,                   /* Public Address */
0x00,                   /* Filter policy */
0x0D,                   /* Local Name Length: 13 bytes */
0x09,                   /* AD_TYPE_COMPLETE_LOCAL_NAME */
'R', 'A', 'C', 'E', 'C', 'A', 'R', '-', 'L', '0', '7', '3',
0x00,                   /* Service UUID length */
0x00, 0x00,             /* Slave connection interval min */
0x00, 0x00              /* Slave connection interval max */
};
BlueNRG_Execute_Command(aci_set_discoverable, sizeof(aci_set_discoverable), &s_st_adv);
}

static void Bluetooth_BlueNRG_Poll(void)
{
if (HAL_GPIO_ReadPin(GPIOA, IDB05A2_IRQ_PIN) == GPIO_PIN_SET) {
uint8_t rx_buf[64] = {0};
uint8_t rx_len = 0;
if (BlueNRG_Read_Packet(rx_buf, sizeof(rx_buf), &rx_len)) {
for (uint8_t i = 0; i < rx_len; i++) {
if (rx_buf[i] >= 32 && rx_buf[i] <= 126) {
RingBuffer_Push(rx_buf[i]);
} else if (rx_buf[i] == '\r' || rx_buf[i] == '\n') {
RingBuffer_Push('\n');
}
}
}
}
}

void Bluetooth_UART_RxCallback(UART_HandleTypeDef *huart)
{
if (huart->Instance == USART2) {
RingBuffer_Push(s_rx_byte_uart2);
g_bt.last_rx_timestamp_ms = HAL_GetTick();
g_bt.is_connected = true;
HAL_UART_Receive_IT(&huart2_debug, &s_rx_byte_uart2, 1);
} else if (huart->Instance == USART1) {
RingBuffer_Push(s_rx_byte_uart1);
g_bt.last_rx_timestamp_ms = HAL_GetTick();
g_bt.is_connected = true;
HAL_UART_Receive_IT(&huart1_bt, &s_rx_byte_uart1, 1);
}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
Bluetooth_UART_RxCallback(huart);
}

void Bluetooth_Process(void)
{
/* Re-arm reception if the HAL aborted it after a receive error */
if (huart2_debug.RxState == HAL_UART_STATE_READY) {
HAL_UART_Receive_IT(&huart2_debug, &s_rx_byte_uart2, 1);
}
if (huart1_bt.RxState == HAL_UART_STATE_READY) {
HAL_UART_Receive_IT(&huart1_bt, &s_rx_byte_uart1, 1);
}

/* Poll BlueNRG over SPI */
Bluetooth_BlueNRG_Poll();

uint8_t byte = 0;

while (RingBuffer_Pop(&byte)) {
if (byte == '\r' || byte == '\n') {
if (g_bt.line_index > 0) {
g_bt.current_line[g_bt.line_index] = '\0';
/* Parse complete command */
Command_Parse(g_bt.current_line);
g_bt.line_index = 0;
}
} else if (byte >= 32 && byte <= 126) {
/* Printable ASCII */
if (g_bt.line_index < (BT_CMD_MAX_LEN - 1)) {
g_bt.current_line[g_bt.line_index++] = (char)byte;
}
}
}
}

void Bluetooth_Send(const char *msg)
{
if (msg == NULL) return;
uint16_t len = (uint16_t)strlen(msg);

/* Send out to both Virtual COM Port and External Bluetooth module */
HAL_UART_Transmit(&huart2_debug, (uint8_t *)msg, len, 50);
HAL_UART_Transmit(&huart1_bt, (uint8_t *)msg, len, 50);
}

bool Bluetooth_IsConnected(void)
{
return g_bt.is_connected;
}

uint32_t Bluetooth_GetTimeSinceLastCommand(void)
{
return (HAL_GetTick() - g_bt.last_rx_timestamp_ms);
}

void Bluetooth_RestartAdvertising(void)
{
Bluetooth_BlueNRG_Start_Advertising();
Bluetooth_Send("OK:BLE_ADV_RESTARTED\r\n");
}

bool Bluetooth_TestSPI(char *out_buf, uint16_t out_max)
{
/* 1. Hardware Reset: Toggle RST pin LOW then HIGH */
HAL_GPIO_WritePin(GPIOA, IDB05A2_RST_PIN, GPIO_PIN_RESET);
HAL_Delay(25);
HAL_GPIO_WritePin(GPIOA, IDB05A2_RST_PIN, GPIO_PIN_SET);
HAL_Delay(120);

/* Wait for boot event IRQ */
uint32_t t0 = HAL_GetTick();
while (HAL_GPIO_ReadPin(GPIOA, IDB05A2_IRQ_PIN) != GPIO_PIN_SET) {
if ((HAL_GetTick() - t0) > 300) break;
HAL_Delay(5);
}
uint8_t irq1 = (uint8_t)HAL_GPIO_ReadPin(GPIOA, IDB05A2_IRQ_PIN);

/* Read boot event */
uint8_t boot_evt[16] = {0};
uint8_t boot_len = 0;
if (irq1 == 1) {
BlueNRG_Read_Packet(boot_evt, sizeof(boot_evt), &boot_len);
}
uint8_t irq2 = (uint8_t)HAL_GPIO_ReadPin(GPIOA, IDB05A2_IRQ_PIN);

/* 3. Send HCI_RESET */
uint8_t st_rst = 0xFF;
const uint8_t hci_reset[] = {0x01, 0x03, 0x0C, 0x00};
BlueNRG_Execute_Command(hci_reset, sizeof(hci_reset), &st_rst);
HAL_Delay(30);

/* 4. GATT INIT */
uint8_t st_gatt = 0xFF;
const uint8_t aci_gatt_init[] = {0x01, 0x01, 0xFD, 0x00};
BlueNRG_Execute_Command(aci_gatt_init, sizeof(aci_gatt_init), &st_gatt);
HAL_Delay(30);

/* 5. GAP INIT (Opcode 0xFC8A) */
uint8_t st_gap = 0xFF;
const uint8_t aci_gap_init[] = {0x01, 0x8A, 0xFC, 0x03, 0x01, 0x00, 0x0C};
BlueNRG_Execute_Command(aci_gap_init, sizeof(aci_gap_init), &st_gap);
HAL_Delay(30);

/* 6. SET DISCOVERABLE (Opcode 0xFC83) */
uint8_t st_adv = 0xFF;
const uint8_t aci_set_discoverable[] = {
0x01, 0x83, 0xFC, 0x18,
0x00,                   /* ADV_IND */
0xA0, 0x00,             /* Min Interval 100 ms */
0xA0, 0x00,             /* Max Interval 100 ms */
0x00,                   /* Public Address */
0x00,                   /* Filter policy */
0x0D,                   /* Local Name Length: 13 bytes */
0x09,                   /* AD_TYPE_COMPLETE_LOCAL_NAME */
'R', 'A', 'C', 'E', 'C', 'A', 'R', '-', 'L', '0', '7', '3',
0x00,                   /* Service UUID length */
0x00, 0x00,             /* Slave connection interval min */
0x00, 0x00              /* Slave connection interval max */
};
BlueNRG_Execute_Command(aci_set_discoverable, sizeof(aci_set_discoverable), &st_adv);

snprintf(out_buf, out_max,
"BLE_TEST:RST=0x%02X,GATT=0x%02X,GAP=0x%02X,ADV=0x%02X,RD=[0x%02X,0x%02X,0x%02X],WR=[0x%02X,0x%02X,0x%02X]\r\n",
st_rst, st_gatt, st_gap, st_adv, s_dbg_rd_c0, s_dbg_rd_c1, s_dbg_rd_c3, s_dbg_wr_c0, s_dbg_wr_c1, s_dbg_wr_c3);
return true;
}

