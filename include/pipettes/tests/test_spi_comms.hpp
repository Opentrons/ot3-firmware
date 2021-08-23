#pragma once

#include "common/core/bit_utils.hpp"

namespace test_spi {

constexpr const size_t BUFFER_SIZE = 5;

class TestSpi {
  public:
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;
    void transmit_receive(const BufferType& transmit, BufferType& receive) {
        receive = transmit;
    }
};

};  // namespace test_spi
