#include "common/firmware/utility_gpio.h"
#include "platform_specific_hal_conf.h"
#include "hardware_config.h"
/**
 * @brief Limit Switch GPIO Initialization Function
 * @param None
 * @retval None
 */
void limit_switch_gpio_init(void) {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    PipetteHardwarePin hardware = pipette_hardware_get_gpio(
            pipette_hardware_device_limit_switch);

    /*Configure GPIO pin*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = hardware.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(hardware.port, &GPIO_InitStruct);
}

/**
 * @brief LED GPIO Initialization Function
 * @param None
 * @retval None
 */
void LED_drive_gpio_init(void) {
    PipetteHardwarePin hardware =
        pipette_hardware_get_gpio(
            pipette_hardware_device_LED_drive);

    /* GPIO Ports Clock Enable */
    if (hardware.port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if (hardware.port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }

    /*Configure GPIO pin*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = hardware.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(hardware.port, &GPIO_InitStruct);
}

void sync_drive_gpio_init() {
    PipetteHardwarePin hardware =
        pipette_hardware_get_gpio(pipette_hardware_device_sync_in);
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = hardware.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(hardware.port, &GPIO_InitStruct);
}

void utility_gpio_init(void) {
    limit_switch_gpio_init();
    LED_drive_gpio_init();
    sync_drive_gpio_init();
}
