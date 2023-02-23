#include "common/firmware/utility_gpio.h"

#include "platform_specific_hal_conf.h"
#include "stm32g4xx_hal_gpio.h"

/**
 * @brief Limit Switch GPIO Initialization Function
 * @param None
 * @retval None
 */
static void limit_switch_gpio_init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /*Configure GPIO pin Gripper : PC7 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pin Clamp: PC2 */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
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
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
 * @brief Sync In GPIO Initialization Function
 * @param None
 * @retval None
 */
static void sync_drive_gpio_init() {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /*Configure sync in GPIO pin : PB7*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure sync out GPIO pin : PB6*/
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
}

static void estop_input_gpio_init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /*Configure GPIO pin EStopin : PA10 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void tool_detect_gpio_init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}


static void g_motor_gpio_init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* enable pin GPIO: C11 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                  &GPIO_InitStruct);

    /* Encoder G Axis GPIO Configuration
    PA0     ------> CHANNEL A
    PA1     ------> CHANNEL B
    PA5    ------> CHANNEL I
    */
    __GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void z_motor_gpio_init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* Peripheral clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();
    /**SPI2 GPIO Configuration
    PB12  ------> SPI2_CS
    PB13  ------> SPI2_SCK
    PB14  ------> SPI2_MISO
    PB15  ------> SPI2_MOSI
    Step/Dir
    PB10   ------> Motor Dir Pin
    PB1   ------> Motor Step Pin
    Enable
    PA9   ------> Motor Enable Pin
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Chip select
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Dir
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Step
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                  &GPIO_InitStruct);

    // Enable
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOA,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                  &GPIO_InitStruct);
}

void utility_gpio_init(void) {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOF_CLK_ENABLE();

    limit_switch_gpio_init();
    LED_drive_gpio_init();
    sync_drive_gpio_init();
    estop_input_gpio_init();
    tool_detect_gpio_init();
    z_motor_gpio_init();
    g_motor_gpio_init();
}
