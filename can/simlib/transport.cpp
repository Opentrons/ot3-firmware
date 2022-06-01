#include "can/simlib/transport.hpp"

#include "common/core/freertos_synchronization.hpp"
#ifdef USE_SOCKETCAN
#include "can/simlib/socketcan_transport.hpp"
#else
#include "can/simlib/socket_transport.hpp"
#endif

/**
 * Create simulating bus transport
 * @return pointer to bus transport
 */
auto can::sim::transport::create()
    -> std::shared_ptr<can::sim::transport::BusTransportBase> {
#ifdef USE_SOCKETCAN
    auto constexpr ChannelEnvironmentVariableName = "CAN_CHANNEL";
    auto constexpr DefaultChannel = "vcan0";

    const char* env_channel_val = std::getenv(ChannelEnvironmentVariableName);
    auto channel = env_channel_val ? env_channel_val : DefaultChannel;
    auto transport = std::make_shared<can::sim::transport::socketcan::SocketCanTransport<
        freertos_synchronization::FreeRTOSCriticalSection>>(channel);
#else
    auto constexpr ServerHostEnvironmentVariableName = "CAN_SERVER_HOST";
    auto constexpr DefaultServerHost = "localhost";
    auto constexpr PortEnvironmentVariableName = "CAN_PORT";
    auto constexpr DefaultPort = 9898;

    const char* env_server_host_val =
        std::getenv(ServerHostEnvironmentVariableName);
    auto host = env_server_host_val ? env_server_host_val : DefaultServerHost;
    const char* env_port_val = std::getenv(PortEnvironmentVariableName);
    auto port =
        env_port_val ? std::strtoul(env_port_val, nullptr, 10) : DefaultPort;

    auto transport = std::make_shared<can::sim::transport::socket::SocketTransport<
        freertos_synchronization::FreeRTOSCriticalSection>>(host, port);

#endif
    return transport;
}