#pragma once
#include <atomic>
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

auto debouce_update(bool new_state, std::atomic_bool& value,
                    std::atomic_bool& value_bounce) -> void;

};  // namespace gpio
