#pragma once

#include "can/core/can_bus.hpp"
#include "motor-control/core/spi.hpp"

namespace interfaces {

/**
 *
 * @return
 */
auto get_can_bus() -> can_bus::CanBus &;

/**
 *
 * @return
 */
auto get_spi() -> spi::TMC2130Spi &;

}  // namespace interfaces