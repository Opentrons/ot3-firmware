#include <cstdlib>
#include <cstring>

#include "FreeRTOS.h"
#include "can/core/ids.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/transport.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/logging.h"
#include "common/simulation/i2c_sim.hpp"
#include "common/simulation/spi.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"
#include "pipettes/core/tasks.hpp"
#include "sensors/simulation/eeprom.hpp"
#include "sensors/simulation/hdc2080.hpp"
#include "sensors/simulation/sensors.hpp"
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

static auto hdcsensor = hdc2080_simulator::HDC2080{};
static auto eeprom = eeprom_simulator::EEProm{};
std::map<uint16_t, sensor_simulator::SensorType> sensor_map = {
    {hdcsensor.ADDRESS, hdcsensor}, {eeprom.ADDRESS, eeprom}};

static auto i2c_comms = sim_i2c::SimI2C{sensor_map};

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

static auto node_from_env(const char* env) -> can_ids::NodeId {
    if (!env) {
        LOG("On left mount by default\n");
        return can_ids::NodeId::pipette_left;
    }
    if (strncmp(env, "left", strlen("left")) == 0) {
        LOG("On left mount from env var\n");
        return can_ids::NodeId::pipette_left;
    } else if (strncmp(env, "right", strlen("right")) == 0) {
        LOG("On right mount from env var\n");
        return can_ids::NodeId::pipette_right;
    } else {
        LOG("On left mount from invalid env var\n");
        return can_ids::NodeId::pipette_left;
    }
}

int main() {
    pipettes_tasks::start_tasks(can_bus_1, pipette_motor.motion_controller,
                                pipette_motor.driver, i2c_comms,
                                node_from_env(std::getenv("MOUNT")));
    vTaskStartScheduler();
}
