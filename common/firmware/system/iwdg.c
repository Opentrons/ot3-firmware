#include "platform_specific_hal_conf.h"
#include "common/firmware/errors.h"
#include "common/firmware/iwdg.h"


static IWDG_HandleTypeDef hiwdg;


void MX_IWDG_Init(void) {
    /* USER CODE END IWDG_Init 1 */
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
    hiwdg.Init.Window = IWDG_WINDOW_DISABLE;
    hiwdg.Init.Reload = IWDG_INTERVAL_MS;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        Error_Handler();
    }
}


void iwdg_refresh(void) {
    if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK) {
        /* Refresh Error */
        Error_Handler();
    }
}
