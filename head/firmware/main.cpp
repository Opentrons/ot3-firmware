#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
#include "common/firmware/errors.h"
// clang-format on
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
#pragma GCC diagnostic pop
#include "can/firmware/hal_can_bus.hpp"
#include "common/firmware/clocking.h"
#include "common/firmware/spi_comms.hpp"
#include "head/core/tasks.hpp"
#include "head/firmware/adc_comms.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/motor_driver_config.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/firmware/motor_hardware.hpp"
#include "presence-sensing/core/presence_sensing_driver.hpp"

static auto can_bus_1 = hal_can_bus::HalCanBus(can_get_device_handle());

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_left("Motor Queue Left");

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_right("Motor Queue Right");

/**
 * @brief SPI MSP Initialization
 * This function configures SPI for the Z/A axis motors
 * @param hspi: SPI handle pointer
 * @retval None
 */

spi::SPI_interface SPI_intf2 = {
    .SPI_handle = &hspi2,
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .GPIO_handle = GPIOB,
    .pin = GPIO_PIN_12,
};
static spi::Spi spi_comms2(SPI_intf2);

spi::SPI_interface SPI_intf3 = {
    .SPI_handle = &hspi3,
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .GPIO_handle = GPIOA,
    .pin = GPIO_PIN_4,
};
static spi::Spi spi_comms3(SPI_intf3);

struct motor_hardware::HardwareConfig pin_configurations_left {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_1,
            .active_setting = GPIO_PIN_RESET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_0,
            .active_setting = GPIO_PIN_SET},
    .enable = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOC,
        .pin = GPIO_PIN_4,
        .active_setting = GPIO_PIN_SET},
};

struct motor_hardware::HardwareConfig pin_configurations_right {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_7,
            .active_setting = GPIO_PIN_RESET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_6,
            .active_setting = GPIO_PIN_SET},
    .enable = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOB,
        .pin = GPIO_PIN_11,
        .active_setting = GPIO_PIN_SET},
};

motor_driver_config::RegisterConfig MotorDriverConfigurations{
    .gconf = 0x04,
    .ihold_irun = 0x71002,
    .chopconf = 0x40101D5,
    .thigh = 0xFFFFF,
    .coolconf = 0x60000};

/**
 * TODO: This motor class is only used in motor handler and should be
 * instantiated inside of the MotorHandler class. However, some refactors
 * should be made to avoid a pretty gross template signature.
 */

static motor_hardware::MotorHardware motor_hardware_right(
    pin_configurations_right, &htim7);
static motor_handler::MotorInterruptHandler motor_interrupt_right(
    motor_queue_right, head_tasks::get_right_queues(), motor_hardware_right);

static motor_class::Motor motor_right{
    spi_comms2,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_hardware_right,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue_right};

static motor_hardware::MotorHardware motor_hardware_left(
    pin_configurations_left, &htim7);
static motor_handler::MotorInterruptHandler motor_interrupt_left(
    motor_queue_left, head_tasks::get_left_queues(), motor_hardware_left);

static motor_class::Motor motor_left{
    spi_comms3,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_hardware_left,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue_left};

extern "C" void motor_callback_glue() {
    motor_interrupt_left.run_interrupt();
    motor_interrupt_right.run_interrupt();
}
adc::ADC_interface ADC_intf2 = {

    .ADC_handle = &adc2};

adc::ADC_interface ADC_intf1 = {

    .ADC_handle = &adc1

};

static auto ADC_comms = adc::ADC(ADC_intf1, ADC_intf2);

static auto psd = presence_sensing_driver::PresenceSensingDriver{ADC_comms};

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();

    initialize_timer(motor_callback_glue);

    if (initialize_spi(&hspi2) != HAL_OK) {
        Error_Handler();
    }
    if (initialize_spi(&hspi3) != HAL_OK) {
        Error_Handler();
    }

    can_start();

    head_tasks::start_tasks(can_bus_1, motor_left.motion_controller,
                            motor_left.driver, motor_right.motion_controller,
                            motor_right.driver, psd);

    vTaskStartScheduler();
}
