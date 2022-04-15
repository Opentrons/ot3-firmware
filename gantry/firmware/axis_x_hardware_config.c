#include "axis_hardware_config.h"

GantryHardwarePin gantry_hardware_get_gpio(GantryHardwareDevice device) {
    GantryHardwarePin pinout;
    switch(device) {
        case gantry_hardware_device_LED_drive:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_11;
            return pinout;
        case gantry_hardware_device_limit_switch:
            pinout.port = GPIOC;
            pinout.pin = GPIO_PIN_2;
            return pinout;
        case gantry_hardware_device_sync_in:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_7;
            return pinout;
        case gantry_hardware_device_sync_out:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_6;
            return pinout;

        default:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_11;
            return pinout;
    }
}
