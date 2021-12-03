#include "gantry/core/interfaces.hpp"

#include "can/simlib/sim_canbus.hpp"
#include "common/simulation/sim_motor_hardware_iface.hpp"
#ifdef __linux__
#include "can/simlib/socketcan_transport.hpp"
#else
#include "can/simlib/stdio_transport.hpp"
#endif

#include "common/core/freertos_synchronization.hpp"
#include "common/simulation/spi.hpp"

static auto constexpr ChannelEnvironmentVariableName = "CAN_CHANNEL";
static auto constexpr DefaultChannel = "vcan0";

#ifdef __linux__
static auto transport = socketcan::SocketCanTransport<
    freertos_synchronization::FreeRTOSCriticalSection>{};
#else
static auto transport = stdio_can::StdioTransport<
    freertos_synchronization::FreeRTOSCriticalSection>{};
#endif

static auto canbus = sim_canbus::SimCANBus(transport);

static auto spibus = sim_spi::SimTMC2130Spi();

static auto motor_interface = sim_motor_hardware_iface::SimMotorHardwareIface();

auto interfaces::get_can_bus() -> can_bus::CanBus& { return canbus; }

auto interfaces::get_spi() -> spi::TMC2130Spi& { return spibus; }

auto interfaces::get_motor_hardware_iface() -> motor_hardware::MotorHardwareIface& {return motor_interface;}