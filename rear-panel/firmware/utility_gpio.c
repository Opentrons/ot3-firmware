#include "rear-panel/firmware/utility_gpio.h"

/*  TODO
    IO1_MCU_OUT PA4
    IO2_MCU_OUT PA5
    IO1_MCU_IN PC4
    IO2_MCU_IN PC5
    DOOR_OPEN_MCU PB0
    AUX1_ID_MCU PB1
    AUX2_ID_MCU PB2
    AUX1_PRESENT_MCU PB4
    AUX2_PRESENT_MCU PB6
    ESTOP_DETECT_AUX2_MCU PB12
    ESTOP_DETECT_AUX1_MCU PB13
*/

/**
 * @brief Sync In GPIO Initialization Function
 * @param None
 * @retval None
 */
void sync_drive_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure sync in GPIO pin : PA6*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = SYNC_MCU_IN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SYNC_MCU_IN_PORT, &GPIO_InitStruct);

    /*Configure sync out GPIO pin : PA1*/
    GPIO_InitStruct.Pin = SYNC_MCU_OUT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(SYNC_MCU_OUT_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(SYNC_MCU_OUT_PORT, SYNC_MCU_OUT_PIN, SYNC_MCU_OUT_AS);
}

void estop_output_gpio_init() {
       /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin EStopin : PA0 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ESTOP_MCU_OUT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ESTOP_MCU_OUT_PORT, &GPIO_InitStruct);
    //disable estop for the other boards, we don't use the ESTOP_MCU_OUT_AS value here since that would enable estop
    HAL_GPIO_WritePin(ESTOP_MCU_OUT_PORT, ESTOP_MCU_OUT_PIN, GPIO_PIN_SET);
}

void estop_input_gpio_init() {
       /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin EStopin : PA7 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ESTOP_MCU_IN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ESTOP_MCU_IN_PORT, &GPIO_InitStruct);
}

void utility_gpio_init(void) {
    sync_drive_gpio_init();
    estop_output_gpio_init();
    //estop_input_gpio_init();
}
