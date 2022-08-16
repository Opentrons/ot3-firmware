#include <signal.h>


#include <cstring>
#include <string>
#include <iostream>
#include <memory>

#include "boost/program_options.hpp"

#include "FreeRTOS.h"
#include "bootloader/core/message_handler.h"
#include "bootloader/core/node_id.h"
#include "can/core/ids.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "common/core/logging.h"
#include "task.h"

namespace po = boost::program_options;

static CANNodeId g_node_id = can_nodeid_pipette_left_bootloader;

/** The simulator's bootloader */
CANNodeId get_node_id(void) { return g_node_id; }

/**
 * Handle a new can message
 * @param cb_data callback data
 * @param identifier arbitration id
 * @param data data
 * @param length length of data
 */
void on_can_message(void* ctx, uint32_t identifier, uint8_t* data, uint8_t length) {
    Message message;
    Message response;
    auto* canbus = static_cast<can::sim::bus::SimCANBus*>(ctx);
    message.arbitration_id.id = identifier;
    message.size = length;
    std::memcpy(
        message.data, data,
        std::min(static_cast<std::size_t>(length), sizeof(message.data)));

    auto handle_message_return = handle_message(&message, &response);
    switch (handle_message_return) {
        case handle_message_ok:
            LOG("Message ok. No response");
            break;
        case handle_message_has_response:
            LOG("Message ok. Has response");
            canbus->send(response.arbitration_id.id, response.data,
                        static_cast<CanFDMessageLength>(response.size));
            break;
        case handle_message_error:
            LOG("Message error.");
            break;
        default:
            LOG("Unknown return.");
            break;
    }
}

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

auto node_from_arg(std::string const& val) -> CANNodeId{
    if (val == "pipette_left") {
        return can_nodeid_pipette_left_bootloader;
    } else if (val == "pipette_right") {
        return can_nodeid_pipette_right_bootloader;
    } else if (val == "gantry_x") {
        return can_nodeid_gantry_x_bootloader;
    } else if (val == "gantry_y") {
        return can_nodeid_gantry_y_bootloader;
    } else if (val == "head") {
        return can_nodeid_head_bootloader;
    } else if (val == "gripper") {
        return can_nodeid_gripper_bootloader;
    } else {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }
}

// this function is automatically called by program options to validate inputs;
// if it throws, that's a validation failure.
void validate(boost::any& v,
              std::vector<std::string> const& values,
              CANNodeId*,
              int) {
    po::validators::check_first_occurrence(v);
    auto const& string_val = po::validators::get_single_string(values);
    // this throws appropriately if the arg is wrong
    v = boost::any(node_from_arg(string_val));
}

auto handle_options(int argc, char** argv) -> po::variables_map {
    auto cmdlinedesc = po::options_description("simulator for OT-3 pipettes");
    auto envdesc = po::options_description("");
    cmdlinedesc.add_options()("help,h", "Show this help message.");
    cmdlinedesc.add_options()(
        "node,n", po::value<CANNodeId>()->default_value(can_nodeid_pipette_left_bootloader),
        "Which node id to use. Maybe  May be specified in an "
        "environment variable called NODE_ID. Accepted values: pipette_left, "
        "pipette_right, gantry_x, gantry_y, head, gripper");
    envdesc.add_options()("NODE_ID",
                          po::value<CANNodeId>()->default_value(can_nodeid_pipette_left_bootloader));
    auto can_arg_xform = can::sim::transport::add_options(cmdlinedesc, envdesc);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cmdlinedesc), vm);
    if (vm.count("help")) {
        std::cout << cmdlinedesc << std::endl;
        std::exit(0);
    }
    po::store(po::parse_environment(
                  envdesc,
                  [can_arg_xform](
                      const std::string& input_val) -> std::string {
                      if (input_val == "NODE_ID") {
                          return "node";
                      };
                      return can_arg_xform(input_val);
                  }),
              vm);
    po::notify(vm);
    return vm;
}


int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    LOG_INIT("BOOTLOADER", []() -> const char* {
        return pcTaskGetName(xTaskGetCurrentTaskHandle());
    });

    auto options = handle_options(argc, argv);
    auto canbus = std::make_shared<can::sim::bus::SimCANBus>(can::sim::transport::create(options));
    g_node_id = options["node"].as<CANNodeId>();
    LOG("Running bootloader for node id %d", g_node_id);
    canbus->setup_node_id_filter(static_cast<can::ids::NodeId>(get_node_id()));
    canbus->set_incoming_message_callback(canbus.get(), on_can_message);

    vTaskStartScheduler();
}
