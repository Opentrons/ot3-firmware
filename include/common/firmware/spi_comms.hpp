#pragma once
#include <cstdint>
#include <span>

#include "platform_specific_hal_conf.h"


namespace spi {
struct SPI_interface {
    /*
      Incase there are multiple handles
      involed(like how the head was different SPI
      interfaces for it's A and Z motor) SPI_handle
      can point to an array of handles. SPI_handle
      to be instantiated in respective project's
      spi.c under void* get_SPI_handle(void*)

    */
    void* SPI_handle;
    GPIO_TypeDef * GPIO_handle;
    uint32_t pin;
};
class Spi {
  public:
    static constexpr auto BUFFER_SIZE = 5;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;
    explicit Spi(SPI_interface SPI_int);
    
    void transmit_receive(const BufferType& transmit, BufferType& receive);

  private:
    static constexpr uint32_t TIMEOUT = 0xFFFF;
    SPI_interface SPI_intf;
};
}  // namespace spi
