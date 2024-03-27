#include "hepa-uv/firmware/utility_gpio.h"

#include "platform_specific_hal_conf.h"
#include "stm32g4xx_hal_gpio.h"

/**
 * @brief LED Drive Initialization Function
 * @param None
 * @retval None
*/
void LED_drive_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /*Configure GPIO pin LED_DRIVE : PA1 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LED_DRIVE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_DRIVE_PORT, &GPIO_InitStruct);
}

/**
 * @brief Door Open GPIO Initialization Function
 * @param None
 * @retval None
*/
void door_open_input_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /*Configure GPIO pin DOOR_OPEN_MCU : PC7 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DOOR_OPEN_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DOOR_OPEN_MCU_PORT, &GPIO_InitStruct);
}

/**
 * @brief Reed Switch GPIO Initialization Function
 * @param None
 * @retval None
*/
void reed_switch_input_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /*Configure GPIO pin REED_SW_MCU : PC11 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = REED_SW_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(REED_SW_MCU_PORT, &GPIO_InitStruct);
}

/**
 * @brief AUX ID Initialization Function
 * @param None
 * @retval None
*/
void aux_input_gpio_init(void) {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /*Configure GPIO pin AUX_ID_MCU : PC12 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = AUX_ID_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(AUX_ID_MCU_PORT, &GPIO_InitStruct);
}

/**
 * @brief Hepa Push Button GPIO Initialization Function
 * @param None
 * @retval None
*/
void hepa_push_button_input_gpio_init(void) {
    /*Configure Ports Clock Enable*/
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /*Configure GPIO pin HEPA_NO_MCU : PB10 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = HEPA_NO_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(HEPA_NO_MCU_PORT, &GPIO_InitStruct);
}

/**
 * @brief UV Push Button GPIO Initialization Function
 * @param None
 * @retval None
*/
void uv_push_button_input_gpio_init(void) {
    /*Configure Ports Clock Enable*/
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /*Configure GPIO pin UV_NO_MCU : PC2 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = UV_NO_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(UV_NO_MCU_PORT, &GPIO_InitStruct);
}

/**
 * @brief HEPA ON/OFF GPIO Initialization Function
 * @param None
 * @retval None
*/
void hepa_on_off_output_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /*Configure GPIO pin LED_DRIVE : PA7 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = HEPA_ON_OFF_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(HEPA_ON_OFF_PORT, &GPIO_InitStruct);
}

/**
 * @brief UV ON/OFF GPIO Initialization Function
 * @param None
 * @retval None
*/
void uv_on_off_output_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /*Configure GPIO pin LED_DRIVE : PA4 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = UV_ON_OFF_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(UV_ON_OFF_MCU_PORT, &GPIO_InitStruct);
}

/**
 * @brief NVIC EXTI interrupt priority Initialization
 * @param None
 * @retval None
 */
static void nvic_priority_enable_init() {
    /* EXTI interrupt init DOOR_OPEN_MCU : PC7*/
    // PC7 -> GPIO_EXTI6 (EXTI9_5_IRQn)
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    /* EXTI interrupt init REED_SW_MCU : PC11*/
    /* EXTI interrupt init HEPA_NO_MCU : PB10*/
    // PC11 -> GPIO_EXTI11 (EXTI15_10_IRQn)
    // PB11 -> GPIO_EXTI11 (EXTI15_10_IRQn)
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    /* EXTI interrupt init UV_NO_MCU : PC2*/
    // PC2 -> GPIO_EXTI2 (EXTI2_IRQn)
    HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}

void utility_gpio_init(void) {
    LED_drive_gpio_init();
    nvic_priority_enable_init();
    door_open_input_gpio_init();
    reed_switch_input_gpio_init();
    aux_input_gpio_init();
    hepa_push_button_input_gpio_init();
    hepa_on_off_output_init();
    uv_push_button_input_gpio_init();
    uv_on_off_output_init();
}
