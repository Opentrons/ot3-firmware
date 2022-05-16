#include <signal.h>

#include "FreeRTOS.h"
#include "can/simlib/sim_canbus.hpp"
#include "eeprom/simulation/eeprom.hpp"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"
#include "i2c/simulation/i2c_sim.hpp"
#include "task.h"

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

/**
 * The CAN bus.
 */
static auto canbus = sim_canbus::SimCANBus(can_transport::create());

static auto sim_eeprom = eeprom::simulator::EEProm{};
static auto i2c_device_map =
    i2c::hardware::SimI2C::DeviceMap{{sim_eeprom.get_address(), sim_eeprom}};
static auto i2c3 = i2c::hardware::SimI2C{i2c_device_map};

int main() {
    signal(SIGINT, signal_handler);

    LOG_INIT("GRIPPER", []() -> const char* {
        return pcTaskGetName(xTaskGetCurrentTaskHandle());
    });

    z_motor_iface::initialize();
    grip_motor_iface::initialize();
    gripper_tasks::start_tasks(
        canbus, z_motor_iface::get_z_motor(),
        grip_motor_iface::get_grip_motor(), z_motor_iface::get_spi(),
        z_motor_iface::get_tmc2130_driver_configs(), i2c3, sim_eeprom);

    vTaskStartScheduler();
}
