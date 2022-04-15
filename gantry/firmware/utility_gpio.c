#include "common/firmware/utility_gpio.h"
#include "platform_specific_hal_conf.h"
#include "axis_hardware_config.h"

/**
 * @brief LED GPIO Initialization Function
 * @param None
 * @retval None
 */

void LED_drive_gpio_init(void) {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GantryHardwarePin hardware =
        gantry_hardware_get_gpio(gantry_hardware_device_LED_drive);

    /*Configure GPIO pin : PB11 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = hardware.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(hardware.port, &GPIO_InitStruct);
}

/**
 * @brief Sync In GPIO Initialization Function
 * @param None
 * @retval None
 */
void sync_drive_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GantryHardwarePin hardware =
        gantry_hardware_get_gpio(gantry_hardware_device_sync_in);

    /*Configure GPIO pin : sync in*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = hardware.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(hardware.port, &GPIO_InitStruct);

    hardware = gantry_hardware_get_gpio(gantry_hardware_device_sync_out);
    GPIO_InitStruct.Pin = hardware.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(hardware.port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(hardware.port, hardware.pin, GPIO_PIN_SET);
}


#include "platform_specific_hal_conf.h"

/**
 * @brief Limit Switch GPIO Initialization Function
 * @param None
 * @retval None
 */
void limit_switch_gpio_init(void) {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GantryHardwarePin hardware =
        gantry_hardware_get_gpio(gantry_hardware_device_limit_switch);

    /*Configure GPIO pin : PC2 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = hardware.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(hardware.port, &GPIO_InitStruct);
}
void utility_gpio_init(void) {
    limit_switch_gpio_init();
    LED_drive_gpio_init();
    sync_drive_gpio_init();
}
