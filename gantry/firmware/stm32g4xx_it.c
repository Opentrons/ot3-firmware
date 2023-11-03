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
 * @brief This function handles TIM7 global interrupt.
 */
void TIM7_IRQHandler(void) { HAL_TIM_IRQHandler(&htim7); }
void TIM2_IRQHandler(void) { HAL_TIM_IRQHandler(&htim2); }
void EXTI9_5_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5); }


extern void xPortSysTickHandler(void);
void SysTick_Handler(void) {
    HAL_IncTick();
    xPortSysTickHandler();
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
