#include "FreeRTOS.h"
#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/transport.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/simulation/eeprom.hpp"
#include "common/simulation/spi.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"
#include "pipettes/core/tasks.hpp"
#include "task.h"

static auto can_bus_1 = sim_canbus::SimCANBus{can_transport::create()};

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

static sim_spi::SimTMC2130Spi spi_comms{};

static sim_motor_hardware_iface::SimMotorHardwareIface plunger_hw{};

static motor_handler::MotorInterruptHandler plunger_interrupt(
    motor_queue, pipettes_tasks::get_queues(), plunger_hw);

static motor_interrupt_driver::MotorInterruptDriver sim_interrupt(
    motor_queue, plunger_interrupt);

// microstepping is currently set to 32 μsteps.
static motor_driver_config::RegisterConfig MotorDriverConfigurations{
    .gconf = 0x04,
    .ihold_irun = 0x70202,
    .chopconf = 0x30101D5,
    .thigh = 0xFFFFF,
    .coolconf = 0x60000};

static auto i2c_comms = sim_i2c::SimEEProm{};

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

int main() {
    pipettes_tasks::start_tasks(can_bus_1, pipette_motor.motion_controller,
                                pipette_motor.driver, i2c_comms);
    vTaskStartScheduler();
}