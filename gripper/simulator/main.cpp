#include <signal.h>

#include <iostream>
#include <memory>
#include <string>

#include "FreeRTOS.h"
#include "boost/program_options.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "common/core/freertos_synchronization.hpp"
#include "common/core/freertos_task.hpp"
#include "common/simulation/state_manager.hpp"
#include "eeprom/simulation/eeprom.hpp"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"
#include "gripper/simulation/sim_interfaces.hpp"
#include "i2c/simulation/i2c_sim.hpp"
#include "sensors/simulation/fdc1004.hpp"
#include "sensors/simulation/mock_hardware.hpp"
#include "task.h"

namespace po = boost::program_options;

static z_motor_iface::diag0_handler call_diag0_handler = nullptr;

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

static sim_mocks::MockSensorHardware fake_sensor_hw{};

static std::shared_ptr<state_manager::StateManagerConnection<
    freertos_synchronization::FreeRTOSCriticalSection>>
    state_manager_connection;

static auto state_manager_task = state_manager::StateManagerTask<
    freertos_synchronization::FreeRTOSCriticalSection>{};

static auto state_manager_task_control =
    freertos_task::FreeRTOSTask<512, decltype(state_manager_task)>{
        state_manager_task};

auto handle_options(int argc, char** argv) -> po::variables_map {
    auto cmdlinedesc =
        po::options_description("simulator for the OT-3 gripper");
    auto envdesc = po::options_description("");
    cmdlinedesc.add_options()("help,h", "Show this help message.");
    auto can_arg_xform = can::sim::transport::add_options(cmdlinedesc, envdesc);
    auto eeprom_arg_xform =
        eeprom::simulator::EEProm::add_options(cmdlinedesc, envdesc);
    auto state_mgr_arg_xform = state_manager::add_options(cmdlinedesc, envdesc);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cmdlinedesc), vm);
    if (vm.count("help")) {
        std::cout << cmdlinedesc << std::endl;
        std::exit(0);
    }
    po::store(po::parse_environment(
                  envdesc,
                  [can_arg_xform, eeprom_arg_xform, state_mgr_arg_xform](
                      const std::string& input_val) -> std::string {
                      auto can_xformed = can_arg_xform(input_val);
                      if (can_xformed != "") {
                          return can_xformed;
                      }
                      auto eeprom_xformed = eeprom_arg_xform(input_val);
                      if (eeprom_xformed != "") {
                          return eeprom_xformed;
                      }
                      auto state_mgr_xformed = state_mgr_arg_xform(input_val);
                      return state_mgr_xformed;
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

    state_manager_connection = state_manager::create<
        freertos_synchronization::FreeRTOSCriticalSection>(options);
    state_manager_task_control.start(5, "state mgr task",
                                     &state_manager_connection);

    z_motor_iface::get_z_motor_interface().provide_state_manager(
        state_manager_connection);
    z_motor_iface::get_brushed_motor_interface().provide_state_manager(
        state_manager_connection);
    fake_sensor_hw.provide_state_manager(state_manager_connection);

    auto sim_eeprom =
        std::make_shared<eeprom::simulator::EEProm>(options, TEMPORARY_SERIAL);
    auto i2c_device_map = i2c::hardware::SimI2C::DeviceMap{
        {sim_eeprom->get_address(), *sim_eeprom}};
    auto i2c3 = std::make_shared<i2c::hardware::SimI2C>(i2c_device_map);
    static auto canbus =
        can::sim::bus::SimCANBus(can::sim::transport::create(options));
    z_motor_iface::initialize(&call_diag0_handler);
    grip_motor_iface::initialize();
    call_diag0_handler = gripper_tasks::start_tasks(
        canbus, z_motor_iface::get_z_motor(),
        grip_motor_iface::get_grip_motor(), z_motor_iface::get_spi(),
        z_motor_iface::get_tmc2130_driver_configs(), i2c2, *i2c3,
        fake_sensor_hw, *sim_eeprom, z_motor_iface::get_z_motor_hardware_task(),
        grip_motor_iface::get_grip_motor_hardware_task());

    vTaskStartScheduler();
}
