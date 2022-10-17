#include <signal.h>

#include <iostream>
#include <memory>
#include <string>

#include "FreeRTOS.h"
#include "boost/program_options.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "eeprom/simulation/eeprom.hpp"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"
#include "i2c/simulation/i2c_sim.hpp"
#include "sensors/simulation/fdc1004.hpp"
#include "sensors/simulation/mock_hardware.hpp"
#include "task.h"

namespace po = boost::program_options;

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

/**
 * The CAN bus.
 */

static auto capsensor = fdc1004_simulator::FDC1004{};
static auto sensor_map =
    i2c::hardware::SimI2C::DeviceMap{{capsensor.get_address(), capsensor}};
static auto i2c2 = i2c::hardware::SimI2C{sensor_map};

static test_mocks::MockSensorHardware fake_sensor_hw{};

auto handle_options(int argc, char** argv) -> po::variables_map {
    auto cmdlinedesc =
        po::options_description("simulator for the OT-3 gripper");
    auto envdesc = po::options_description("");
    cmdlinedesc.add_options()("help,h", "Show this help message.");
    auto can_arg_xform = can::sim::transport::add_options(cmdlinedesc, envdesc);
    auto eeprom_arg_xform =
        eeprom::simulator::EEProm::add_options(cmdlinedesc, envdesc);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cmdlinedesc), vm);
    if (vm.count("help")) {
        std::cout << cmdlinedesc << std::endl;
        std::exit(0);
    }
    po::store(po::parse_environment(
                  envdesc,
                  [can_arg_xform, eeprom_arg_xform](
                      const std::string& input_val) -> std::string {
                      auto can_xformed = can_arg_xform(input_val);
                      if (can_xformed != "") {
                          return can_xformed;
                      }
                      auto eeprom_xformed = eeprom_arg_xform(input_val);
                      return eeprom_xformed;
                  }),
              vm);
    po::notify(vm);
    return vm;
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);

    LOG_INIT("GRIPPER", []() -> const char* {
        return pcTaskGetName(xTaskGetCurrentTaskHandle());
    });
    const uint32_t TEMPORARY_SERIAL = 0x103321;
    auto options = handle_options(argc, argv);
    auto sim_eeprom =
        std::make_shared<eeprom::simulator::EEProm>(options, TEMPORARY_SERIAL);
    auto i2c_device_map = i2c::hardware::SimI2C::DeviceMap{
        {sim_eeprom->get_address(), *sim_eeprom}};
    auto i2c3 = std::make_shared<i2c::hardware::SimI2C>(i2c_device_map);
    static auto canbus = can::sim::bus::SimCANBus(
        can::sim::transport::create(options));
    z_motor_iface::initialize();
    grip_motor_iface::initialize();
    gripper_tasks::start_tasks(canbus, z_motor_iface::get_z_motor(),
                               grip_motor_iface::get_grip_motor(),
                               z_motor_iface::get_spi(),
                               z_motor_iface::get_tmc2130_driver_configs(),
                               i2c2, *i2c3, fake_sensor_hw, *sim_eeprom);

    vTaskStartScheduler();
}
