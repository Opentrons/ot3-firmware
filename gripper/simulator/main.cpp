#include <signal.h>

#include "FreeRTOS.h"
#include "can/simlib/sim_canbus.hpp"
#include "eeprom/simulation/eeprom.hpp"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"
#include "i2c/simulation/i2c_sim.hpp"
#include "sensors/simulation/fdc1004.hpp"
#include "sensors/simulation/hardware.hpp"
#include "task.h"

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

/**
 * The CAN bus.
 */
static auto canbus = can::sim::bus::SimCANBus(can::sim::transport::create());

static auto capsensor = fdc1004_simulator::FDC1004{};
static auto sim_eeprom = eeprom::simulator::EEProm{};
static auto sensor_map =
    i2c::hardware::SimI2C::DeviceMap{{capsensor.get_address(), capsensor}};
static auto i2c2 = i2c::hardware::SimI2C{sensor_map};
static auto i2c_device_map =
    i2c::hardware::SimI2C::DeviceMap{{sim_eeprom.get_address(), sim_eeprom}};
static auto i2c3 = i2c::hardware::SimI2C{i2c_device_map};

static sensors::hardware::SimulatedSensorHardware fake_sensor_hw{};

int main() {
    signal(SIGINT, signal_handler);

    LOG_INIT("GRIPPER", []() -> const char* {
        return pcTaskGetName(xTaskGetCurrentTaskHandle());
    });

    z_motor_iface::initialize();
    grip_motor_iface::initialize();
    gripper_tasks::start_tasks(canbus, z_motor_iface::get_z_motor(),
                               grip_motor_iface::get_grip_motor(),
                               z_motor_iface::get_spi(),
                               z_motor_iface::get_tmc2130_driver_configs(),
                               i2c2, i2c3, fake_sensor_hw, sim_eeprom);

    vTaskStartScheduler();
}
