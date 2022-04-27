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
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/core/stepper_motor/tmc2130_driver.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"
#include "pipettes/core/configs.hpp"
#include "pipettes/core/interfaces.hpp"
#include "pipettes/core/tasks.hpp"
#include "sensors/simulation/fdc1004.hpp"
#include "sensors/simulation/hardware.hpp"
#include "sensors/simulation/hdc2080.hpp"
#include "sensors/simulation/mmr920C04.hpp"
#include "sensors/simulation/sensors.hpp"
#include "spi/simulation/spi.hpp"
#include "task.h"

static auto PIPETTE_TYPE = get_pipette_type();

static auto can_bus_1 = sim_canbus::SimCANBus{can_transport::create()};

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

static spi::hardware::SimSpiDeviceBase spi_comms{};

static sim_motor_hardware_iface::SimMotorHardwareIface plunger_hw{};

static motor_handler::MotorInterruptHandler plunger_interrupt(
    motor_queue, pipettes_tasks::get_queues(), plunger_hw);

static motor_interrupt_driver::MotorInterruptDriver sim_interrupt(
    motor_queue, plunger_interrupt, plunger_hw);

static auto hdcsensor = hdc2080_simulator::HDC2080{};
static auto capsensor = fdc1004_simulator::FDC1004{};
static auto sim_eeprom = eeprom::simulator::EEProm{};
static auto pressuresensor = mmr920C04_simulator::MMR920C04{};
std::map<uint16_t, sensor_simulator::SensorType> sensor_map = {
    {hdcsensor.ADDRESS, hdcsensor},
    {sim_eeprom.ADDRESS, sim_eeprom},
    {capsensor.ADDRESS, capsensor},
    {pressuresensor.ADDRESS, pressuresensor}};

static auto i2c3_comms = i2c::hardware::SimI2C{sensor_map};
static auto i2c1_comms = i2c::hardware::SimI2C{sensor_map};

static sensors::hardware::SimulatedSensorHardware fake_sensor_hw{};

static motor_class::Motor pipette_motor{
    configs::linear_motion_sys_config_by_axis(PIPETTE_TYPE), plunger_hw,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue};

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

static auto driver_configs = interfaces::driver_config_by_axis(PIPETTE_TYPE);

int main() {
    signal(SIGINT, signal_handler);

    LOG_INIT(PipetteTypeString[PIPETTE_TYPE], []() -> const char* {
        return pcTaskGetName(xTaskGetCurrentTaskHandle());
    });

    pipettes_tasks::start_tasks(can_bus_1, pipette_motor.motion_controller,
                                i2c3_comms, i2c1_comms, fake_sensor_hw,
                                spi_comms, driver_configs,
                                node_from_env(std::getenv("MOUNT")));

    vTaskStartScheduler();
}
