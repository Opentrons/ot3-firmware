#include "common/firmware/spi.h"

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
SPI_HandleTypeDef MX_SPI1_Init() {

    /* USER CODE BEGIN SPI2_Init 0 */

    /* USER CODE END SPI2_Init 0 */

    /* USER CODE BEGIN SPI2_Init 1 */

    /* USER CODE END SPI2_Init 1 */
    /* SPI2 parameter configuration*/
    SPI_HandleTypeDef hspi1 = {
        .Instance = SPI2,
        .Init = {
            .Mode = SPI_MODE_MASTER,
            .Direction = SPI_DIRECTION_2LINES,
            .DataSize = SPI_DATASIZE_8BIT,
            .CLKPolarity = SPI_POLARITY_LOW,
            .CLKPhase = SPI_PHASE_1EDGE,
            .NSS = SPI_NSS_HARD_OUTPUT,
            .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32,
            .FirstBit = SPI_FIRSTBIT_MSB,
            .TIMode = SPI_TIMODE_DISABLE,
            .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
            .CRCPolynomial = 7,
            .CRCLength = SPI_CRC_LENGTH_DATASIZE,
            .NSSPMode = SPI_NSS_PULSE_ENABLE
        }

    };

    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Spi_Error_Handler();
    }
    /* USER CODE BEGIN SPI2_Init 2 */

    /* USER CODE END SPI2_Init 2 */
    return hspi1;
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init(void)
{

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(
        GPIOB,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        &GPIO_InitStruct);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
}

/**
  * Enable DMA controller clock
  */
void MX_DMA_Init(void)
{

    /* DMA controller clock enable */
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Channel2_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
    /* DMA1_Channel3_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}


void Spi_Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}


