#include "rear-panel/firmware/utility_gpio.h"

/*  TODO
    IO1_MCU_OUT PA4
    IO2_MCU_OUT PA5
    IO1_MCU_IN PC4
    IO2_MCU_IN PC5
*/

/**
 * @brief AUX Present and ID line setup
 * @param none
 * @retval none
 */
void aux_input_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*Aux1 ID pin*/
    GPIO_InitStruct.Pin = AUX1_ID_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(AUX1_ID_MCU_PORT, &GPIO_InitStruct);

    /*Aux2 ID pin*/
    GPIO_InitStruct.Pin = AUX2_ID_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(AUX2_ID_MCU_PORT, &GPIO_InitStruct);

    /*Aux1 present pin*/
    GPIO_InitStruct.Pin = AUX1_PRESENT_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(AUX1_PRESENT_MCU_PORT, &GPIO_InitStruct);

    /*Aux2 present pin*/
    GPIO_InitStruct.Pin = AUX2_PRESENT_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(AUX2_PRESENT_MCU_PORT, &GPIO_InitStruct);
}
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
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /*Configure GPIO pin EStopin : PA7 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ESTOP_MCU_IN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ESTOP_MCU_IN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ESTOP_DETECT_AUX1_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ESTOP_DETECT_AUX1_MCU_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ESTOP_DETECT_AUX2_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ESTOP_DETECT_AUX2_MCU_PORT, &GPIO_InitStruct);
}

void door_open_input_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /*Configure GPIO pin DOOR_OPEN : PB0 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DOOR_OPEN_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DOOR_OPEN_MCU_PORT, &GPIO_InitStruct);
}

void pgood_input_gpio_init() {
    #if !(PCBA_PRIMARY_REVISION == 'b' && PCBA_SECONDARY_REVISION == '1')
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /*Configure GPIO pin PGOOD : PA8 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = PGOOD_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(PGOOD_MCU_PORT, &GPIO_InitStruct);
    #endif
}

static void heartbeat_led_init() {
    /*Configure GPIO pin*/
#if !(PCBA_PRIMARY_REVISION == 'b' && PCBA_SECONDARY_REVISION == '1')
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = HEARTBEAT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(HEARTBEAT_PORT, &GPIO_InitStruct);
#endif
}


void utility_gpio_init(void) {
    heartbeat_led_init();
    sync_drive_gpio_init();
    estop_output_gpio_init();
    estop_input_gpio_init();
    door_open_input_gpio_init();
    aux_input_gpio_init();
    pgood_input_gpio_init();
}
