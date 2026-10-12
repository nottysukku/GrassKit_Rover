/**
  ******************************************************************************
  * @file           : stm32l0xx_it.h
  * @brief          : This file contains the headers of the interrupt handlers.
  ******************************************************************************
  */

#ifndef __STM32L0xx_IT_H
#define __STM32L0xx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void DMA1_Channel4_5_6_7_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32L0xx_IT_H */
