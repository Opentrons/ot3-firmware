#include "head/core/za_gpio_constants.h"

/**
 * @brief LED Driver GPIO Initialization Function
 * @param None
 * @retval None
 */

void LED_drive_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin : PB6 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * @brief Limit Switch Z Initialization Function
 * @param None
 * @retval None
 */
void limit_switch_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin : PB7:z and PB9:a*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void turn_on_LED_pin() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
}

void turn_off_LED_pin() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
}

void check_limit_switch(MountType mount) {
    if (mount == left || mount == both) {
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7)) {
            turn_on_LED_pin(mount);
        } else {
            turn_off_LED_pin(mount);
        }
    }
    if (mount == right || mount == both) {
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9)) {
            turn_on_LED_pin(mount);
        } else {
            turn_off_LED_pin(mount);
        }

    }
}

void utility_gpio_init() {
    limit_switch_gpio_init();
    LED_drive_gpio_init();
}