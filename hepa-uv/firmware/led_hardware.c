#include "hepa-uv/firmware/led_hardware.h"
#include "timer_hardware.h"

void button_led_hw_update_pwm(uint32_t duty_cycle, LED_TYPE led, PUSH_BUTTON_TYPE button) {

    if (button == HEPA_BUTTON) {
        switch(led) {
            case RED_LED:
                htim1.Instance->CCR2 = duty_cycle;
                break;
            case GREEN_LED:
                htim8.Instance->CCR1 = duty_cycle;
                break;
            case BLUE_LED:
                htim16.Instance->CCR1 = duty_cycle;
                break;
            case WHITE_LED:
                htim1.Instance->CCR3 = duty_cycle;
                break;
            default:
                break;
        }
    } else if (button == UV_BUTTON) {
        switch(led) {
            case RED_LED:
                htim8.Instance->CCR3 = duty_cycle;
                break;
            case GREEN_LED:
                htim8.Instance->CCR2 = duty_cycle;
                break;
            case BLUE_LED:
                htim1.Instance->CCR4 = duty_cycle;
                break;
            case WHITE_LED:
                htim20.Instance->CCR1=duty_cycle;
                break;
            default:
                break;
        }
    }
}

void set_button_led_pwm(PUSH_BUTTON_TYPE button, uint32_t red, uint32_t green, uint32_t blue, uint32_t white) {
    button_led_hw_update_pwm(red, RED_LED, button);
    button_led_hw_update_pwm(green, GREEN_LED, button);
    button_led_hw_update_pwm(blue, BLUE_LED, button);
    button_led_hw_update_pwm(white, WHITE_LED, button);
}
