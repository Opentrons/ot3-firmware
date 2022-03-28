#include "common/firmware/utility_gpio.h"
#include "platform_specific_hal_conf.h"
#include "gantry/core/axis_type.h"

/**
 * @brief LED GPIO Initialization Function
 * @param None
 * @retval None
 */
void LED_drive_gpio_init(void) {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin : PB11 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * @brief Sync In GPIO Initialization Function
 * @param None
 * @retval None
 */
void sync_drive_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin : sync in*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = get_sync_pin();
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * @brief Limit Switch GPIO Initialization Function
 * @param None
 * @retval None
 */
void limit_switch_gpio_init(void) {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /*Configure GPIO pin : PC2 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}
void utility_gpio_init(void) {
    limit_switch_gpio_init();
    LED_drive_gpio_init();
    sync_drive_gpio_init();
}