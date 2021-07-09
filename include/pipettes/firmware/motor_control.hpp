#pragma once

#include "firmware/common/spi_comms.hpp"

namespace motor_control {

/**
 * Implementation of the Motor concept.
 *
 * TODO (2021-06-29): Issue SPI commands to motor.
 */

enum class MotorRegisters : uint8_t {
    WRITE = 0x80,
    IHOLD_IRUN = 0x10,
    GCONF = 0x00,
    GSTAT = 0x01,
    IOIN = 0x04,
    DRVSTATUS = 0x6F,
    DCCTRL = 0x6E,
    COOLCONF = 0x6D,
    CHOPCONF = 0x6C
};

class MotorControl {
  public:
    uint8_t status = 0x0;
    uint32_t data = 0x0;
    MotorControl(spi::Spi spi_comms) : spi_comms(spi_comms) {}
    void set_speed(uint32_t s);
    [[nodiscard]] auto get_speed() const -> uint32_t { return speed; }
    void move();
    void setup();
    void get_status();

  private:
    uint32_t speed{0};
    spi::Spi spi_comms;
    void build_command(MotorRegisters command, std::span<uint8_t>& output);
    uint32_t reset_data();
};

}  // namespace motor_control