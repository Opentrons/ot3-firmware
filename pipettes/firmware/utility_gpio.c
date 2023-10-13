#include "common/firmware/utility_gpio.h"
#include "stm32g4xx_hal_conf.h"
#include "hardware_config.h"
#include "pipettes/core/pipette_type.h"
#include "stm32g4xx_hal_gpio.h"


static void enable_gpio_port(void* port) {
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (port == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
}

/**
 * @brief Tip Sense GPIO Initialization Function
 * @param None
 * @retval None
 */
static void tip_sense_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    enable_gpio_port(GPIOC);
    /*Configure GPIO pin : C2 */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}


/**
 * @brief NVIC EXTI interrupt priority Initialization
 * @param None
 * @retval None
 */
static void nvic_priority_enable_init() {
    if (pipette_type != NINETY_SIX_CHANNEL) {
        IRQn_Type block_2 = get_interrupt_line(gpio_block_2);
        /* EXTI interrupt init block tip sense*/
        HAL_NVIC_SetPriority(block_2, 10, 0);
        HAL_NVIC_EnableIRQ(block_2);
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
static void LED_drive_gpio_init() {
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

static void sync_drive_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
    PipetteHardwarePin sync_in_hardware =
        pipette_hardware_get_gpio(pipette_type, pipette_hardware_device_sync_in);
    /* GPIO Ports Clock Enable */
    enable_gpio_port(sync_in_hardware.port);

    /*Configure GPIO pin*/
    GPIO_InitTypeDef sync_in_init = {0};
    sync_in_init.Pin = sync_in_hardware.pin;
    sync_in_init.Mode = GPIO_MODE_INPUT;
    sync_in_init.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(sync_in_hardware.port, &sync_in_init);

    PipetteHardwarePin sync_out_hardware =
        pipette_hardware_get_gpio(pipette_type, pipette_hardware_device_sync_out);

    enable_gpio_port(sync_out_hardware.port);

    GPIO_InitTypeDef sync_out_init = {0};
    sync_out_init.Pin = sync_out_hardware.pin;
    sync_out_init.Mode = GPIO_MODE_OUTPUT_PP;
    sync_out_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(sync_out_hardware.port, &sync_out_init);
    HAL_GPIO_WritePin(sync_out_hardware.port, sync_out_hardware.pin, GPIO_PIN_SET);
}

static void data_ready_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
    // Note, only in this file are we using "front" to refer to
    // the single channel's data ready line. Unfortunately, the data ready
    // line on the single matches the front channel on the eight channel
    // but we normally will refer to the rear sensor as the primary sensor
    // in the rest of the codebase.
    PipetteHardwarePin hardware =
            pipette_hardware_get_gpio(
                pipette_type, pipette_hardware_device_data_ready_front);
    enable_gpio_port(hardware.port);
    if (pipette_type != SINGLE_CHANNEL) {
        PipetteHardwarePin hardware_front = pipette_hardware_get_gpio(
            pipette_type, pipette_hardware_device_data_ready_rear);
        enable_gpio_port(hardware_front.port);
        /*Configure GPIO pin*/
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = hardware_front.pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(hardware_front.port, &GPIO_InitStruct);

    }

    /*Configure GPIO pin*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = hardware.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(hardware.port, &GPIO_InitStruct);

}


static void estop_input_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (pipette_type == NINETY_SIX_CHANNEL) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /*Configure GPIO pin EStopin : PB9 */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    } else {
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /*Configure GPIO pin EStopin : PC12*/
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }
}

static void mount_id_init() {
    PipetteType pipette_type = get_pipette_type();
    if (pipette_type == NINETY_SIX_CHANNEL) {
        // C3: mount id
        __HAL_RCC_GPIOC_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    } else {
        // B0: mount id
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

void utility_gpio_init() {
    mount_id_init();
    nvic_priority_enable_init();
    limit_switch_gpio_init();
    tip_sense_gpio_init();
    LED_drive_gpio_init();
    sync_drive_gpio_init();
    data_ready_gpio_init();
    encoder_gpio_init();
    estop_input_gpio_init();
}

int utility_gpio_get_mount_id(PipetteType pipette_type) {
    // If this line is low, it is a left pipette (returns 1)
    // if this line is high, it is a right pipette (returns 0)
    int level = 0;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    void* port;
    if (pipette_type == NINETY_SIX_CHANNEL) {
        port = GPIOC;
        level = HAL_GPIO_ReadPin(port, GPIO_PIN_3);
        GPIO_InitStruct.Pin = GPIO_PIN_3;
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    } else {
        port = GPIOB;
        level =  HAL_GPIO_ReadPin(port, GPIO_PIN_0);
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    HAL_GPIO_WritePin(port, GPIO_InitStruct.Pin, (level == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    return (level == GPIO_PIN_RESET) ? 1 : 0;
}
