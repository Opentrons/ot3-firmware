#include "gantry/core/interfaces.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/socket_can.hpp"
#include "common/simulation/spi.hpp"
#include "motor-control/core/motion_controller.hpp"
#include "common/core/freertos_synchronization.hpp"

static auto constexpr ChannelEnvironmentVariableName = "CAN_CHANNEL";
static auto constexpr DefaultChannel = "vcan0";

static auto transport = socket_can::SocketCanTransport<
    freertos_synchronization::FreeRTOSCriticalSection>{};

static auto canbus = sim_canbus::SimCANBus(transport);

static auto spibus = sim_spi::SimTMC2130Spi();

static auto hardware_config = motion_controller::HardwareConfig{};


auto interfaces::get_can_bus() -> can_bus::CanBus & {
    return canbus;
}

auto interfaces::get_hardware_config() -> motion_controller::HardwareConfig{
    return hardware_config;
}

auto interfaces::get_spi() -> spi::TMC2130Spi & {
    return spibus;
}