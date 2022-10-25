#include "common/simulation/state_manager.hpp"

namespace po = boost::program_options;

auto state_manager::add_options(
    boost::program_options::options_description& cmdline_desc,
    boost::program_options::options_description& env_desc)
    -> std::function<std::string(std::string)> {
    cmdline_desc.add_options()(
        "state-mgr-host", po::value<std::string>()->default_value("localhost"),
        "state manager host to connect to. May be specified in an environment "
        "variable called STATE_MANAGER_HOST.")(
        "state-mgr-port", po::value<std::uint16_t>()->default_value(9999),
        "port for the state manager. May be specified in an environment "
        "variable called STATE_MANAGER_PORT.");
    env_desc.add_options()(
        "state-mgr-host", po::value<std::string>()->default_value("localhost"))(
        "state-mgr-port", po::value<std::uint16_t>()->default_value(9999));
    return [](std::string input_val) -> std::string {
        if (input_val == "STATE_MANAGER_HOST") {
            return "state-mgr-host";
        } else if (input_val == "STATE_MANAGER_PORT") {
            return "state-mgr-port";
        }
        return "";
    };
}