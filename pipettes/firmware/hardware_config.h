#pragma once
#include "pipettes/core/pipette_type.h"
#include "platform_specific_hal_conf.h"

typedef struct {
    void* port;
    uint16_t pin;
}PipetteHardwarePin;

typedef enum {
    pipette_hardware_device_LED_drive,
    pipette_hardware_device_sync_in,
    pipette_hardware_device_sync_out,
    pipette_hardware_device_data_ready_front,
    pipette_hardware_device_data_ready_rear
}PipetteHardwareDevice;

typedef enum {
    gpio_block_9_5,
    gpio_block_15_10,
    gpio_block_2,
    gpio_block_3,
    gpio_block_1,
}GPIOInterruptBlock;

uint16_t pipette_hardware_spi_pins(const PipetteType pipette_type, GPIO_TypeDef* which_handle);
uint16_t pipette_hardware_motor_driver_pins(const PipetteType pipette_type, GPIO_TypeDef* for_handle);
PipetteHardwarePin pipette_hardware_get_gpio(const PipetteType pipette_type, PipetteHardwareDevice device);
IRQn_Type get_interrupt_line(GPIOInterruptBlock gpio_pin_type);
