#pragma once
#include "pipettes/core/pipette_type.h"
#include "platform_specific_hal_conf.h"

typedef struct {
    void* port;
    uint16_t pin;
}PipetteHardwarePin;

typedef enum {
    pipette_hardware_device_limit_switch,
    pipette_hardware_device_LED_drive,
    pipette_hardware_device_sync_in,
    pipette_hardware_device_sync_out
}PipetteHardwareDevice;

uint16_t pipette_hardware_spi_pins(PipetteType pipette_type, GPIO_TypeDef* which_handle);
PipetteHardwarePin pipette_hardware_get_gpio(PipetteType pipette_type, PipetteHardwareDevice device);
