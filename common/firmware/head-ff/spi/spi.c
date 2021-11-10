#include "common/firmware/spi.h"
#include "common/firmware/errors.h"

#include "platform_specific_hal_conf.h"

SPI_HandleTypeDef handle, handle_SPI2, handle_SPI3;

/**
 * @brief SPI MSP Initialization
 * This function configures SPI for the Z/A axis motors
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
    // PB14 -> MISO
    // PB12 -> CS
    // PB13 -> CLK
    // PB15 -> MOSI
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hspi->Instance == SPI2) {
        /* Peripheral clock enable */
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**SPI2 GPIO Configuration
        PB12     ------> SPI2_NSS
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_MISO
        PB15     ------> SPI2_MOSI
         Step/Dir
         PC7  ---> Dir Pin Motor A
         PC6  ---> Step Pin Motor A
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
           
        // PB11 A motor control  
        // ctrl /Dir/Step pin
        GPIO_InitStruct.Pin = GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                      &GPIO_InitStruct);
    }
    // PC11 -> MISO
    // PA04 -> CS
    // PC10 -> CLK
    // PC12 -> MOSI
    
    else if (hspi->Instance == SPI3) {
        /* Peripheral clock enable */
        __HAL_RCC_SPI3_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**SPI3 GPIO Configuration
        PA4     ------> SPI3_NSS
        PC10     ------> SPI3_SCK
        PC11     ------> SPI3_MISO
        PC12     ------> SPI3_MOSI
         Step/Dir
         PC1  ---> Dir Pin Motor on SPI3 (Z-axis)
         PC0  ---> Step Pin Motor on SPI3 (Z-axis)
        */
        GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        // Chip select
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // Ctrl/Dir/Step pin for motor on SPI3 (Z-axis)
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                      &GPIO_InitStruct);
    }
}



/**
 * @brief SPI MSP De-Initialization
 * This function denits SPI for Z/A motors
 * @param hspi: SPI2 handle pointer
 * @retval None
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
    if (hspi->Instance == SPI2) {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();

        /**SPI2 GPIO Configuration
        PB12     ------> SPI2_NSS
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_MISO
        PB15     ------> SPI2_MOSI
        
        Step/Dir
         PC7  ---> Dir Pin Motor on SPI2 (A-axis)
         PC6  ---> Step Pin Motor on SPI2 (A-axis)
        */
        HAL_GPIO_DeInit(GPIOB,
                        GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6 | GPIO_PIN_7);
    }
    else if (hspi->Instance == SPI3) {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();

        /**SPI3 GPIO Configuration
        PA04     ------> SPI3_NSS
        PC10     ------> SPI3_SCK
        PC11     ------> SPI3_MISO
        PC12     ------> SPI3_MOSI

        Step/Dir
         PC1  ---> Dir Pin Motor on SPI3 (Z-axis)
         PC0  ---> Step Pin Motor on SPI3 (Z-axis)
        */
        HAL_GPIO_DeInit(GPIOC,
                        GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_0 | GPIO_PIN_1);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
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

/**
 * @brief SPI3 Initialization Function
 * @param None
 * @retval None
 */
SPI_HandleTypeDef MX_SPI3_Init() {
    /* SPI2 parameter configuration*/
    __HAL_RCC_SPI3_CLK_ENABLE();
    SPI_HandleTypeDef hspi3 = {
        .Instance = SPI3,
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

    if (HAL_SPI_Init(&hspi3) != HAL_OK) {
        Error_Handler();
    }
    return hspi3;
}

void SPI2_init() {
    handle_SPI2 = MX_SPI2_Init();
}

void SPI3_init() {
    handle_SPI3 = MX_SPI3_Init();
}

void Set_CS_Pin() { 
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

}

void Reset_CS_Pin() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
}

void hal_transmit_receive(uint8_t* transmit, uint8_t* receive, uint16_t buff_size, uint32_t timeout) {
    HAL_SPI_TransmitReceive(&handle, transmit, receive, buff_size, timeout);
}
void hal_transmit_receive_SPI2(uint8_t* transmit, uint8_t* receive, uint16_t buff_size, uint32_t timeout) {
    HAL_SPI_TransmitReceive(&handle_SPI2, transmit, receive, buff_size, timeout);
}
void hal_transmit_receive_SPOI3(uint8_t* transmit, uint8_t* receive, uint16_t buff_size, uint32_t timeout) {
    HAL_SPI_TransmitReceive(&handle_SPI3, transmit, receive, buff_size, timeout);
}