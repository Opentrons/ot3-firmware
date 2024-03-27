#include "hepa-uv/firmware/uv_control_hardware.hpp"

#include "hepa-uv/firmware/uv_hardware.h"

using namespace uv_control_hardware;

auto UVControlHardware::get_uv_light_current() -> uint16_t {
    return get_uv_current_reading();
}