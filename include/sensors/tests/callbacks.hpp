#pragma once

#include "sensors/core/callback_types.hpp"

namespace mock_callbacks {
struct EmptyCallback {
    void operator()(const sensor_callbacks::MaxMessageBuffer &buffer) {}
    void operator()() {}
};

struct UpdateCallback {
  public:
    uint16_t update_value;

    UpdateCallback() : update_value(0) {}
    void handle_data(const sensor_callbacks::MaxMessageBuffer &buffer) {
        update_value += buffer[3];
    }

    void send_to_can() {}

    void reset() { update_value = 0; }
};

struct MultiUpdateCallback {
  public:
    uint8_t register_a_value;
    uint8_t register_b_value;

    MultiUpdateCallback() : register_a_value(0), register_b_value(0) {}
    void handle_data(const sensor_callbacks::MaxMessageBuffer &buffer_a,
                     const sensor_callbacks::MaxMessageBuffer &buffer_b) {
        register_a_value += buffer_a[3];
        register_b_value += buffer_b[3];
    }

    void send_to_can() {}

    void reset() {
        register_a_value = 0;
        register_b_value = 0;
    }
};

}  // namespace mock_callbacks