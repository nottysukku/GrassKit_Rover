/**
  ******************************************************************************
  * @file           : stm32l0xx_it.c
  * @brief          : Interrupt Service Routines implementation.
  ******************************************************************************
  */

#include "main.h"
#include "stm32l0xx_it.h"

/* External Peripheral Handles */
extern UART_HandleTypeDef huart1_bt;
extern UART_HandleTypeDef huart2_debug;
extern DMA_HandleTypeDef  hdma_spi2_tx;

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1) {
    }
}

void SVC_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

/**
  * @brief Handles USART1 (External Bluetooth Module) interrupt.
  */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1_bt);
}

/**
  * @brief Handles USART2 (ST-LINK Virtual COM Port) interrupt.
  */
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2_debug);
}

/**
  * @brief Handles DMA1 Channel 4..7 (I2S Audio TX) interrupt.
  */
void DMA1_Channel4_5_6_7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_spi2_tx);
}
