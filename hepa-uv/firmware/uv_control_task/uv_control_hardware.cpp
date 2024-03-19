#include "hepa-uv/firmware/uv_control_hardware.hpp"

#include "hepa-uv/firmware/uv_hardware.h"

using namespace uv_control_hardware;

uint32_t UVControlHardware::get_uv_light_voltage() {
    return get_uv_light_voltage_reading();
}