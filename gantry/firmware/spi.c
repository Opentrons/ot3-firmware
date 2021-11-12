#include "common/firmware/spi.h"

#include "common/firmware/errors.h"
#include "platform_specific_hal_conf.h"

SPI_HandleTypeDef handle;

/**
 * @brief SPI MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hspi->Instance == SPI2) {
        /* Peripheral clock enable */
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**SPI2 GPIO Configuration
        PB12  ------> SPI2_CS
        PB13  ------> SPI2_SCK
        PB14  ------> SPI2_MISO
        PB15  ------> SPI2_MOSI
        Step/Dir
        PB1   ------> Motor Dir Pin
        PC8   ------> Motor Step Pin
        Enable
        PA9   ------> Motor Enable Pin
        */
        GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Chip select
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Dir
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Step
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                      &GPIO_InitStruct);

        // Enable
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOA,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                      &GPIO_InitStruct);
    }
}

/**
 * @brief SPI MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
    if (hspi->Instance == SPI2) {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                                   GPIO_PIN_15 | GPIO_PIN_1);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8);
    }
}

/**
 * @brief SPI2 Initialization Function
 * @param None
 * @retval None
 */
SPI_HandleTypeDef MX_SPI2_Init() {
    /* SPI2 parameter configuration*/
    __HAL_RCC_SPI2_CLK_ENABLE();
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

    if (HAL_SPI_Init(&hspi2) != HAL_OK) {
        Error_Handler();
    }
    return hspi2;
}

void SPI2_init() { handle = MX_SPI2_Init(); }
void SPI_init() {
    SPI2_init();
}
void Set_CS_Pin() { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); }

void Reset_CS_Pin() { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); }

void hal_transmit_receive(uint8_t* transmit, uint8_t* receive,
                          uint16_t buff_size, uint32_t timeout) {
    HAL_SPI_TransmitReceive(&handle, transmit, receive, buff_size, timeout);
}