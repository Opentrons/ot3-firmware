#include "motor_hardware.h"
#include "pipettes/core/pipette_type.h"
#include "common/firmware/errors.h"
#include "stm32l5xx_hal.h"
#include "hardware_config.h"


TIM_HandleTypeDef htim7;
static motor_interrupt_callback plunger_callback = NULL;

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hspi->Instance == SPI2) {
        /**SPI2 GPIO Configuration
        PC6     ------> SPI2_CS
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_CIPO
        PB15     ------> SPI2_COPI

        */
        PipetteType pipette_type = get_pipette_type();
        GPIO_InitStruct.Pin = pipette_hardware_spi_pins(pipette_type, GPIOB);
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Chip Select
        GPIO_InitStruct.Pin = pipette_hardware_spi_pins(pipette_type, GPIOC);
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }
}

SPI_HandleTypeDef hspi2 = {
    .Instance = SPI2,
    .Init = {.Mode = SPI_MODE_MASTER,
             .Direction = SPI_DIRECTION_2LINES,
             .DataSize = SPI_DATASIZE_8BIT,
             .CLKPolarity = SPI_POLARITY_HIGH,
             .CLKPhase = SPI_PHASE_2EDGE,
             .NSS = SPI_NSS_SOFT,
             .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32,
             .FirstBit = SPI_FIRSTBIT_MSB,
             .TIMode = SPI_TIMODE_DISABLE,
             .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
             .CRCPolynomial = 7,
             .CRCLength = SPI_CRC_LENGTH_DATASIZE,
             .NSSPMode = SPI_NSS_PULSE_DISABLE}

};

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
    if (hspi->Instance == SPI2) {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();

        /**SPI2 GPIO Configuration
        PC6     ------> SPI2_CS
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_CIPO
        PB15     ------> SPI2_COPI

         Step/Dir
         PC3  ---> Dir Pin
         PC7  ---> Step Pin
         Enable
         PC8  ---> Enable Pin
        */
        PipetteType pipette_type = get_pipette_type();
        HAL_GPIO_DeInit(GPIOB, pipette_hardware_spi_pins(pipette_type, GPIOB));
        HAL_GPIO_DeInit(GPIOC, pipette_hardware_spi_pins(pipette_type, GPIOC));
    }
}

void motor_driver_CLK_gpio_init() {
    // Driver Clock Pin
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
}

void motor_driver_gpio_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    PipetteType pipette_type = get_pipette_type();

    GPIO_InitStruct.Pin = pipette_hardware_motor_driver_pins(pipette_type, GPIOC);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    if (pipette_type != NINETY_SIX_CHANNEL) {
        /*
         * (TODO: 25-04-2022) We will be switching all
         * motor drivers to tmc2160 and will no longer need
         * to wire up the motor drive clocks to an external oscillator.
         *
         * The 96 channel already uses the internal clock so we
         * should ignore this setup when we're compiling for the 96 channel.
         */
        // Driver Clock Pin.
        motor_driver_CLK_gpio_init();
    } else {
        // Enable Dir/Step pin
        GPIO_InitStruct.Pin = pipette_hardware_motor_driver_pins(pipette_type, GPIOA);
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        // Enable/Dir/Step pin
        GPIO_InitStruct.Pin = pipette_hardware_motor_driver_pins(pipette_type, GPIOB);
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = pipette_hardware_motor_driver_pins(pipette_type, GPIOD);
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    }

}

HAL_StatusTypeDef initialize_spi(void) {
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    motor_driver_gpio_init();
    return HAL_SPI_Init(&hspi2);
}

void MX_TIM7_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim7.Instance = TIM7;
    htim7.Init.Prescaler = 499;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = 1;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    // Check which version of the timer triggered this callback
    if ((htim == &htim7) && plunger_callback) {
        plunger_callback();
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
    if (htim == &htim7) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM7_CLK_ENABLE();

        /* TIM7 interrupt Init */
        HAL_NVIC_SetPriority(TIM7_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM7_IRQn);
    }
}

void initialize_timer(motor_interrupt_callback callback) {
    plunger_callback = callback;
    MX_TIM7_Init();
}
