#pragma once

#include <array>
#include <cstdint>

namespace spi {

namespace utils {

using std::size_t;
static constexpr std::size_t MAX_BUFFER_SIZE = 5;
using MaxMessageBuffer = std::array<uint8_t, MAX_BUFFER_SIZE>;

}  // namespace utils

}  // namespace spi