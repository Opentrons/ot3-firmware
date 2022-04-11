#include "gripper/core/interfaces.hpp"
#include "motor-control/firmware/brushed_motor/brushed_motor_hardware.hpp"
#include "motor-control/firmware/brushed_motor/driver_hardware.hpp"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#pragma GCC diagnostic pop

/**
 * Brushed motor pin configuration.
 */
struct motor_hardware::BrushedHardwareConfig brushed_motor_pins {
    .pwm_1 =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .tim = &htim1,
            .channel = TIM_CHANNEL_1},
    .pwm_2 =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .tim = &htim3,
            .channel = TIM_CHANNEL_1},
    .enable =
        {  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_11,
            .active_setting = GPIO_PIN_SET},
    .limit_switch = {  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOC,
        .pin = GPIO_PIN_2,
        .active_setting = GPIO_PIN_SET},
};

/**
 * Brushed motor dac configuration.
 */
struct brushed_motor_driver::DacConfig dac_config {
    .dac_handle = &hdac1, .channel = DAC_CHANNEL_1,
    .data_algn = DAC_ALIGN_12B_R,
};
/**
 * The brushed motor hardware interface.
 */
static motor_hardware::BrushedMotorHardware brushed_motor_hardware_iface(
    brushed_motor_pins, nullptr);

/**
 * The brushed motor driver hardware interface.
 */
static brushed_motor_driver::BrushedMotorDriver brushed_motor_driver_iface(
    dac_config, brushed_motor_driver::DriverConfig{.vref = 0.5}, update_pwm);

void grip_motor_iface::initialize() {
    // Initialize DAC
    initialize_dac();
}

auto grip_motor_iface::get_motor_hardware_iface()
    -> motor_hardware::BrushedMotorHardwareIface& {
    return brushed_motor_hardware_iface;
}

auto grip_motor_iface::get_motor_driver_hardware_iface()
    -> brushed_motor_driver::BrushedMotorDriverIface& {
    return brushed_motor_driver_iface;
}
