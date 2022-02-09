#pragma once

#include <array>
#include <cstdint>

template <typename Buffer>
concept GenericByteBuffer = requires(Buffer buff) {
    {std::is_same_v<Buffer, std::array<uint8_t, sizeof(Buffer)>>};
};