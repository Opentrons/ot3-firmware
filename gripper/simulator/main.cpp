#include <signal.h>

#include "FreeRTOS.h"
#include "can/simlib/sim_canbus.hpp"
#include "eeprom/simulation/eeprom.hpp"
#include "eeprom/simulation/write_protect.hpp"
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
std::map<uint16_t, sensor_simulator::SensorType> sensor_map = {
    {sim_eeprom.ADDRESS, sim_eeprom}};
static auto i2c3 = i2c::hardware::SimI2C{sensor_map};

static auto eeprom_wp_pin = eeprom::sim_write_protect::SimWriteProtectPin{};

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
        z_motor_iface::get_tmc2130_driver_configs(), i2c3, eeprom_wp_pin);

    vTaskStartScheduler();
}
