#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32l5xx.h"
#include "task.h"

// clang-format on

#include "can/firmware/hal_can_bus.hpp"
#include "common/firmware/clocking.h"
#include "pipettes/core/motion_controller_task.hpp"
#include "pipettes/core/motor_driver_task.hpp"
#include "pipettes/core/move_group_task.hpp"
#include "pipettes/core/move_status_reporter_task.hpp"
#include "pipettes/core/tasks.hpp"
#include "pipettes/firmware/can_task.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/motor_driver_config.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/firmware/motor_hardware.hpp"
#include "common/firmware/spi_comms.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#pragma GCC diagnostic pop

static auto can_bus_1 = hal_can_bus::HalCanBus(can_get_device_handle());

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move> motor_queue(
    "Motor Queue");

spi::SPI_interface SPI_intf = {
    .SPI_handle = &hspi2,
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .GPIO_handle = GPIOC,
    .pin = GPIO_PIN_6,
};
static spi::Spi spi_comms(SPI_intf);

struct motion_controller::HardwareConfig plunger_pins {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_3,
            .active_setting = GPIO_PIN_SET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_7,
            .active_setting = GPIO_PIN_SET},
    .enable = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOC,
        .pin = GPIO_PIN_8,
        .active_setting = GPIO_PIN_SET},
};

static motor_hardware::MotorHardware plunger_hw(plunger_pins, &htim7);
static motor_handler::MotorInterruptHandler plunger_interrupt(motor_queue,
                                                              pipettes_tasks::get_queues(),
                                                              plunger_hw);

// microstepping is currently set to 32 μsteps.
static motor_driver_config::RegisterConfig MotorDriverConfigurations{.gconf = 0x04,
                                                .ihold_irun = 0x70202,
                                                .chopconf = 0x30101D5,
                                                .thigh = 0xFFFFF,
                                                .coolconf = 0x60000};

/**
 * TODO: This motor class is only used in motor handler and should be
 * instantiated inside of the MotorHandler class. However, some refactors
 * should be made to avoid a pretty gross template signature.
 */

static motor_class::Motor pipette_motor{
    spi_comms,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 3.03},
        .steps_per_rev = 200,
        .microstep = 32},
    plunger_hw,
    motor_messages::MotionConstraints{.min_velocity = 1,
                      .max_velocity = 2,
                      .min_acceleration = 1,
                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue};



auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    MX_ICACHE_Init();

    auto& queues = pipettes_tasks::get_queues();
    auto& tasks = pipettes_tasks::get_tasks();

    auto can_writer = can_task::start_writer(can_bus_1);
    can_task::start_reader(can_bus_1);

    auto motion = pipettes_motion_controller_task::start_task(
        pipette_motor.motion_controller, queues);
    auto motor = pipettes_motor_driver_task::start_task(
        pipette_motor.driver, queues);
    auto move_group = pipettes_move_group_task::start_task(queues);
    auto move_status_reporter =
        pipettes_move_status_reporter_task::start_task(queues);

    tasks.can_writer = &can_writer;
    tasks.motion_controller = &motion;
    tasks.motor_driver = &motor;
    tasks.move_group = &move_group;
    tasks.move_status_reporter = &move_status_reporter;

    queues.motion_queue = &motion.get_queue();
    queues.motor_queue = &motor.get_queue();
    queues.move_group_queue = &move_group.get_queue();
    queues.set_queue(&can_writer.get_queue());
    queues.move_status_report_queue = &move_status_reporter.get_queue();

    vTaskStartScheduler();
}
