#include "gantry/core/interfaces.hpp"

#include "can/simlib/sim_canbus.hpp"
#include "common/simulation/sim_motor_hardware_iface.hpp"
#include "gantry/core/utils.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"

#ifdef USE_SOCKETCAN
#include "can/simlib/socketcan_transport.hpp"
#else
#include "can/simlib/socket_transport.hpp"
#endif

#include "common/core/freertos_synchronization.hpp"
#include "common/simulation/spi.hpp"

#ifdef USE_SOCKETCAN
static auto constexpr ChannelEnvironmentVariableName = "CAN_CHANNEL";
static auto constexpr DefaultChannel = "vcan0";

static auto transport = socketcan_transport::SocketCanTransport<
    freertos_synchronization::FreeRTOSCriticalSection>{};
#else
static auto constexpr ServerIpEnvironmentVariableName = "CAN_SERVER_IP";
static auto constexpr DefaultServerIp = "127.0.0.1";
static auto constexpr PortEnvironmentVariableName = "CAN_PORT";
static auto constexpr DefaultPort = 9898;

static auto transport = socket_transport::SocketTransport<
    freertos_synchronization::FreeRTOSCriticalSection>{};
#endif

/**
 * The CAN bus.
 */
static auto canbus = sim_canbus::SimCANBus(transport);

/**
 * The SPI bus.
 */
static auto spi_comms = sim_spi::SimTMC2130Spi();

/**
 * The motor interface.
 */
static auto motor_interface = sim_motor_hardware_iface::SimMotorHardwareIface();

/**
 * The pending move queue
 */
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");
/**
 * The completed move queue.
 */
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Ack>
    complete_queue("Complete Queue");

/**
 * The motor struct.
 */
static motor_class::Motor motor{
    spi_comms,
    lms::LinearMotionSystemConfig<lms::BeltConfig>{
        .mech_config =
            lms::BeltConfig{.belt_pitch = 2, .pulley_tooth_count = 10},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_interface,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    utils::register_config(),
    motor_queue,
    complete_queue};

/**
 * Handler of motor interrupts.
 */
static motor_handler::MotorInterruptHandler motor_interrupt(motor_queue,
                                                            complete_queue,
                                                            motor_interface);

void interfaces::initialize() {
#ifdef USE_SOCKETCAN
    const char* env_channel_val = std::getenv(ChannelEnvironmentVariableName);
    auto channel = env_channel_val ? env_channel_val : DefaultChannel;
    transport.open(channel);
#else
    const char* env_server_ip_val =
        std::getenv(ServerIpEnvironmentVariableName);
    auto ip = env_server_ip_val ? env_server_ip_val : DefaultServerIp;
    const char* env_port_val = std::getenv(PortEnvironmentVariableName);
    auto port =
        env_port_val ? std::strtoul(env_port_val, nullptr, 10) : DefaultPort;
    transport.open(ip, port);
#endif
}

auto interfaces::get_can_bus() -> can_bus::CanBus& { return canbus; }

auto interfaces::get_spi() -> spi::TMC2130Spi& { return spi_comms; }

auto interfaces::get_motor_hardware_iface()
    -> motor_hardware::MotorHardwareIface& {
    return motor_interface;
}

auto interfaces::get_motor() -> motor_class::Motor<lms::BeltConfig>& {
    return motor;
}