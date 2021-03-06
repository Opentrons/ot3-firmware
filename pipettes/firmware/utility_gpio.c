#include "common/firmware/utility_gpio.h"
#include "stm32g4xx_hal_conf.h"
#include "hardware_config.h"
#include "pipettes/core/pipette_type.h"


/**
 * @brief Tip Sense GPIO Initialization Function
 * @param None
 * @retval None
 */
void tip_sense_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    if (pipette_type == NINETY_SIX_CHANNEL) {
        /* GPIO Ports Clock Enable */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /*Configure GPIO pin : PC12, back tip sense */
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /*Configure GPIO pin : PH1, front tip sense */
        // TODO put correct pin configuration here
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    } else {
        /* GPIO Ports Clock Enable */
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /*Configure GPIO pin : C2 */
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }
}


/**
 * @brief Limit Switch GPIO Initialization Function
 * @param None
 * @retval None
 */
void limit_switch_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // Enable linear limit switch
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    if (pipette_type == NINETY_SIX_CHANNEL) {
        /*
         * Right gear -> PC14
         * Left gear -> PA10
         */
        GPIO_InitStruct.Pin = GPIO_PIN_14;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
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
    sync_out_init.Mode = GPIO_MODE_OUTPUT_PP;
    sync_out_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(sync_out_hardware.port, &sync_out_init);
    HAL_GPIO_WritePin(sync_out_hardware.port, sync_out_hardware.pin, GPIO_PIN_SET);
}

void data_ready_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
    PipetteHardwarePin hardware;
    IRQn_Type exti_line = get_interrupt_line(pipette_type);
    if (pipette_type == SINGLE_CHANNEL || pipette_type == EIGHT_CHANNEL)
    {
        hardware =
            pipette_hardware_get_gpio(
                pipette_type, pipette_hardware_device_data_ready_front);
    }
    else {
        hardware =
            pipette_hardware_get_gpio(
        pipette_type, pipette_hardware_device_data_ready_front);
        PipetteHardwarePin hardware_rear = pipette_hardware_get_gpio(pipette_type,
                                                                     pipette_hardware_device_data_ready_rear);
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /*Configure GPIO pin*/
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = hardware_rear.pin;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(hardware.port, &GPIO_InitStruct);

        /* EXTI interrupt init*/
        HAL_NVIC_SetPriority(exti_line, 10, 0);
        HAL_NVIC_EnableIRQ(exti_line);
    }

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /*Configure GPIO pin*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = hardware.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(hardware.port, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(exti_line, 10, 0);
    HAL_NVIC_EnableIRQ(exti_line);
}

void utility_gpio_init() {
    limit_switch_gpio_init();
    tip_sense_gpio_init();
    LED_drive_gpio_init();
    sync_drive_gpio_init();
    data_ready_gpio_init();
}
