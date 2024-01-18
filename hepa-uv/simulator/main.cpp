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
#include "hepa-uv/core/tasks.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
#include "task.h"

namespace po = boost::program_options;

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

/**
 * The CAN bus.
 */

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
        po::options_description("simulator for the Flex HEPA/UV module.");
    auto envdesc = po::options_description("");
    cmdlinedesc.add_options()("help,h", "Show this help message.");
    auto can_arg_xform = can::sim::transport::add_options(cmdlinedesc, envdesc);
    auto state_mgr_arg_xform = state_manager::add_options(cmdlinedesc, envdesc);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cmdlinedesc), vm);
    if (vm.count("help")) {
        std::cout << cmdlinedesc << std::endl;
        std::exit(0);
    }
    po::store(po::parse_environment(
                  envdesc,
                  [can_arg_xform, state_mgr_arg_xform](
                      const std::string& input_val) -> std::string {
                      auto can_xformed = can_arg_xform(input_val);
                      if (can_xformed != "") {
                          return can_xformed;
                      }
                      auto state_mgr_xformed = state_mgr_arg_xform(input_val);
                      return state_mgr_xformed;
                  }),
              vm);
    po::notify(vm);
    return vm;
}

static auto gpio_drive_pins = gpio_drive_hardware::GpioDrivePins{};

int main(int argc, char** argv) {
    // TODO: (ba, 2024-01-18): Fix the simulator later on
    signal(SIGINT, signal_handler);

    LOG_INIT("HEPA/UV", []() -> const char* {
        return pcTaskGetName(xTaskGetCurrentTaskHandle());
    });
    auto options = handle_options(argc, argv);

    state_manager_connection = state_manager::create<
        freertos_synchronization::FreeRTOSCriticalSection>(options);
    state_manager_task_control.start(5, "state mgr task",
                                     &state_manager_connection);

    static auto canbus =
        can::sim::bus::SimCANBus(can::sim::transport::create(options));
    hepauv_tasks::start_tasks(canbus, gpio_drive_pins);

    vTaskStartScheduler();
}
