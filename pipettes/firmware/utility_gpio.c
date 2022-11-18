#include "common/firmware/utility_gpio.h"
#include "stm32g4xx_hal_conf.h"
#include "hardware_config.h"
#include "pipettes/core/pipette_type.h"

static void enable_gpio_port(void* port) {
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
}

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
    enable_gpio_port(GPIOC);
    if (pipette_type == NINETY_SIX_CHANNEL) {
        /*Configure GPIO pin :
         * PC12, front tip sense
         * PC7, rear tip sense
         * */
        GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_7;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    } else {
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

    // Enable limit switches
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    if (pipette_type == NINETY_SIX_CHANNEL) {
        /*
         * Right gear -> PC10
         * Left gear -> PB12
         * Plunger -> PA4
         */
        enable_gpio_port(GPIOC);
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        enable_gpio_port(GPIOB);
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        enable_gpio_port(GPIOA);
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    } else {
        enable_gpio_port(GPIOA);
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void encoder_gpio_init() {
    /* Peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    /* Encoder P Axis GPIO Configuration
    PA0     ------> CHANNEL B ----> GPIO_PIN_0
    PA1     ------> CHANNEL A ----> GPIO_PIN_1
     On EVT hardware, this is the same for single and 96 channel main boards.
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
    enable_gpio_port(hardware.port);

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
    enable_gpio_port(GPIOB);

    /*Configure GPIO pin*/
    GPIO_InitTypeDef sync_in_init = {0};
    sync_in_init.Pin = sync_in_hardware.pin;
    sync_in_init.Mode = GPIO_MODE_INPUT;
    sync_in_init.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(sync_in_hardware.port, &sync_in_init);

    PipetteHardwarePin sync_out_hardware =
        pipette_hardware_get_gpio(pipette_type, pipette_hardware_device_sync_out);

    enable_gpio_port(GPIOC);

    GPIO_InitTypeDef sync_out_init = {0};
    sync_out_init.Pin = sync_out_hardware.pin;
    sync_out_init.Mode = GPIO_MODE_OUTPUT_PP;
    sync_out_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(sync_out_hardware.port, &sync_out_init);
    HAL_GPIO_WritePin(sync_out_hardware.port, sync_out_hardware.pin, GPIO_PIN_SET);
}

void data_ready_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
    PipetteHardwarePin hardware =
            pipette_hardware_get_gpio(
                pipette_type, pipette_hardware_device_data_ready_front);
    enable_gpio_port(hardware.port);
    IRQn_Type exti_line = get_interrupt_line(pipette_type);
    if (pipette_type != SINGLE_CHANNEL && pipette_type != EIGHT_CHANNEL) {
        PipetteHardwarePin hardware_rear = pipette_hardware_get_gpio(
            pipette_type, pipette_hardware_device_data_ready_rear);
        enable_gpio_port(hardware_rear.port);
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


void estop_input_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
       /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (pipette_type == NINETY_SIX_CHANNEL) {
        /*Configure GPIO pin EStopin : PB9 */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    else {
        /*Configure GPIO pin EStopin : PA10 */
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    
}

void utility_gpio_init() {
    limit_switch_gpio_init();
    tip_sense_gpio_init();
    LED_drive_gpio_init();
    sync_drive_gpio_init();
    data_ready_gpio_init();
    encoder_gpio_init();
    estop_input_gpio_init();
}
