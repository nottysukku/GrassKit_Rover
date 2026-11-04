/**
  ******************************************************************************
  * @file           : wifi_st67w61.c
  * @brief          : ST67W611M1 (X-NUCLEO-67W61M1) driver: SPI transport, AT
  *                   commands, SoftAP and a small web server for phone control.
  *
  * SPI transport follows ST's reference host driver (X-CUBE-ST67W61,
  * Middlewares/ST/ST67W6X_Network_Driver/Driver/W61_bus/spi_iface.c):
  *   - CS is active HIGH. The module raises SPI_RDY when it can take a
  *     transaction (after the host raises CS) or when it has data to send.
  *   - Every transaction starts with an 8-byte header: 0xAA 0x55 (magic 0x55AA,
  *     LSB first), payload length (LE), version/rx_stall/flags, type (0 = AT),
  *     2 reserved bytes. Payload is padded to 4 bytes with 0x88.
  *   - Host clocks its header (+ payload) while receiving the module header,
  *     then clocks the rest of the module payload, then waits for SPI_RDY to
  *     drop (header ack) BEFORE lowering CS.
  * AT dialect follows ST's AT parser (W61_at sources): "+CIP:<id>,CONNECTED",
  * passive receive "+IPD:<id>,<len>" + AT+CIPRECVDATA, AT+CIPSEND -> '>' ->
  * data -> "SEND OK".
  ******************************************************************************
  */

#include "wifi_st67w61.h"
#include "command_parser.h"
#include "motor_control.h"
#include "debug.h"
#include "web_page.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Transport constants                                                 */
/* ------------------------------------------------------------------ */
#define W61_HDR_LEN              8U
#define W61_TYPE_AT              0x00U
#define W61_MAX_XFER             1520U   /* ST default W61_MAX_SPI_XFER */
#define W61_TX_MAX               1032U   /* largest payload this host sends */
#define W61_TXN_RDY_TIMEOUT_MS   300U
#define W61_HDR_ACK_TIMEOUT_MS   50U
#define W61_CS_SETUP_US          100U    /* CS high -> first SCK edge */
#define W61_RDY_EDGE_PENDING()   ((EXTI->PR & WIFI_RDY_PIN) != 0U)
#define W61_RDY_EDGE_CLEAR()     (EXTI->PR = WIFI_RDY_PIN)

#define RXQ_SIZE                 2048U
#define HTTP_PORT                WIFI_DEFAULT_PORT
#define MAX_LINKS                5U
#define LINK_BUF                 192U
#define SEND_CHUNK               1000U
#define CAP_BUF                  256U

typedef enum { AT_NONE = 0, AT_OK, AT_ERROR } at_result_t;

typedef struct {
    bool     open;
    bool     data_pending;
    bool     is_http;
    bool     classified;
    bool     done;
    uint16_t len;
    uint32_t t_open;
    char     buf[LINK_BUF];
} link_t;

static SPI_HandleTypeDef hspi1_wifi;
static WiFi_Manager_t g_wifi;

static uint8_t s_txf[W61_HDR_LEN + W61_TX_MAX];
static uint8_t s_rxf[W61_HDR_LEN + W61_MAX_XFER + 4U];

static uint8_t s_rxq[RXQ_SIZE];
static volatile uint16_t s_rxq_head;
static volatile uint16_t s_rxq_tail;

static volatile bool s_isr_armed;
static volatile uint8_t s_last_rx_stall;
static uint8_t s_last_hdr[W61_HDR_LEN];

static struct {
    uint32_t txn_ok, txn_timeout, hdr_err, ack_timeout, rx_stall, rxq_overflow, spi_err;
    uint32_t rx_bytes, tx_bytes;
} s_stat;

/* AT parser state (main context only) */
static at_result_t s_res;
static bool s_prompt, s_send_ok, s_send_fail, s_send_recv, s_boot_ready;
static char s_line[200];
static uint16_t s_line_len;
static char s_info[96];
static int32_t s_bin_remaining;
static uint8_t s_bin_link;
static int32_t s_bin_total;

static link_t s_link[MAX_LINKS];
static uint32_t s_closed_at[MAX_LINKS];
static uint32_t s_last_web_cmd_ms;

/* Reply capture (Command_Parse replies routed to the web response) */
static bool s_cap_on;
static char s_cap[CAP_BUF];
static uint16_t s_cap_len;

/* ------------------------------------------------------------------ */
/* Private prototypes                                                  */
/* ------------------------------------------------------------------ */
static void wifi_gpio_init(void);
static void wifi_spi_init(void);
static void w61_service(void);
static bool wifi_bringup(void);
static void links_process(void);

/* ------------------------------------------------------------------ */
/* Low-level helpers                                                   */
/* ------------------------------------------------------------------ */
static inline bool rdy(void)
{
    return (WIFI_RDY_PORT->IDR & WIFI_RDY_PIN) != 0U;
}

static inline void cs(bool on)
{
    if (on) {
        WIFI_SPI_CS_PORT->BSRR = WIFI_SPI_CS_PIN;
    } else {
        WIFI_SPI_CS_PORT->BRR = WIFI_SPI_CS_PIN;
    }
}

/* Busy-wait in microseconds using the SysTick down-counter (safe in IRQ) */
static void delay_us(uint32_t us)
{
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    uint32_t load = SysTick->LOAD + 1U;
    uint32_t prev = SysTick->VAL;
    uint32_t elapsed = 0U;
    while (elapsed < ticks) {
        uint32_t now = SysTick->VAL;
        elapsed += (prev >= now) ? (prev - now) : (prev + load - now);
        prev = now;
    }
}

static inline void w61_lock(void)   { NVIC_DisableIRQ(EXTI2_3_IRQn); }
static inline void w61_unlock(void) { NVIC_EnableIRQ(EXTI2_3_IRQn); }

static void rxq_push(const uint8_t *d, uint16_t n)
{
    for (uint16_t i = 0; i < n; i++) {
        uint16_t next = (uint16_t)((s_rxq_head + 1U) % RXQ_SIZE);
        if (next == s_rxq_tail) {
            s_stat.rxq_overflow++;
            return;
        }
        s_rxq[s_rxq_head] = d[i];
        s_rxq_head = next;
    }
}

static int rxq_pop(void)
{
    if (s_rxq_tail == s_rxq_head) {
        return -1;
    }
    uint8_t b = s_rxq[s_rxq_tail];
    s_rxq_tail = (uint16_t)((s_rxq_tail + 1U) % RXQ_SIZE);
    return b;
}

/**
 * One SPI transaction (ST spi_do_xfer + spi_xfer_one). Must run with the
 * RDY interrupt masked, or from inside it. data may be NULL (read only).
 * Returns the module payload length (>= 0, pushed to the RX queue) or < 0.
 */
static int32_t w61_xfer(const uint8_t *data, uint16_t len)
{
    uint32_t t0;
    int32_t ret;
    bool module_initiated = rdy();

    cs(true);
    if (!module_initiated) {
        t0 = HAL_GetTick();
        while (!rdy()) {
            if ((HAL_GetTick() - t0) > W61_TXN_RDY_TIMEOUT_MS) {
                cs(false);
                s_stat.txn_timeout++;
                return -1;
            }
        }
    }
    /* Give the module time to arm its SPI DMA after CS rises */
    delay_us(W61_CS_SETUP_US);
    W61_RDY_EDGE_CLEAR();

    uint16_t xfer = W61_HDR_LEN;
    s_txf[0] = 0xAAU;
    s_txf[1] = 0x55U;
    s_txf[2] = (uint8_t)(len & 0xFFU);
    s_txf[3] = (uint8_t)(len >> 8);
    s_txf[4] = 0U;
    s_txf[5] = W61_TYPE_AT;
    s_txf[6] = 0U;
    s_txf[7] = 0U;
    if ((data != NULL) && (len > 0U)) {
        memcpy(&s_txf[W61_HDR_LEN], data, len);
        xfer = (uint16_t)((W61_HDR_LEN + len + 3U) & ~3U);
        for (uint16_t i = (uint16_t)(W61_HDR_LEN + len); i < xfer; i++) {
            s_txf[i] = 0x88U;
        }
    }

    if (HAL_SPI_TransmitReceive(&hspi1_wifi, s_txf, s_rxf, xfer, 20) != HAL_OK) {
        s_stat.spi_err++;
        ret = -2;
        goto ack;
    }
    memcpy(s_last_hdr, s_rxf, W61_HDR_LEN);
    if ((s_rxf[0] != 0xAAU) || (s_rxf[1] != 0x55U)) {
        s_stat.hdr_err++;
        ret = -3;
        goto ack;
    }

    uint16_t rlen = (uint16_t)(s_rxf[2] | ((uint16_t)s_rxf[3] << 8));
    if (rlen > W61_MAX_XFER) {
        s_stat.hdr_err++;
        ret = -4;
        goto ack;
    }
    s_last_rx_stall = (uint8_t)((s_rxf[4] >> 2) & 1U);
    if (s_last_rx_stall) {
        s_stat.rx_stall++;
    }

    if ((uint32_t)rlen + W61_HDR_LEN > xfer) {
        uint16_t remain = (uint16_t)(((uint32_t)rlen + W61_HDR_LEN - xfer + 3U) & ~3U);
        uint8_t *p = &s_rxf[xfer];
        memset(p, 0, remain);
        if (HAL_SPI_TransmitReceive(&hspi1_wifi, p, p, remain, 50) != HAL_OK) {
            s_stat.spi_err++;
            ret = -5;
            goto ack;
        }
    }

    if ((rlen > 0U) && (s_rxf[5] == W61_TYPE_AT)) {
        rxq_push(&s_rxf[W61_HDR_LEN], rlen);
        s_stat.rx_bytes += rlen;
    }
    if ((data != NULL) && !s_last_rx_stall) {
        s_stat.tx_bytes += len;
    }
    ret = (int32_t)rlen;

ack:
    /* Header ack: module drops SPI_RDY after decoding our header. Keep CS
     * high until then (ST wiki: lowering CS early can wedge the module). */
    t0 = HAL_GetTick();
    while (rdy() && !W61_RDY_EDGE_PENDING()) {
        if ((HAL_GetTick() - t0) > W61_HDR_ACK_TIMEOUT_MS) {
            s_stat.ack_timeout++;
            break;
        }
    }
    cs(false);
    if (ret >= 0) {
        s_stat.txn_ok++;
    }
    return ret;
}

/* Module-initiated reads, from main context */
static void w61_poll_rx(void)
{
    uint8_t guard = 8U;
    while (rdy() && (guard-- > 0U)) {
        w61_lock();
        (void)w61_xfer(NULL, 0);
        w61_unlock();
    }
}

/* Host write with rx_stall retry */
static bool w61_send(const uint8_t *data, uint16_t len)
{
    if (len > W61_TX_MAX) {
        return false;
    }
    uint8_t timeouts = 0U;
    for (uint8_t attempt = 0; attempt < 30U; attempt++) {
        w61_lock();
        int32_t r = w61_xfer(data, len);
        bool stalled = (r >= 0) && (s_last_rx_stall != 0U);
        w61_unlock();
        if ((r >= 0) && !stalled) {
            return true;
        }
        if ((r == -1) && (++timeouts >= 3U)) {
            return false; /* module never raised SPI_RDY */
        }
        uint32_t t0 = HAL_GetTick();
        while ((HAL_GetTick() - t0) < 3U) {
            w61_poll_rx();
        }
    }
    return false;
}

/* SPI_RDY edge interrupt: read module data immediately */
void EXTI2_3_IRQHandler(void)
{
    if ((EXTI->PR & WIFI_RDY_PIN) != 0U) {
        EXTI->PR = WIFI_RDY_PIN;
        if (s_isr_armed) {
            uint8_t guard = 4U;
            while (rdy() && (guard-- > 0U)) {
                (void)w61_xfer(NULL, 0);
            }
        }
    }
    /* Clear any other line pending in this group */
    EXTI->PR = (GPIO_PIN_2 & EXTI->PR);
}

/* ------------------------------------------------------------------ */
/* AT parser                                                           */
/* ------------------------------------------------------------------ */
static void link_reset(uint8_t id)
{
    if (id < MAX_LINKS) {
        memset(&s_link[id], 0, sizeof(link_t));
    }
}

static void link_rx_byte(uint8_t id, uint8_t b)
{
    if (id >= MAX_LINKS) {
        return;
    }
    link_t *l = &s_link[id];
    if (l->len < (LINK_BUF - 1U)) {
        l->buf[l->len++] = (char)b;
        l->buf[l->len] = '\0';
    }
}

static void at_line(char *line)
{
    if (strcmp(line, "OK") == 0) {
        s_res = AT_OK;
        return;
    }
    if ((strcmp(line, "ERROR") == 0) || (strncmp(line, "busy", 4) == 0)) {
        s_res = AT_ERROR;
        Debug_Log("[AT<] %s\r\n", line);
        return;
    }
    if (strcmp(line, "SEND OK") == 0) {
        s_send_ok = true;
        return;
    }
    if ((strcmp(line, "SEND FAIL") == 0) || (strcmp(line, "SEND:ERROR") == 0)) {
        s_send_fail = true;
        return;
    }
    if (strncmp(line, "Recv ", 5) == 0) {
        s_send_recv = true;
        return;
    }
    if (strcmp(line, "ready") == 0) {
        s_boot_ready = true;
        Debug_Log("[AT<] ready\r\n");
        return;
    }
    if (strncmp(line, "+CIP:", 5) == 0) {
        int id = atoi(line + 5);
        char *c = strchr(line, ',');
        if ((id >= 0) && (id < (int)MAX_LINKS) && (c != NULL)) {
            if (strncmp(c + 1, "CONNECTED", 9) == 0) {
                link_reset((uint8_t)id);
                s_link[id].open = true;
                s_link[id].t_open = HAL_GetTick();
            } else if (strncmp(c + 1, "DISCONNECTED", 12) == 0) {
                link_reset((uint8_t)id);
            }
        }
        return;
    }
    if (strncmp(line, "+IPD:", 5) == 0) {
        int id = atoi(line + 5);
        if ((id >= 0) && (id < (int)MAX_LINKS)) {
            if (!s_link[id].open) {
                if ((HAL_GetTick() - s_closed_at[id]) < 2000U) {
                    return; /* leftovers of a socket we just closed */
                }
                s_link[id].open = true;
            }
            s_link[id].data_pending = true;
            s_link[id].t_open = HAL_GetTick(); /* last activity */
        }
        return;
    }
    if ((line[0] >= '0') && (line[0] <= '9') && (strstr(line, ",CLOSED") != NULL)) {
        link_reset((uint8_t)atoi(line));
        return;
    }
    if (line[0] == '+') {
        strncpy(s_info, line, sizeof(s_info) - 1U);
        s_info[sizeof(s_info) - 1U] = '\0';
    }
    Debug_Log("[AT<] %s\r\n", line);
}

static void at_feed(uint8_t b)
{
    if (s_bin_remaining > 0) {
        link_rx_byte(s_bin_link, b);
        s_bin_remaining--;
        return;
    }
    if ((s_line_len == 0U) && (b == '>')) {
        s_prompt = true;
        return;
    }
    if (b == '\r') {
        return;
    }
    if (b == '\n') {
        s_line[s_line_len] = '\0';
        if (s_line_len > 0U) {
            at_line(s_line);
        }
        s_line_len = 0U;
        return;
    }
    if (s_line_len < (sizeof(s_line) - 1U)) {
        s_line[s_line_len++] = (char)b;
    }
    /* "+CIPRECVDATA:<len>,<binary data>" */
    if ((b == ',') && (s_line_len > 13U) && (strncmp(s_line, "+CIPRECVDATA:", 13) == 0)) {
        s_line[s_line_len] = '\0';
        s_bin_remaining = atoi(s_line + 13);
        s_bin_total = s_bin_remaining;
        s_line_len = 0U;
    }
}

static void w61_service(void)
{
    int b;
    w61_poll_rx();
    while ((b = rxq_pop()) >= 0) {
        at_feed((uint8_t)b);
    }
}

static at_result_t at_cmd(const char *cmd, uint32_t timeout_ms, bool quiet)
{
    if (!quiet) {
        Debug_Log("[AT>] %s", cmd);
    }
    s_res = AT_NONE;
    if (!w61_send((const uint8_t *)cmd, (uint16_t)strlen(cmd))) {
        Debug_Log("[WIFI] SPI send failed (%s)\r\n", cmd);
        return AT_ERROR;
    }
    uint32_t t0 = HAL_GetTick();
    while ((s_res == AT_NONE) && ((HAL_GetTick() - t0) < timeout_ms)) {
        w61_service();
    }
    if ((s_res == AT_NONE) && !quiet) {
        Debug_Log("[WIFI] timeout waiting for reply\r\n");
    }
    return s_res;
}

static bool link_send(uint8_t id, const char *data, uint16_t len)
{
    char cmd[40];
    while (len > 0U) {
        uint16_t chunk = (len > SEND_CHUNK) ? SEND_CHUNK : len;
        s_prompt = false;
        s_send_ok = false;
        s_send_fail = false;
        s_send_recv = false;
        snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u,%u\r\n", id, chunk);
        s_res = AT_NONE;
        if (!w61_send((const uint8_t *)cmd, (uint16_t)strlen(cmd))) {
            return false;
        }
        uint32_t t0 = HAL_GetTick();
        while (!s_prompt && (s_res != AT_ERROR) && ((HAL_GetTick() - t0) < 2000U)) {
            w61_service();
        }
        if (!s_prompt) {
            Debug_Log("[WEB] link %u: no '>' prompt\r\n", id);
            return false;
        }
        if (!w61_send((const uint8_t *)data, chunk)) {
            return false;
        }
        t0 = HAL_GetTick();
        uint32_t t_recv = 0U;
        while (!s_send_ok && !s_send_fail && ((HAL_GetTick() - t0) < 3000U)) {
            w61_service();
            if (s_send_recv && (t_recv == 0U)) {
                t_recv = HAL_GetTick();
            }
            if ((t_recv != 0U) && ((HAL_GetTick() - t_recv) > 300U)) {
                break; /* "Recv N bytes" seen, treat as accepted */
            }
        }
        if (s_send_fail || (!s_send_ok && !s_send_recv)) {
            Debug_Log("[WEB] link %u: send failed\r\n", id);
            return false;
        }
        g_wifi.tx_packet_count++;
        data += chunk;
        len = (uint16_t)(len - chunk);
    }
    return true;
}

static void link_close(uint8_t id)
{
    char cmd[24];
    snprintf(cmd, sizeof(cmd), "AT+CIPCLOSE=%u\r\n", id);
    (void)at_cmd(cmd, 1000U, true);
    link_reset(id);
    s_closed_at[id] = HAL_GetTick();
}

/* ------------------------------------------------------------------ */
/* Web / raw command handling                                          */
/* ------------------------------------------------------------------ */
static void cap_begin(void)
{
    s_cap_len = 0U;
    s_cap[0] = '\0';
    s_cap_on = true;
}

static void cap_end(void)
{
    s_cap_on = false;
}

static void url_decode(const char *src, char *dst, uint16_t cap)
{
    uint16_t o = 0U;
    while ((*src != '\0') && (*src != ' ') && (*src != '&') && (o < (cap - 1U))) {
        if ((*src == '%') && (src[1] != '\0') && (src[2] != '\0')) {
            char hex[3] = { src[1], src[2], '\0' };
            dst[o++] = (char)strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            dst[o++] = ' ';
            src++;
        } else {
            dst[o++] = *src++;
        }
    }
    dst[o] = '\0';
}

static void http_reply(uint8_t id, const char *status, const char *ctype, const char *extra_hdr,
                       const char *body, uint16_t body_len)
{
    char hdr[200];
    int n = snprintf(hdr, sizeof(hdr),
                     "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %u\r\n%s"
                     "Cache-Control: no-store\r\nConnection: close\r\n\r\n",
                     status, ctype, body_len, extra_hdr);
    if (link_send(id, hdr, (uint16_t)n) && (body_len > 0U)) {
        (void)link_send(id, body, body_len);
    }
}

static void http_handle(uint8_t id, char *req)
{
    /* req = "GET /path HTTP/1.1..." */
    char *path = strchr(req, ' ');
    if (path == NULL) {
        http_reply(id, "400 Bad Request", "text/plain", "", "bad request", 11);
        return;
    }
    path++;

    if ((strncmp(path, "/ ", 2) == 0) || (strncmp(path, "/index", 6) == 0)) {
        Debug_Log("[WEB] link %u: page requested\r\n", id);
        http_reply(id, "200 OK", "text/html; charset=utf-8", "Content-Encoding: gzip\r\n",
                   (const char *)g_web_page_gz, g_web_page_gz_len);
        return;
    }

    if (strncmp(path, "/c?x=", 5) == 0) {
        char cmd[64];
        url_decode(path + 5, cmd, sizeof(cmd));
        s_last_web_cmd_ms = HAL_GetTick();
        g_wifi.rx_packet_count++;
        cap_begin();
        if (strcmp(cmd, "STATUS") != 0) {
            /* STATUS polls must not feed the safety watchdog */
            Command_Parse(cmd);
        }
        Command_SendWebStatus();
        cap_end();
        for (uint16_t i = 0; i < s_cap_len; i++) {
            if ((s_cap[i] == '\r') || (s_cap[i] == '\n')) {
                s_cap[i] = ' ';
            }
        }
        http_reply(id, "200 OK", "text/plain", "", s_cap, s_cap_len);
        return;
    }

    http_reply(id, "404 Not Found", "text/plain", "", "not found", 9);
}

static void raw_handle(uint8_t id)
{
    /* Plain TCP client (terminal app): one command per line */
    link_t *l = &s_link[id];
    char *nl;
    while ((nl = strchr(l->buf, '\n')) != NULL) {
        *nl = '\0';
        char line[64];
        strncpy(line, l->buf, sizeof(line) - 1U);
        line[sizeof(line) - 1U] = '\0';
        uint16_t consumed = (uint16_t)((nl - l->buf) + 1);
        memmove(l->buf, nl + 1, l->len - consumed + 1U);
        l->len = (uint16_t)(l->len - consumed);

        s_last_web_cmd_ms = HAL_GetTick();
        g_wifi.rx_packet_count++;
        cap_begin();
        Command_Parse(line);
        cap_end();
        if (s_cap_len > 0U) {
            (void)link_send(id, s_cap, s_cap_len);
        }
    }
    if (l->len >= (LINK_BUF - 1U)) {
        l->len = 0U; /* overlong line, drop */
        l->buf[0] = '\0';
    }
}

static void links_process(void)
{
    char cmd[40];
    for (uint8_t id = 0; id < MAX_LINKS; id++) {
        link_t *l = &s_link[id];

        if (l->data_pending) {
            l->data_pending = false;
            for (uint8_t k = 0; k < 4U; k++) {
                s_bin_link = id;
                s_bin_total = 0;
                snprintf(cmd, sizeof(cmd), "AT+CIPRECVDATA=%u,512\r\n", id);
                if (at_cmd(cmd, 1000U, true) != AT_OK) {
                    break;
                }
                if (s_bin_total < 512) {
                    break;
                }
            }
        }

        if (l->open && !l->classified && (l->len >= 4U)) {
            l->classified = true;
            l->is_http = (strncmp(l->buf, "GET ", 4) == 0) || (strncmp(l->buf, "POST", 4) == 0) ||
                         (strncmp(l->buf, "HEAD", 4) == 0);
        }

        if (l->open && l->classified && !l->done) {
            if (l->is_http) {
                if (strchr(l->buf, '\n') != NULL) {
                    l->done = true;
                    http_handle(id, l->buf);
                    link_close(id);
                }
            } else {
                raw_handle(id);
            }
        }

        /* Reclaim idle HTTP sockets (browser pre-connects) */
        if (l->open && (l->len == 0U) && ((HAL_GetTick() - l->t_open) > 8000U)) {
            link_close(id);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Bring-up                                                            */
/* ------------------------------------------------------------------ */
static void wifi_gpio_init(void)
{
    GPIO_InitTypeDef g = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* CS (PB6): active HIGH, idle LOW */
    HAL_GPIO_WritePin(WIFI_SPI_CS_PORT, WIFI_SPI_CS_PIN, GPIO_PIN_RESET);
    g.Pin = WIFI_SPI_CS_PIN;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(WIFI_SPI_CS_PORT, &g);

    /* CHIP_EN (PB4): LOW = module off */
    HAL_GPIO_WritePin(WIFI_CHIP_EN_PORT, WIFI_CHIP_EN_PIN, GPIO_PIN_RESET);
    g.Pin = WIFI_CHIP_EN_PIN;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(WIFI_CHIP_EN_PORT, &g);

    /* BOOT (PB10): LOW = run application firmware */
    HAL_GPIO_WritePin(WIFI_BOOT_PORT, WIFI_BOOT_PIN, GPIO_PIN_RESET);
    g.Pin = WIFI_BOOT_PIN;
    HAL_GPIO_Init(WIFI_BOOT_PORT, &g);

    /* SPI_RDY (PB3): input, interrupt on both edges */
    g.Pin = WIFI_RDY_PIN;
    g.Mode = GPIO_MODE_IT_RISING_FALLING;
    g.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(WIFI_RDY_PORT, &g);
    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 3, 0); /* below SysTick (0) and USART2 (2) */
}

static void wifi_spi_init(void)
{
    GPIO_InitTypeDef g = {0};

    WIFI_SPI_CLK_ENABLE();
    g.Pin = WIFI_SPI_SCK_PIN | WIFI_SPI_MISO_PIN | WIFI_SPI_MOSI_PIN;
    g.Mode = GPIO_MODE_AF_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    g.Alternate = GPIO_AF0_SPI1;
    HAL_GPIO_Init(GPIOA, &g);

    /* Mode 0, 8-bit, MSB first (ST67W611M1 SPI wiki), 32 MHz / 8 = 4 MHz */
    hspi1_wifi.Instance = WIFI_SPI_INSTANCE;
    hspi1_wifi.Init.Mode = SPI_MODE_MASTER;
    hspi1_wifi.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1_wifi.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1_wifi.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1_wifi.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1_wifi.Init.NSS = SPI_NSS_SOFT;
    hspi1_wifi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi1_wifi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1_wifi.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1_wifi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1_wifi.Init.CRCPolynomial = 7;
    (void)HAL_SPI_DeInit(&hspi1_wifi);
    (void)HAL_SPI_Init(&hspi1_wifi);
}

void WiFi_Init(void)
{
    NVIC_DisableIRQ(EXTI2_3_IRQn);
    s_isr_armed = false;

    memset(&g_wifi, 0, sizeof(g_wifi));
    memset(&s_stat, 0, sizeof(s_stat));
    memset(s_link, 0, sizeof(s_link));
    s_rxq_head = 0U;
    s_rxq_tail = 0U;
    s_line_len = 0U;
    s_bin_remaining = 0;
    s_boot_ready = false;
    s_cap_on = false;

    g_wifi.state = WIFI_STATE_UNINITIALIZED;
    strncpy(g_wifi.ssid, WIFI_DEFAULT_SSID, sizeof(g_wifi.ssid) - 1U);
    strncpy(g_wifi.password, WIFI_DEFAULT_PASSWORD, sizeof(g_wifi.password) - 1U);
    g_wifi.channel = WIFI_DEFAULT_CHANNEL;
    g_wifi.port = HTTP_PORT;
    strncpy(g_wifi.ip_addr, WIFI_HOST_IP_STR, sizeof(g_wifi.ip_addr) - 1U);

    wifi_gpio_init();
    wifi_spi_init();

    Debug_Log("[WIFI] Power-cycling ST67W611M1 (CHIP_EN low 200 ms)...\r\n");
    HAL_GPIO_WritePin(WIFI_CHIP_EN_PORT, WIFI_CHIP_EN_PIN, GPIO_PIN_RESET);
    HAL_Delay(200);

    /* Arm the SPI_RDY interrupt before the module boots so "ready" is read at once */
    W61_RDY_EDGE_CLEAR();
    NVIC_ClearPendingIRQ(EXTI2_3_IRQn);
    s_isr_armed = true;
    NVIC_EnableIRQ(EXTI2_3_IRQn);

    HAL_GPIO_WritePin(WIFI_CHIP_EN_PORT, WIFI_CHIP_EN_PIN, GPIO_PIN_SET);
    g_wifi.state = WIFI_STATE_WAIT_RDY;
    g_wifi.state_entry_time_ms = HAL_GetTick();
}

static bool at_ok(const char *cmd, uint32_t timeout)
{
    at_result_t r = at_cmd(cmd, timeout, false);
    if (r != AT_OK) {
        Debug_Log("[WIFI]   -> %s\r\n", (r == AT_ERROR) ? "ERROR" : "no reply");
        return false;
    }
    return true;
}

static bool wifi_bringup(void)
{
    char cmd[96];
    bool alive = false;

    for (uint8_t i = 0; (i < 5U) && !alive; i++) {
        alive = (at_cmd("AT\r\n", 500U, false) == AT_OK);
    }
    if (!alive) {
        Debug_Log("[WIFI] Module did not answer AT. hdr=[%02X %02X %02X %02X %02X %02X %02X %02X] "
                  "ok=%lu tmo=%lu hdr_err=%lu\r\n",
                  s_last_hdr[0], s_last_hdr[1], s_last_hdr[2], s_last_hdr[3],
                  s_last_hdr[4], s_last_hdr[5], s_last_hdr[6], s_last_hdr[7],
                  s_stat.txn_ok, s_stat.txn_timeout, s_stat.hdr_err);
        return false;
    }
    Debug_Log("[WIFI] Module answers AT over SPI.\r\n");

    s_info[0] = '\0';
    (void)at_cmd("AT+GMR\r\n", 1000U, false);

    /* SoftAP (ST driver uses mode 3 = SoftAP + station, station idle) */
    if (!at_ok("AT+CWMODE=3,0\r\n", 3000U) && !at_ok("AT+CWMODE=2\r\n", 3000U)) {
        return false;
    }
    snprintf(cmd, sizeof(cmd), "AT+CWSAP=\"%s\",\"%s\",%u,3,4,0\r\n",
             g_wifi.ssid, g_wifi.password, g_wifi.channel);
    if (!at_ok(cmd, 8000U)) {
        return false;
    }

    s_info[0] = '\0';
    if (at_cmd("AT+CIPAP?\r\n", 2000U, false) == AT_OK) {
        /* +CIPAP:ip:"192.168.x.x" */
        char *q = strstr(s_info, "ip:\"");
        if (q != NULL) {
            q += 4;
            char *e = strchr(q, '"');
            if ((e != NULL) && ((e - q) < (int)sizeof(g_wifi.ip_addr))) {
                memcpy(g_wifi.ip_addr, q, (size_t)(e - q));
                g_wifi.ip_addr[e - q] = '\0';
            }
        }
    }

    (void)at_ok("AT+CIPMUX=1\r\n", 2000U);
    (void)at_ok("AT+CIPRECVMODE=1\r\n", 2000U);
    (void)at_ok("AT+CIPDINFO=0\r\n", 2000U);
    (void)at_ok("AT+CIPSERVERMAXCONN=4\r\n", 2000U);
    snprintf(cmd, sizeof(cmd), "AT+CIPSERVER=1,%u\r\n", g_wifi.port);
    if (!at_ok(cmd, 3000U)) {
        snprintf(cmd, sizeof(cmd), "AT+CIPSERVER=1,%u,\"TCP\",0,0\r\n", g_wifi.port);
        if (!at_ok(cmd, 3000U)) {
            return false;
        }
    }
    (void)at_ok("AT+CIPSTO=10\r\n", 2000U);
    return true;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */
void WiFi_Process(void)
{
    uint32_t now = HAL_GetTick();

    switch (g_wifi.state) {
        case WIFI_STATE_WAIT_RDY:
            w61_service();
            if (s_boot_ready) {
                Debug_Log("[WIFI] Module booted (+%lums).\r\n", now - g_wifi.state_entry_time_ms);
                g_wifi.state = WIFI_STATE_INIT_AT;
            } else if ((now - g_wifi.state_entry_time_ms) > 3000U) {
                Debug_Log("[WIFI] No 'ready' message after 3 s (txn ok=%lu tmo=%lu hdr_err=%lu), trying AT.\r\n",
                          s_stat.txn_ok, s_stat.txn_timeout, s_stat.hdr_err);
                g_wifi.state = WIFI_STATE_INIT_AT;
            }
            break;

        case WIFI_STATE_INIT_AT:
            if (wifi_bringup()) {
                g_wifi.state = WIFI_STATE_SERVER_READY;
                Debug_Log("\r\n[WIFI] ===== SoftAP READY =====\r\n");
                Debug_Log("[WIFI] Wi-Fi name: %s   password: %s\r\n", g_wifi.ssid, g_wifi.password);
                Debug_Log("[WIFI] Then open in the phone browser: http://%s/\r\n\r\n", g_wifi.ip_addr);
            } else {
                Debug_Log("[WIFI] Bring-up failed; retrying in 10 s (send WIFIDIAG for details).\r\n");
                g_wifi.state = WIFI_STATE_ERROR;
            }
            g_wifi.state_entry_time_ms = now;
            break;

        case WIFI_STATE_ERROR:
            w61_service();
            if ((now - g_wifi.state_entry_time_ms) > 10000U) {
                WiFi_Init();
            }
            break;

        case WIFI_STATE_SERVER_READY:
        case WIFI_STATE_CLIENT_CONNECTED:
            w61_service();
            links_process();
            g_wifi.state = WiFi_IsClientConnected() ? WIFI_STATE_CLIENT_CONNECTED : WIFI_STATE_SERVER_READY;
            break;

        default:
            break;
    }
}

bool WiFi_SendPacket(uint8_t link_id, const char *data, uint16_t length)
{
    if ((link_id >= MAX_LINKS) || !s_link[link_id].open) {
        return false;
    }
    return link_send(link_id, data, length);
}

/* Command replies: copied into the pending web response, otherwise dropped */
void WiFi_BroadcastTelemetry(const char *data, uint16_t length)
{
    if (!s_cap_on) {
        return;
    }
    uint16_t room = (uint16_t)(CAP_BUF - 1U - s_cap_len);
    uint16_t n = (length < room) ? length : room;
    memcpy(&s_cap[s_cap_len], data, n);
    s_cap_len = (uint16_t)(s_cap_len + n);
    s_cap[s_cap_len] = '\0';
}

bool WiFi_IsClientConnected(void)
{
    return (s_last_web_cmd_ms != 0U) && ((HAL_GetTick() - s_last_web_cmd_ms) < 3000U);
}

const char *WiFi_GetStateString(void)
{
    switch (g_wifi.state) {
        case WIFI_STATE_UNINITIALIZED:    return "UNINITIALIZED";
        case WIFI_STATE_RESETTING:        return "RESETTING";
        case WIFI_STATE_WAIT_RDY:         return "WAIT_RDY";
        case WIFI_STATE_INIT_AT:          return "INIT_AT";
        case WIFI_STATE_CONFIG_SOFTAP:    return "CONFIG_SOFTAP";
        case WIFI_STATE_START_SERVER:     return "START_SERVER";
        case WIFI_STATE_SERVER_READY:     return "SERVER_READY";
        case WIFI_STATE_CLIENT_CONNECTED: return "CLIENT_CONNECTED";
        case WIFI_STATE_ERROR:            return "ERROR";
        default:                          return "UNKNOWN";
    }
}

WiFi_State_t WiFi_GetState(void)
{
    return g_wifi.state;
}

void WiFi_RunDiagnostic(void)
{
    Debug_Log("\r\n=== ST67W61 DIAGNOSTIC ===\r\n");
    Debug_Log("state=%s ip=%s CHIP_EN=%d BOOT=%d RDY=%d CS=%d\r\n", WiFi_GetStateString(), g_wifi.ip_addr,
              HAL_GPIO_ReadPin(WIFI_CHIP_EN_PORT, WIFI_CHIP_EN_PIN), HAL_GPIO_ReadPin(WIFI_BOOT_PORT, WIFI_BOOT_PIN),
              HAL_GPIO_ReadPin(WIFI_RDY_PORT, WIFI_RDY_PIN), HAL_GPIO_ReadPin(WIFI_SPI_CS_PORT, WIFI_SPI_CS_PIN));
    Debug_Log("txn ok=%lu rdy_timeout=%lu hdr_err=%lu ack_timeout=%lu rx_stall=%lu spi_err=%lu rxq_ovf=%lu\r\n",
              s_stat.txn_ok, s_stat.txn_timeout, s_stat.hdr_err, s_stat.ack_timeout, s_stat.rx_stall,
              s_stat.spi_err, s_stat.rxq_overflow);
    Debug_Log("rx_bytes=%lu tx_bytes=%lu web_cmds=%lu web_sends=%lu\r\n", s_stat.rx_bytes, s_stat.tx_bytes,
              g_wifi.rx_packet_count, g_wifi.tx_packet_count);
    Debug_Log("last hdr=[%02X %02X %02X %02X %02X %02X %02X %02X]\r\n", s_last_hdr[0], s_last_hdr[1],
              s_last_hdr[2], s_last_hdr[3], s_last_hdr[4], s_last_hdr[5], s_last_hdr[6], s_last_hdr[7]);
    if ((g_wifi.state == WIFI_STATE_SERVER_READY) || (g_wifi.state == WIFI_STATE_CLIENT_CONNECTED)) {
        (void)at_cmd("AT\r\n", 500U, false);
        (void)at_cmd("AT+CWLIF\r\n", 1000U, false);
    }
    Debug_Log("=== END DIAGNOSTIC ===\r\n");
}
