#include "gripper/firmware/utility_gpio.h"

#include "platform_specific_hal_conf.h"
#include "stm32g4xx_hal_gpio.h"

/**
 * @brief Limit Switch GPIO Initialization Function
 * @param None
 * @retval None
 */
static void limit_switch_gpio_init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /*Configure GPIO pin Gripper*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = Z_LIM_SW_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(Z_LIM_SW_PORT, &GPIO_InitStruct);

    /*Configure GPIO pin Clamp*/
    GPIO_InitStruct.Pin = G_LIM_SW_PIN;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(G_LIM_SW_PORT, &GPIO_InitStruct);
}

/**
 * @brief LED GPIO Initialization Function
 * @param None
 * @retval None
 */
static void LED_drive_gpio_init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /*Configure GPIO pin : PC6 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief Sync In GPIO Initialization Function
 * @param None
 * @retval None
 */
static void sync_drive_gpio_init() {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    #if PCBA_PRIMARY_REVISION == 'c'
    __HAL_RCC_GPIOD_CLK_ENABLE();
    #endif
    /*Configure sync in GPIO pin : PB7*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = NSYNC_IN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(NSYNC_IN_PORT, &GPIO_InitStruct);

    /*Configure sync out GPIO pin : PB6*/
    GPIO_InitStruct.Pin = NSYNC_OUT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(NSYNC_OUT_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(NSYNC_OUT_PORT, NSYNC_OUT_PIN, GPIO_PIN_SET);
}

static void estop_input_gpio_init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /*Configure GPIO pin EStopin : PA10 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ESTOP_IN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ESTOP_IN_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void tool_detect_gpio_init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = TOOL_DETECT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(TOOL_DETECT_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(TOOL_DETECT_PORT, TOOL_DETECT_PIN, GPIO_PIN_SET);
}


static void g_motor_gpio_init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /* enable pin GPIO: C11 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = G_MOT_ENABLE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(G_MOT_ENABLE_PORT,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                  &GPIO_InitStruct);
}

static void z_motor_gpio_init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /*
    Step/Dir
    PB10   ------> Motor Dir Pin
    PB1   ------> Motor Step Pin
    Enable
    PA9   ------> Motor Enable Pin
    Diag0
    PB2   ------> Motor Diag0 Pin
    */
    GPIO_InitStruct.Pin = Z_MOT_DIR_PIN | Z_MOT_STEP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(Z_MOT_STEPDIR_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = Z_MOT_ENABLE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(Z_MOT_ENABLE_PORT,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                  &GPIO_InitStruct);

    GPIO_InitStruct.Pin = Z_MOT_DIAG0_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(Z_MOT_STEPDIR_PORT, &GPIO_InitStruct);
}

#if PCBA_PRIMARY_REVISION != 'b' && PCBA_PRIMARY_REVISION != 'a'
inline static void ebrake_gpio_init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = EBRAKE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(EBRAKE_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(EBRAKE_PORT, EBRAKE_PIN, GPIO_PIN_RESET);
}
#endif

void utility_gpio_init(void) {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOF_CLK_ENABLE();

    #if PCBA_PRIMARY_REVISION != 'b' && PCBA_PRIMARY_REVISION != 'a'
        ebrake_gpio_init();
    #endif

    limit_switch_gpio_init();
    LED_drive_gpio_init();
    sync_drive_gpio_init();
    estop_input_gpio_init();
    tool_detect_gpio_init();
    z_motor_gpio_init();
    g_motor_gpio_init();
}
