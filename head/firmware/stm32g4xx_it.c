/**
 ******************************************************************************
 * @file    Templates/Src/stm32g4xx_it.c
 * @author  MCD Application Team
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_it.h"

#include "FreeRTOSConfig.h"
#include "can/firmware/hal_can.h"
#include "stm32g4xx_hal.h"

/** @addtogroup STM32G4xx_HAL_Examples
 * @{
 */

/** @addtogroup Templates
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi1_rx;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

extern void motor_callback_glue();
/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief   This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void) {}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void) {
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
    }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void) {
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1) {
    }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void) {
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1) {
    }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void) {
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) {
    }
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void) {}

/******************************************************************************/
/*                 STM32G4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32g4xxxx.s).                                             */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
{
}*/
/**
 * @brief This function handles DMA1 channel2 global interrupt.
 */
void DMA1_Channel2_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_spi1_rx); }

/**
 * @brief This function handles DMA1 channel3 global interrupt.
 */
void DMA1_Channel3_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_spi1_tx); }

/**
 * @brief This function handles FDCAN1 interrupt 0.
 */
void FDCAN1_IT0_IRQHandler(void) {
    HAL_FDCAN_IRQHandler(can_get_device_handle());
}

/**
  * @brief This function handles EXTI line4 interrupt.
  */
void EXTI4_IRQHandler(void) {
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4)) {
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    }
}

/**
 * @brief This function handles TIM7 global interrupt.
 */
#ifdef ENABLE_CCMRAM
__attribute__((section( ".ccmram" )))
#endif
void TIM7_IRQHandler(void) { 
    // We ONLY ever enable the Update interrupt, so for a small efficiency gain
    // we always make the assumption that this interrupt was triggered by the
    // TIM_IT_UPDATE source.
    __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
    motor_callback_glue();
 }

/**
 * @brief This function handles TIM2 global interrupt.
 */
void TIM2_IRQHandler(void) { HAL_TIM_IRQHandler(&htim2); }

/**
 * @brief This function handles TIM3 global interrupt.
 */
void TIM3_IRQHandler(void) { HAL_TIM_IRQHandler(&htim3); }

void EXTI15_10_IRQHandler(void) {
    // make this get/deduce pin! Needs to work for C13/z and C15/a!
    EXTI_HandleTypeDef hexti_13 = {.Line = EXTI_LINE_13};
    EXTI_HandleTypeDef hexti_15 = {.Line = EXTI_LINE_15};
    if (HAL_EXTI_GetPending(&hexti_13, 0)) {
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    } else if (HAL_EXTI_GetPending(&hexti_15, 0)) {
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
    }
}

extern void xPortSysTickHandler(void);
void SysTick_Handler(void) {
    HAL_IncTick();
    xPortSysTickHandler();
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
