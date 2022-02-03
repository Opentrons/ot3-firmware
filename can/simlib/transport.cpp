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
auto can_transport::create()
    -> std::shared_ptr<can_transport::BusTransportBase> {
#ifdef USE_SOCKETCAN
    auto constexpr ChannelEnvironmentVariableName = "CAN_CHANNEL";
    auto constexpr DefaultChannel = "vcan0";

    const char* env_channel_val = std::getenv(ChannelEnvironmentVariableName);
    auto channel = env_channel_val ? env_channel_val : DefaultChannel;
    auto transport = std::make_shared<socketcan_transport::SocketCanTransport<
        freertos_synchronization::FreeRTOSCriticalSection>>(channel);
#else
    auto constexpr ServerIpEnvironmentVariableName = "CAN_SERVER_IP";
    auto constexpr DefaultServerIp = "127.0.0.1";
    auto constexpr PortEnvironmentVariableName = "CAN_PORT";
    auto constexpr DefaultPort = 9898;

    const char* env_server_ip_val =
        std::getenv(ServerIpEnvironmentVariableName);
    auto ip = env_server_ip_val ? env_server_ip_val : DefaultServerIp;
    const char* env_port_val = std::getenv(PortEnvironmentVariableName);
    auto port =
        env_port_val ? std::strtoul(env_port_val, nullptr, 10) : DefaultPort;

    auto transport = std::make_shared<socket_transport::SocketTransport<
        freertos_synchronization::FreeRTOSCriticalSection>>(ip, port);

#endif
    return transport;
}