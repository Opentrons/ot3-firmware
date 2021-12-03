#include "common/firmware/adc.h"
#include "common/firmware/errors.h"

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
void MX_ADC1_Init(ADC_HandleTypeDef* adc1)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_InjectionConfTypeDef sConfigInjected = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  adc1->Instance = ADC1;
  adc1->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  adc1->Init.Resolution = ADC_RESOLUTION_12B;
  adc1->Init.ScanConvMode = ADC_SCAN_ENABLE;
  adc1->Init.ContinuousConvMode = DISABLE;
  adc1->Init.DiscontinuousConvMode = DISABLE;
  adc1->Init.DataAlign = ADC_DATAALIGN_LEFT;
  adc1->Init.NbrOfConversion = 1;
  adc1->Init.DMAContinuousRequests = DISABLE;
  adc1->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  adc1->Init.LowPowerAutoWait = DISABLE;
  adc1->Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(adc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(adc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Injected Channel
  */
  sConfigInjected.InjectedChannel = ADC_CHANNEL_2;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
  sConfigInjected.InjectedSingleDiff = ADC_SINGLE_ENDED;
  sConfigInjected.InjectedNbrOfConversion = 2;
  sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_12CYCLES_5;
  sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONV_EDGE_RISING;
  sConfigInjected.ExternalTrigInjecConv = ADC_EXTERNALTRIGINJEC_T1_TRGO;
  sConfigInjected.AutoInjectedConv = DISABLE;
  sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
  sConfigInjected.QueueInjectedContext = ENABLE;
  sConfigInjected.InjectedOffset = 0;
  sConfigInjected.InjectedOffsetNumber = ADC_OFFSET_NONE;
  if (HAL_ADCEx_InjectedConfigChannel(adc1, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Injected Channel
  */
  sConfigInjected.InjectedChannel = ADC_CHANNEL_14;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_2;
  if (HAL_ADCEx_InjectedConfigChannel(adc1, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
void MX_ADC2_Init(ADC_HandleTypeDef* adc2)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_InjectionConfTypeDef sConfigInjected = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */
  /** Common config
  */
  adc2->Instance = ADC2;
  adc2->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  adc2->Init.Resolution = ADC_RESOLUTION_12B;
  adc2->Init.ScanConvMode = ADC_SCAN_ENABLE;
  adc2->Init.ContinuousConvMode = DISABLE;
  adc2->Init.DiscontinuousConvMode = DISABLE;
  adc2->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  adc2->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  adc2->Init.DataAlign = ADC_DATAALIGN_LEFT;
  adc2->Init.NbrOfConversion = 2;
  adc2->Init.DMAContinuousRequests = DISABLE;
  adc2->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  adc2->Init.LowPowerAutoWait = DISABLE;
  adc2->Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(adc2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Injected Channel
  */
  sConfigInjected.InjectedChannel = ADC_CHANNEL_14;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
  sConfigInjected.InjectedSingleDiff = ADC_SINGLE_ENDED;
  sConfigInjected.InjectedNbrOfConversion = 2;
  sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_12CYCLES_5;
  sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONV_EDGE_RISING;
  sConfigInjected.ExternalTrigInjecConv = ADC_EXTERNALTRIGINJEC_T1_TRGO;
  sConfigInjected.AutoInjectedConv = DISABLE;
  sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
  sConfigInjected.QueueInjectedContext = ENABLE;
  sConfigInjected.InjectedOffset = 0;
  sConfigInjected.InjectedOffsetNumber = ADC_OFFSET_NONE;
  if (HAL_ADCEx_InjectedConfigChannel(adc2, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Injected Channel
  */
  sConfigInjected.InjectedChannel = ADC_CHANNEL_4;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_2;
  if (HAL_ADCEx_InjectedConfigChannel(adc2, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_6CYCLES_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(adc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(adc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

void adc_setup(ADC_HandleTypeDef* handle){
    if (HAL_OK != HAL_ADC_Init(handle)) {
        Error_Handler();
    }
    HAL_ADCEx_Calibration_Start(handle, ADC_SINGLE_ENDED);
}
