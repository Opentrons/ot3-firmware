#pragma once

#include <functional>
#include "common/core/i2c.hpp"

namespace sensor_callbacks {
using namespace i2c;
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

// Types representing the possible callback signatures
using SendToCanFunctionTypeDef = std::function<void()>;
using SingleBufferTypeDef = std::function<void(const MaxMessageBuffer &)>;
using MultiBufferTypeDef =
    std::function<void(const MaxMessageBuffer &, const MaxMessageBuffer &)>;

}  // namespace sensor_callbacks
