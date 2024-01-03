#include "motor_hardware.h"
#include "system_stm32g4xx.h"


static motor_interrupt_callback timer_callback = NULL;
static z_encoder_overflow_callback z_enc_overflow_callback = NULL;
static brushed_motor_interrupt_callback brushed_timer_callback = NULL;
static encoder_overflow_callback gripper_enc_overflow_callback = NULL;
static encoder_idle_state_callback gripper_enc_idle_state_overflow_callback =
    NULL;
static stopwatch_overflow_callback gripper_force_stopwatch_overflow_callback = NULL;
static z_motor_disengage_callback disengage_z_callback = NULL;


void set_z_motor_timer_callback(
        motor_interrupt_callback callback,
        z_encoder_overflow_callback enc_callback,
        z_motor_disengage_callback disengage_callback) {
    timer_callback = callback;
    z_enc_overflow_callback = enc_callback;
    disengage_z_callback = disengage_callback;
}

void set_brushed_motor_timer_callback(
        brushed_motor_interrupt_callback callback,
        encoder_overflow_callback g_enc_f_callback,
        encoder_idle_state_callback g_enc_idle_callback,
        stopwatch_overflow_callback g_stopwatch_overflow_callback) {
    brushed_timer_callback = callback;
    gripper_enc_overflow_callback = g_enc_f_callback;
    gripper_enc_idle_state_overflow_callback = g_enc_idle_callback;
    gripper_force_stopwatch_overflow_callback = g_stopwatch_overflow_callback;
}


/**
  * @brief  Initializes the TIM Base MSP.
  * @param  htim TIM Base handle
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim) {
    if (htim == &htim7) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM7_CLK_ENABLE();
        /* TIM7 interrupt Init */
        HAL_NVIC_SetPriority(TIM7_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM7_IRQn);

    } else if (htim == &htim1) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM1_CLK_ENABLE();
        /* TIM1 interrupt Init */
        HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
        HAL_NVIC_SetPriority(TIM1_CC_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
        // GPIO init being handled in HAL_TIM_Base_MspPostInit

    } else if (htim == &htim3) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
        /* TIM3 interrupt Init */
        HAL_NVIC_SetPriority(TIM3_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
        // GPIO init being handled in HAL_TIM_Base_MspPostInit

    } else if (htim == &htim4) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM4_CLK_ENABLE();
        /* TIM4 interrupt Init */
        HAL_NVIC_SetPriority(TIM4_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(TIM4_IRQn);

    } else if (htim == &htim15) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM15_CLK_ENABLE();
        /* TIM15 interrupt Init */
        HAL_NVIC_SetPriority(TIM1_BRK_TIM15_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);

    }
}


void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base) {
    if (htim_base->Instance == TIM7) {
        /* Peripheral clock disable */
        __HAL_RCC_TIM7_CLK_DISABLE();
        /* TIM7 interrupt DeInit */
        HAL_NVIC_DisableIRQ(TIM7_IRQn);

    } else if (htim_base->Instance == TIM1) {
        /* Peripheral clock disable */
        __HAL_RCC_TIM1_CLK_DISABLE();
        /* TIM1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(TIM1_UP_TIM16_IRQn);
        HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
        HAL_GPIO_DeInit(G_MOT_PWM_CH1_PORT, G_MOT_PWM_CH1_PIN);

    } else if (htim_base->Instance == TIM3) {
        /* Peripheral clock disable */
        __HAL_RCC_TIM3_CLK_DISABLE();
        /* TIM3 interrupt DeInit */
        HAL_NVIC_DisableIRQ(TIM3_IRQn);
        HAL_GPIO_DeInit(G_MOT_PWM_CH2_PORT, G_MOT_PWM_CH2_PIN);

    } else if (htim_base->Instance == TIM4) {
        /* Peripheral clock disable */
        __HAL_RCC_TIM4_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(TIM4_IRQn);
    } else if (htim_base->Instance == TIM15) {
        /* Peripheral clock disable */
        __HAL_RCC_TIM15_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(TIM1_BRK_TIM15_IRQn);
    }
}

/**
  * @brief  Initializes the TIM Encoder Interface MSP.
  * @param  htim TIM Encoder Interface handle
  * @retval None
  */
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim) {
    if (htim == &htim2) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM2_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        /** Encoder G Axis GPIO Configuration
            PA0     ------> CHANNEL A
            PA1     ------> CHANNEL B
            PA5    ------> CHANNEL I (UNUSED)
        */
        GPIO_InitStruct.Pin = G_MOT_ENC_A_PIN | G_MOT_ENC_B_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(G_MOT_ENC_PORT, &GPIO_InitStruct);

        /* TIM2 interrupt Init */
        HAL_NVIC_SetPriority(TIM2_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    } else if (htim == &htim8) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM8_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**TIM8 GPIO Configuration
            PC6     ------> TIM8_CH1
            PC7     ------> TIM8_CH2
        */
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = Z_MOT_ENC_A_PIN|Z_MOT_ENC_B_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_TIM8;
        HAL_GPIO_Init(Z_MOT_ENC_AB_PORT, &GPIO_InitStruct);
        
    }
}

/**
  * @brief  DeInitializes TIM Encoder Interface MSP.
  * @param  htim TIM Encoder Interface handle
  * @retval None
  */
void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef *htim) {
    if (htim == &htim2) {
        __HAL_RCC_TIM2_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(TIM2_IRQn);
        HAL_GPIO_DeInit(G_MOT_ENC_PORT, G_MOT_ENC_A_PIN | G_MOT_ENC_B_PIN);
    } else if (htim == &htim8) {
        __HAL_RCC_TIM8_CLK_DISABLE();
        HAL_GPIO_DeInit(Z_MOT_ENC_AB_PORT, Z_MOT_ENC_A_PIN|Z_MOT_ENC_B_PIN);
    }
}

__attribute__((section(".ccmram")))
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    // Check which version of the timer triggered this callback
    if (htim == &htim7 && timer_callback) {
        timer_callback();

    } else if (htim == &htim1 && brushed_timer_callback) {
        brushed_timer_callback();

    } else if (htim == &htim2 && gripper_enc_overflow_callback) {
        uint32_t direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(htim);
        gripper_enc_overflow_callback(direction ? -1 : 1);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);

    } else if (htim == &htim4 && gripper_enc_idle_state_overflow_callback) {
        gripper_enc_idle_state_overflow_callback(true);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);

    } else if (htim == &htim8 && z_enc_overflow_callback) {
        uint32_t direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(htim);
        z_enc_overflow_callback(direction ? -1 : 1);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    } else if (htim == &htim15 && gripper_force_stopwatch_overflow_callback) {
        gripper_force_stopwatch_overflow_callback(
            GRIPPER_ENCODER_FORCE_STOPWATCH_PERIOD/GRIPPER_ENCODER_FORCE_STOPWATCH_FREQ);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim) {
    if (htim == &htim4 && gripper_enc_idle_state_overflow_callback) {
        gripper_enc_idle_state_overflow_callback(false);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC1);
    } else if (htim == &htim15) {
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC1);
    }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == Z_MOT_ENABLE_PIN && disengage_z_callback) {
        disengage_z_callback();
    }
}