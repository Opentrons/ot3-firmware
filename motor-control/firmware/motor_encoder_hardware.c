//Initialize Header Files
#include "motor_encoder_hardware.h"

#include "FreeRTOS.h"
#include "task.h"

#include "platform_specific_hal_conf.h"

void motor_encoder_set_pin(void* port, uint16_t pin, uint8_t active_setting)
{
    if(!port) {return;}
    // Hall Function here
    HAL_GPIO_WritePin(port, pin, active_setting);
}

static uint8_t invert_gpio_value(uint8_t setting) {
    switch(setting) {
        case GPIO_PIN_SET:
            return GPIO_PIN_RESET;
        case GPIO_PIN_RESET:
            return GPIO_PIN_SET;
        default:
            return GPIO_PIN_SET;
    }
}

void motor_encoder_reset_pin(void* port, uint16_t pin, uint8_t active_setting) {
    if(!port) {return;}
    HAL_GPIO_WritePin(port, pin, invert_gpio_value(active_setting));
}

// Starts the encoder timer as interrupt
Void motor_encoder_start_timer(void* htim, uint32_t Channel)
{
    HAL_TIM_Encoder_Start_IT(htim, Channel);
}
// Stops the encoder timer interrupt
void motor_encoder_stop_timer(void* htim, uint32_t Channel)
{
    HAL_TIM_Encoder_Stop_IT(htim, Channel);
}

// obtain the count of the motor motor encoder
uint32_t motor_encoder_get_count(void *htim, uint32_t Channel)
{
    HAL_TIM_IC_CaptureCallback(*htim, uint32_t Channel);

}
