#pragma once

#include <array>
#include <cstdint>

namespace spi {

namespace utils {

using std::size_t;
static constexpr std::size_t MAX_BUFFER_SIZE = 5;
using MaxMessageBuffer = std::array<uint8_t, MAX_BUFFER_SIZE>;

struct ChipSelectInterface {
    uint32_t cs_pin;
    void* GPIO_handle;
};

// Bit positions to pack in an 8 bit response tag
enum class ResponseTag : size_t {
    IS_ERROR_RESPONSE = 0,
};

[[nodiscard]] constexpr auto byte_from_tag(ResponseTag tag) -> uint8_t {
    return (1 << static_cast<size_t>(tag));
}

template <std::ranges::range Tags>
[[nodiscard]] auto byte_from_tags(const Tags& tags) -> uint8_t {
    uint8_t result = 0;
    for (auto tag : tags) {
        result |= byte_from_tag(tag);
    }
    return result;
}

[[nodiscard]] inline constexpr auto tag_in_token(uint32_t token,
                                                 ResponseTag tag) -> bool {
    return bool(token &
                (1 << (static_cast<size_t>(tag) + static_cast<size_t>(8))));
}

[[nodiscard]] inline constexpr auto build_token(uint8_t reg, uint8_t tags = 0)
    -> uint32_t {
    return (static_cast<uint32_t>(tags) << 8) | (reg);
}

template <typename RegType>
[[nodiscard]] inline constexpr auto reg_from_token(uint32_t id) -> RegType {
    return static_cast<RegType>(static_cast<uint8_t>(id & 0xff));
}
}  // namespace utils

}  // namespace spi