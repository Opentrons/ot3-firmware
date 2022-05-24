#include <signal.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "FreeRTOS.h"
#include "can/core/ids.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/transport.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/logging.h"
#include "eeprom/simulation/eeprom.hpp"
#include "i2c/simulation/i2c_sim.hpp"

#include "pipettes/simulator/interfaces.hpp"
#include "pipettes/core/central_tasks.hpp"
#include "pipettes/core/configs.hpp"
#include "pipettes/core/gear_motor_tasks.hpp"
#include "pipettes/core/linear_motor_tasks.hpp"
#include "pipettes/core/motor_configurations.hpp"
#include "pipettes/core/peripheral_tasks.hpp"
#include "pipettes/core/sensor_tasks.hpp"
#include "sensors/simulation/fdc1004.hpp"
#include "sensors/simulation/hardware.hpp"
#include "sensors/simulation/hdc2080.hpp"
#include "sensors/simulation/mmr920C04.hpp"
#include "spi/simulation/spi.hpp"
#include "task.h"

constexpr auto PIPETTE_TYPE = get_pipette_type();

static auto can_bus_1 = sim_canbus::SimCANBus{can_transport::create()};


static spi::hardware::SimSpiDeviceBase spi_comms{};

static auto motor_config = motor_configs::motor_configurations<PIPETTE_TYPE>();

static auto interrupt_queues = interfaces::get_interrupt_queues<PIPETTE_TYPE>();

static auto linear_motor_hardware =
    interfaces::linear_motor::get_motor_hardware();
static auto plunger_interrupt = interfaces::linear_motor::get_interrupt(
    linear_motor_hardware, interrupt_queues.plunger_queue);
static auto plunger_interrupt_driver = interfaces::linear_motor::get_interrupt_driver(
    linear_motor_hardware, interrupt_queues.plunger_queue, plunger_interrupt);
static auto linear_motion_control =
    interfaces::linear_motor::get_motion_control(linear_motor_hardware,
                                                 interrupt_queues);

static auto gear_hardware =
    interfaces::gear_motor::get_motor_hardware(motor_config.hardware_pins);
static auto gear_interrupts =
    interfaces::gear_motor::get_interrupts(gear_hardware, interrupt_queues);
static auto gear_motion_control =
    interfaces::gear_motor::get_motion_control(gear_hardware, interrupt_queues);


static auto hdcsensor = hdc2080_simulator::HDC2080{};
static auto capsensor = fdc1004_simulator::FDC1004{};
static auto sim_eeprom = eeprom::simulator::EEProm{};
static auto pressuresensor = mmr920C04_simulator::MMR920C04{};
i2c::hardware::SimI2C::DeviceMap sensor_map_i2c1 = {
    {hdcsensor.get_address(), hdcsensor},
    {capsensor.get_address(), capsensor},
    {pressuresensor.get_address(), pressuresensor}};
i2c::hardware::SimI2C::DeviceMap sensor_map_i2c3 = {
    {sim_eeprom.get_address(), sim_eeprom}};

static auto i2c3_comms = i2c::hardware::SimI2C{sensor_map_i2c3};
static auto i2c1_comms = i2c::hardware::SimI2C{sensor_map_i2c1};

static sensors::hardware::SimulatedSensorHardware fake_sensor_hw{};

static auto node_from_env(const char* env) -> can_ids::NodeId {
    if (!env) {
        LOG("On left mount by default");
        return can_ids::NodeId::pipette_left;
    }
    if (strncmp(env, "left", strlen("left")) == 0) {
        LOG("On left mount from env var");
        return can_ids::NodeId::pipette_left;
    } else if (strncmp(env, "right", strlen("right")) == 0) {
        LOG("On right mount from env var");
        return can_ids::NodeId::pipette_right;
    } else {
        LOG("On left mount from invalid env var");
        return can_ids::NodeId::pipette_left;
    }
}

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

static const char* PipetteTypeString[] = {
    "SINGLE CHANNEL PIPETTE", "EIGHT CHANNEL PIPETTE",
    "NINETY SIX CHANNEL PIPETTE", "THREE EIGHTY FOUR CHANNEL PIPETTE"};


auto initialize_motor_tasks(
    can_ids::NodeId id,
    motor_configs::HighThroughputPipetteDriverHardware& conf,
    interfaces::gear_motor::GearMotionControl& gear_motion) {
    sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                              peripheral_tasks::get_i2c3_client(),
                              peripheral_tasks::get_i2c1_client(),
                              peripheral_tasks::get_i2c1_poller_client(),
                              fake_sensor_hw, id, sim_eeprom);

    linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, linear_motion_control,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id);

    // TODO Convert gear motor tasks
    gear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, gear_motion.right,
        peripheral_tasks::get_spi_client(), conf.right_gear_motor, id);
}

auto initialize_motor_tasks(
    can_ids::NodeId id,
    motor_configs::LowThroughputPipetteDriverHardware& conf,
    interfaces::gear_motor::UnavailableGearMotionControl&) {
    sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                              peripheral_tasks::get_i2c3_client(),
                              peripheral_tasks::get_i2c1_client(),
                              peripheral_tasks::get_i2c1_poller_client(),
                              fake_sensor_hw, id, sim_eeprom);
    linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, linear_motion_control,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id);
}

int main() {
    signal(SIGINT, signal_handler);

    LOG_INIT(PipetteTypeString[PIPETTE_TYPE], []() -> const char* {
        return pcTaskGetName(xTaskGetCurrentTaskHandle());
    });

    central_tasks::start_tasks(can_bus_1, node_from_env(std::getenv("MOUNT")));
    peripheral_tasks::start_tasks(i2c3_comms, i2c1_comms, spi_comms);
    initialize_motor_tasks(node_from_env(std::getenv("MOUNT")),
                           motor_config.driver_configs,
                           gear_motion_control);

    vTaskStartScheduler();
}
