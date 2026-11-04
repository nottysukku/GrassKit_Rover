/**
  ******************************************************************************
  * @file    ncp_bridge.c
  * @brief   Temporary NUCLEO-L073RZ firmware for updating the ST67W611M1
  *          (X-NUCLEO-67W61M1) Wi-Fi module with ST's QConn_Flash tool.
  *
  * Same idea as ST's NCP_Loader (X-CUBE-ST67W61): put the module in UART boot
  * mode and copy UART levels between the module and the ST-LINK virtual COM
  * port in a tight loop. Run QConn_Flash_Cmd on the ST-LINK COM port.
  *
  * Wiring (two jumpers; on NUCLEO-64 the Arduino D0/D1 pins are not connected
  * to the MCU by default): shield D0 / D1 (module UART TX / RX) to Nucleo
  * CN7 pin 1 (PC10) and CN7 pin 2 (PC11), either way round: the module TX line
  * is detected at start-up. The line towards the module RX is driven
  * open-drain with a pull-up, so a swapped pair never fights an output.
  *
  * The motor driver (X-STM32MP-RBT01) is held disabled the whole time.
  *
  * Reset the Nucleo to re-enter module boot mode.
  ******************************************************************************
  */

#include "stm32l0xx_hal.h"
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef huart2;

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {
    }
}

static void clock_32mhz(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};
    RCC_PeriphCLKInitTypeDef per = {0};

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    osc.PLL.PLLMUL = RCC_PLLMUL_4;
    osc.PLL.PLLDIV = RCC_PLLDIV_2;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        Error_Handler();
    }
    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_1) != HAL_OK) {
        Error_Handler();
    }
    per.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    per.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
    (void)HAL_RCCEx_PeriphCLKConfig(&per);
}

static void out_low(GPIO_TypeDef *port, uint32_t pins)
{
    GPIO_InitTypeDef g = {0};
    HAL_GPIO_WritePin(port, pins, GPIO_PIN_RESET);
    g.Pin = pins;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &g);
}

static void input(GPIO_TypeDef *port, uint32_t pins, uint32_t pull)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = pins;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = pull;
    HAL_GPIO_Init(port, &g);
}

static void say(const char *s)
{
    (void)HAL_UART_Transmit(&huart2, (uint8_t *)s, (uint16_t)strlen(s), 200);
}

int main(void)
{
    char buf[160];

    HAL_Init();
    clock_32mhz();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Motor driver safe: PWM (PC6, PB5, PB0, PB1), DIR (PC0..PC3) and the
     * EN/nFAULT lines (PC4, PC5, PC8, PC9) all driven low = bridges off */
    out_low(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |
                   GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9);
    out_low(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_5);

    /* Wi-Fi shield control: CHIP_EN (PB4) low = module off, BOOT (PB10) low,
     * SPI CS (PB6) low; SPI lines (PA5..PA7) left as inputs */
    out_low(GPIOB, GPIO_PIN_4 | GPIO_PIN_10 | GPIO_PIN_6);
    input(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_NOPULL);

    /* Status messages on the ST-LINK COM port, 115200 8N1 */
    {
        GPIO_InitTypeDef g = {0};
        __HAL_RCC_USART2_CLK_ENABLE();
        g.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        g.Mode = GPIO_MODE_AF_PP;
        g.Pull = GPIO_PULLUP;
        g.Speed = GPIO_SPEED_FREQ_HIGH;
        g.Alternate = GPIO_AF4_USART2;
        HAL_GPIO_Init(GPIOA, &g);
        huart2.Instance = USART2;
        huart2.Init.BaudRate = 115200;
        huart2.Init.WordLength = UART_WORDLENGTH_8B;
        huart2.Init.StopBits = UART_STOPBITS_1;
        huart2.Init.Parity = UART_PARITY_NONE;
        huart2.Init.Mode = UART_MODE_TX_RX;
        huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart2.Init.OverSampling = UART_OVERSAMPLING_16;
        (void)HAL_UART_Init(&huart2);
    }
    say("\r\n[BRIDGE] ST67W611M1 flash bridge. Motors held off.\r\n");

    /* ---- Which jumper is the module TX? (skip with -DTX_PIN=11) ---- */
#ifndef TX_PIN
    input(GPIOC, GPIO_PIN_10 | GPIO_PIN_11, GPIO_PULLDOWN);
    HAL_Delay(300);
    uint32_t off10 = (GPIOC->IDR >> 10) & 1U;
    uint32_t off11 = (GPIOC->IDR >> 11) & 1U;

    /* Normal boot for 2 s: module TX idles high and may print a boot log */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
    uint32_t e10 = 0U, e11 = 0U, hi10 = 0U, hi11 = 0U, n = 0U;
    uint32_t p10 = off10, p11 = off11;
    uint32_t t0 = HAL_GetTick();
    while ((HAL_GetTick() - t0) < 2000U) {
        uint32_t idr = GPIOC->IDR;
        uint32_t v10 = (idr >> 10) & 1U, v11 = (idr >> 11) & 1U;
        e10 += (v10 != p10);
        e11 += (v11 != p11);
        hi10 += v10;
        hi11 += v11;
        p10 = v10;
        p11 = v11;
        n++;
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

    uint32_t tx_pin;   /* module TX -> our input */
    const char *why;
    if ((e10 > 4U) && (e10 > (4U * e11))) {
        tx_pin = 10U; why = "boot-log edges";
    } else if ((e11 > 4U) && (e11 > (4U * e10))) {
        tx_pin = 11U; why = "boot-log edges";
    } else if ((hi10 > (n * 9U / 10U)) && (hi11 < (n / 10U))) {
        tx_pin = 10U; why = "idle-high level";
    } else if ((hi11 > (n * 9U / 10U)) && (hi10 < (n / 10U))) {
        tx_pin = 11U; why = "idle-high level";
    } else {
        tx_pin = 11U; why = "UNSURE, defaulting";
    }
    snprintf(buf, sizeof(buf),
             "[BRIDGE] off: PC10=%lu PC11=%lu | on: PC10 edges=%lu high=%lu%%, PC11 edges=%lu high=%lu%%\r\n",
             off10, off11, e10, (hi10 * 100U) / n, e11, (hi11 * 100U) / n);
    say(buf);
#else
    uint32_t tx_pin = TX_PIN;
    const char *why = "set at build time";
#endif
    uint32_t rx_pin = (tx_pin == 10U) ? 11U : 10U;
    snprintf(buf, sizeof(buf), "[BRIDGE] module TX on PC%lu, module RX on PC%lu (%s)\r\n", tx_pin, rx_pin, why);
    say(buf);

    /* Line towards module RX: open-drain + pull-up, idle released (high) */
    {
        GPIO_InitTypeDef g = {0};
        GPIOC->BSRR = (1U << rx_pin);
        g.Pin = (1U << rx_pin);
        g.Mode = GPIO_MODE_OUTPUT_OD;
        g.Pull = GPIO_PULLUP;
        g.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOC, &g);
        input(GPIOC, (1U << tx_pin), GPIO_PULLUP);
    }

    /* ---- Module held in reset with BOOT high until the PC starts talking ----
     * QConn sends one command, then (after ~1 s, expecting a DTR/RTS reset it
     * cannot do through the ST-LINK port) the UART sync. Releasing CHIP_EN on
     * the PC's first byte gives the boot ROM a clean start before that sync. */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);   /* BOOT high */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);  /* CHIP_EN low */

    say("[BRIDGE] Module held in reset (BOOT high). Close this port and run QConn now.\r\n");
    HAL_Delay(20);

    /* ST-LINK side as plain GPIO: PA2 -> ST-LINK RX (push-pull), PA3 <- ST-LINK TX */
    (void)HAL_UART_DeInit(&huart2);
    {
        GPIO_InitTypeDef g = {0};
        GPIOA->BSRR = GPIO_PIN_2;
        g.Pin = GPIO_PIN_2;
        g.Mode = GPIO_MODE_OUTPUT_PP;
        g.Pull = GPIO_NOPULL;
        g.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &g);
        input(GPIOA, GPIO_PIN_3, GPIO_PULLUP);
    }

    /* Wait for the PC's first start bit, then boot the module into its ROM */
    while ((GPIOA->IDR & GPIO_PIN_3) != 0U) {
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);    /* CHIP_EN high */
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET); /* BOOT low (as ST NCP_Loader) */

    __disable_irq();
    SysTick->CTRL = 0U;

    const uint32_t txm = (1U << tx_pin);
    const uint32_t rx_set = (1U << rx_pin);
    const uint32_t rx_clr = (1U << (rx_pin + 16U));
    volatile uint32_t *const c_idr = &GPIOC->IDR;
    volatile uint32_t *const a_idr = &GPIOA->IDR;
    volatile uint32_t *const a_bsrr = &GPIOA->BSRR;
    volatile uint32_t *const c_bsrr = &GPIOC->BSRR;

    for (;;) {
        *a_bsrr = (*c_idr & txm) ? GPIO_PIN_2 : ((uint32_t)GPIO_PIN_2 << 16U);
        *c_bsrr = (*a_idr & GPIO_PIN_3) ? rx_set : rx_clr;
    }
}
