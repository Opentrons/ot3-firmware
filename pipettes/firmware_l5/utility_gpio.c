#include "common/firmware/utility_gpio.h"
#include "platform_specific_hal_conf.h"
#include "hardware_config.h"
#include "pipettes/core/pipette_type.h"


/**
 * @brief Tip Sense GPIO Initialization Function
 * @param None
 * @retval None
 */
static void tip_sense_gpio_init() {
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

        __HAL_RCC_GPIOH_CLK_ENABLE();
        /*Configure GPIO pin : PH1, front tip sense */
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    } else {
        /* GPIO Ports Clock Enable */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /*Configure GPIO pin : A10 */
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}


/**
 * @brief Limit Switch GPIO Initialization Function
 * @param None
 * @retval None
 */
static void limit_switch_gpio_init() {
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
static void LED_drive_gpio_init() {
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

static void sync_drive_gpio_init() {
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

static void data_ready_gpio_init() {
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
        HAL_NVIC_SetPriority(EXTI8_IRQn, 10, 0);
        HAL_NVIC_EnableIRQ(EXTI8_IRQn);
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

int utility_gpio_tip_present() {
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_SET) ? 1 : 0;
}


static void estop_input_gpio_init() {
       /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /*Configure GPIO pin EStopin : PA9 */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void mount_id_init() {
    // B1: mount id
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void utility_gpio_init() {
    mount_id_init();
    limit_switch_gpio_init();
    tip_sense_gpio_init();
    LED_drive_gpio_init();
    sync_drive_gpio_init();
    data_ready_gpio_init();
    estop_input_gpio_init();
}

int utility_gpio_get_mount_id() {
    // If this line is low, it is a left pipette (returns 1)
    // if this line is high, it is a right pipette (returns 0)
    int level =  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (level == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    return (level == GPIO_PIN_RESET) ? 1 : 0;
}
