#pragma once
#include <cstdint>

namespace gpio {

struct PinConfig {
    void* port;
    uint16_t pin;
    uint8_t active_setting;
};

auto set(const PinConfig& pc) -> void;
auto reset(const PinConfig& pc) -> void;
auto is_set(const PinConfig& pc) -> bool;
auto read_pin(const PinConfig& pc) -> uint8_t;

};  // namespace gpio
