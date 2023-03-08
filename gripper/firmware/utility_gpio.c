#include "common/firmware/utility_gpio.h"

#include "platform_specific_hal_conf.h"
#include "stm32g4xx_hal_gpio.h"

#if PCBA_PRIMARY_REVISION == 'b' || PCBA_PRIMARY_REVISION == 'a'
static uint16_t led_gpio_pin = GPIO_PIN_6;
static uint16_t z_limit_sw_pin = GPIO_PIN_7;
static GPIO_TypeDef* nsync_out_port = GPIOB;
static uint16_t nsync_out_pin = GPIO_PIN_6;
#else
static uint16_t led_gpio_pin = GPIO_PIN_10;
static uint16_t z_limit_sw_pin = GPIO_PIN_13;
static GPIO_TypeDef* nsync_out_port = GPIOD;
static uint16_t nsync_out_pin = GPIO_PIN_2;
#endif

/**
 * @brief Limit Switch GPIO Initialization Function
 * @param None
 * @retval None
 */
static void limit_switch_gpio_init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /*Configure GPIO pin Gripper : PC7 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = z_limit_sw_pin;
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
    GPIO_InitStruct.Pin = led_gpio_pin;
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
    GPIO_InitStruct.Pin = nsync_out_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(nsync_out_port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(nsync_out_port, nsync_out_pin, GPIO_PIN_SET);
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
    /* enable pin GPIO: C11 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
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
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOA,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                  &GPIO_InitStruct);
}

inline static void ebrake_gpio_init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
}

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
