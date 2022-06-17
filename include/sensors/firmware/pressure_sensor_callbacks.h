#include "platform_specific_hal_conf.h"
#include "sensors/core/tasks/pressure_driver.hpp"

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_8 ||
        GPIO_Pin == GPIO_PIN_9 ||
        GPIO_Pin == GPIO_PIN_15)
    {
        MMR92C04::drdy_interrupt_response();
    }
}