#include "motor_hardware.h"
#include "stm32g4xx_hal.h"

void initialize_rev_specific_pins() {
    // on rev1, we have electromagnetic brakes
    // for both motors and a correct sense for
    // the right motor enable pin
    // Left brake: PB5
    // Left enable: PC4
    // Right brake: PB0
    // Right enable: PB11
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_5 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                  &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5 | GPIO_PIN_0, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
}
