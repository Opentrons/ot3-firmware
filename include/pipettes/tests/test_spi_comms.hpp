#pragma once

#include "../../include/pipettes/core/bit_utils.hpp"

class TestSpi {
  public:
      static constexpr auto BUFFER_SIZE = 5;
      using BufferType = std::array<uint8_t, BUFFER_SIZE>;
      TestSpi();
      send_command(BufferType aTxBuffer, uint32_t& data, uint8_t& status) {
        auto aRxBuffer = std::array<uint8_t, BUFFER_SIZE>{};
        auto rxiter = aRxBuffer.begin();
        rxiter = bit_utils::int_to_bytes(status, rxiter);
        bit_utils::int_to_bytes(data, rxiter);

        transmit_receive(aTxBuffer, aRxBuffer);

        status = *aRxBuffer.begin();
        std::span sp{aRxBuffer};
        bit_utils::bytes_to_int<uint32_t, uint8_t>(sp.subspan(1, 4), data);
      }
  private:
    void transmit_receive(BufferType& transmit, BufferType& receive) {receive = transmit;}

};
