#include "platform_specific_hal_conf.h"

typedef struct {
    void* port;
    uint16_t pin;
}GantryHardwarePin;

typedef enum {
    gantry_hardware_device_limit_switch,
    gantry_hardware_device_LED_drive,
    gantry_hardware_device_sync_in
}GantryHardwareDevice;

GantryHardwarePin gantry_hardware_get_gpio(GantryHardwareDevice device);