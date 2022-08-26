#include "can/simlib/transport.hpp"

#include <string>

#include "boost/program_options.hpp"
#include "common/core/freertos_synchronization.hpp"
#ifdef USE_SOCKETCAN
#include "can/simlib/socketcan_transport.hpp"
#else
#include "can/simlib/socket_transport.hpp"
#endif

namespace po = boost::program_options;

auto can::sim::transport::add_options(po::options_description& cmdline_desc,
                                      po::options_description& env_desc)
    -> std::function<std::string(std::string)> {
#ifdef USE_SOCKETCAN
    cmdline_desc.add_options()("can-channel,c",
                               po::value<std::string>()->default_value("vcan0"),
                               "can channel identifier. May be specified in an "
                               "environment variable called CAN_CHANNEL.");
    env_desc.add_options()("can-channel",
                           po::value<std::string>()->default_value("vcan0"));
    return [](std::string input_val) -> std::string {
        if (input_val == "CAN_CHANNEL") {
            return "can-channel";
        }
        return "";
    }
#else
    cmdline_desc.add_options()(
        "server-host,s", po::value<std::string>()->default_value("localhost"),
        "can server to connect to. May be specified in an environment variable "
        "called CAN_SERVER_HOST.")(
        "port,p", po::value<uint16_t>()->default_value(9898),
        "port for the can server. May be specified in an environment variable "
        "called CAN_PORT.");
    env_desc.add_options()(
        "server-host", po::value<std::string>()->default_value("localhost"))(
        "port", po::value<uint16_t>()->default_value(9898));
    return [](std::string input_val) -> std::string {
        if (input_val == "CAN_SERVER_HOST") {
            return "server-host";
        } else if (input_val == "CAN_PORT") {
            return "port";
        }
        return "";
    };
#endif
}

/**
 * Create simulating bus transport
 * @return pointer to bus transport
 */
auto can::sim::transport::create(
    const boost::program_options::variables_map& options)
    -> std::shared_ptr<can::sim::transport::BusTransportBase> {
#ifdef USE_SOCKETCAN
    auto channel = options["can-channel"].as<std::string>();
    auto transport =
        std::make_shared<can::sim::transport::socketcan::SocketCanTransport<
            freertos_synchronization::FreeRTOSCriticalSection>>(channel);
#else
    auto server = options["server-host"].as<std::string>();
    auto port = options["port"].as<std::uint16_t>();

    auto transport =
        std::make_shared<can::sim::transport::socket::SocketTransport<
            freertos_synchronization::FreeRTOSCriticalSection>>(server, port);

#endif
    return transport;
}
