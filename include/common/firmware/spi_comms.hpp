#pragma once
#include <cstdint>
#include <span>

namespace spi {
typedef enum { common, gantry, head, pipette } component;
typedef enum { gantry_x, gantry_y, motor_a, motor_z, na } sub_component;
typedef enum { _SPI0, _SPI1, _SPI2, _SPI3 } SPI_interface;

struct module {
    component comp;
    sub_component sub_comp;
    SPI_interface intf;
    //SPI_HandleTypeDef 
    void* ptr; 
};

class Spi {
  public:
    static constexpr auto BUFFER_SIZE = 5;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;
    Spi();
    Spi(struct module);
    void transmit_receive(const BufferType& transmit, BufferType& receive);
    void* get_SPI_config();
    void set_SPI_config(component, sub_component, SPI_interface, void*);

  private:
    static constexpr uint32_t TIMEOUT = 0xFFFF;
    struct module* SPI_config;
};
}  // namespace spi

