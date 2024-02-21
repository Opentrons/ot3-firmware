#include "motor_hardware.h"
#include "pipettes/core/pipette_type.h"
#include "common/firmware/errors.h"
#include "stm32g4xx_hal.h"
#include "hardware_config.h"


void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hspi->Instance == SPI2) {
        /**SPI2 GPIO Configuration
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
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        if (pipette_type == NINETY_SIX_CHANNEL) {
            GPIO_InitStruct.Pin = pipette_hardware_spi_pins(pipette_type, GPIOC);
            HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
            HAL_GPIO_WritePin(GPIOC, GPIO_InitStruct.Pin, GPIO_PIN_SET);

            GPIO_InitStruct.Pin = GPIO_PIN_11;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
            HAL_GPIO_WritePin(GPIOB, GPIO_InitStruct.Pin, GPIO_PIN_SET);
        } else {
            GPIO_InitStruct.Pin = GPIO_PIN_12;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
            HAL_GPIO_WritePin(GPIOB, GPIO_InitStruct.Pin, GPIO_PIN_SET);
        }

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
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_CIPO
        PB15     ------> SPI2_COPI

        */
        PipetteType pipette_type = get_pipette_type();
        HAL_GPIO_DeInit(GPIOB, pipette_hardware_spi_pins(pipette_type, GPIOB));
        HAL_GPIO_DeInit(GPIOC, pipette_hardware_spi_pins(pipette_type, GPIOC));
    }
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
        // Driver Clock Pin.
        // Enable Dir/Step pin
        GPIO_InitStruct.Pin = pipette_hardware_motor_driver_pins(pipette_type, GPIOA);
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        // Diag0 pin
        GPIO_InitStruct.Pin = pipette_hardware_motor_driver_diag0_pin(pipette_type);
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    } else {
        // Enable Dir/Step pin
        GPIO_InitStruct.Pin = pipette_hardware_motor_driver_pins(pipette_type, GPIOA);
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        // Enable/Dir/Step pin
        GPIO_InitStruct.Pin = pipette_hardware_motor_driver_pins(pipette_type, GPIOB);
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        // Diag0 pin
        GPIO_InitStruct.Pin = pipette_hardware_motor_driver_diag0_pin(pipette_type);
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
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

