#include "common/firmware/utility_gpio.h"
#include "platform_specific_hal_conf.h"
#include "hardware_config.h"
#include "pipettes/core/pipette_type.h"
/**
 * @brief Limit Switch GPIO Initialization Function
 * @param None
 * @retval None
 */
void limit_switch_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    PipetteHardwarePin hardware = pipette_hardware_get_gpio(
            pipette_type, pipette_hardware_device_limit_switch);

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
void LED_drive_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
    PipetteHardwarePin hardware =
        pipette_hardware_get_gpio(
            pipette_type, pipette_hardware_device_LED_drive);

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
    PipetteType pipette_type = get_pipette_type();
    PipetteHardwarePin sync_in_hardware =
        pipette_hardware_get_gpio(pipette_type, pipette_hardware_device_sync_in);
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin*/
    GPIO_InitTypeDef sync_in_init = {0};
    sync_in_init.Pin = sync_in_hardware.pin;
    sync_in_init.Mode = GPIO_MODE_INPUT;
    sync_in_init.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(sync_in_hardware.port, &sync_in_init);

    PipetteHardwarePin sync_out_hardware =
        pipette_hardware_get_gpio(pipette_type, pipette_hardware_device_sync_out);

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef sync_out_init = {0};
    sync_out_init.Pin = sync_out_hardware.pin;
    sync_out_init.Mode = GPIO_MODE_OUTPUT_OD;
    sync_out_init.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(sync_out_hardware.port, &sync_out_init);
    HAL_GPIO_WritePin(sync_out_hardware.port, sync_out_hardware.pin, GPIO_PIN_SET);
}

void utility_gpio_init() {
    limit_switch_gpio_init();
    LED_drive_gpio_init();
    sync_drive_gpio_init();
}
