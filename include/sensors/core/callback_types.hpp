#pragma once

#include <functional>

namespace sensor_callbacks {
/*
 * Generic types for callbacks used in sensor types.
 *
 * SingleRegister Concept - accepts a single buffer to convert the given
 * data to a fixed point value.
 * MultiRegister Concept - accepts two buffers to convert the given data to
 * a fixed point value.
 *
 * SingleRegisterCallback - wrappers around the callback structs in
 * the sensor tasks. Mostly to work around the fact that we can't use
 * templates with `std::variant`.
 *
 * MultiRegisterCallback - wrappers around the callback structs in
 * the sensor tasks. Mostly to work around the fact that we can't use
 * templates with `std::variant`.
 *
 * TODO (lc 02-27-2022): We should think about how to genericise these
 * concepts to take in any number of buffers, but not required at the moment.
 */
// Max sized buffer for data we'll ever need is ~40 bits.
using MaxMessageBuffer = std::array<uint8_t, 5>;


template<typename Callback>
concept SingleRegister = requires(Callback cb, const MaxMessageBuffer &buffer) {
    {cb.handle_data(buffer)};
    {cb.send_to_can()};
};

template<typename Callback>
concept MultiRegister = requires(Callback cb, const MaxMessageBuffer &buffer) {
    {cb.handle_data(buffer, buffer)};
    {cb.send_to_can()};
};

template<SingleRegister scb>
struct SingleRegisterCallback {
    SingleRegisterCallback(scb& callback) : callback{callback} {}
    scb& callback;

    void operator()(const MaxMessageBuffer &buffer) {
        callback.handle_data(buffer);
    }

    void operator()() {
        callback.send_to_can();
    }
};

template<MultiRegister mcb>
struct MultiRegisterCallback {
    MultiRegisterCallback(mcb& callback) : callback{callback} {}
    mcb& callback;

    void operator()(const MaxMessageBuffer &buffer_a, const MaxMessageBuffer &buffer_b) {
        callback.handle_data(buffer_a, buffer_b);
    }

    void operator()() {
        callback.send_to_can();
    }
};

// Types representing the possible callback signatures
using SendToCanFunctionTypeDef = std::function<void()>;
using SingleBufferTypeDef = std::function<void(const MaxMessageBuffer&)>;
using MultiBufferTypeDef = std::function<void(const MaxMessageBuffer&, const MaxMessageBuffer&)>;

}  // namespace sensor_callbacks
