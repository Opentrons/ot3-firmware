#pragma once

#include "firmware/common/spi_comms.hpp"

namespace motor_control {

const uint8_t WRITE = 0x80;
/**
 * Implementation of the Motor concept.
 *
 */

enum class MotorRegisters : uint8_t {
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
    void set_speed(uint32_t s) { speed = s; }
    [[nodiscard]] auto get_speed() const -> uint32_t { return speed; }
    void move();
    void setup();
    void get_status();
    void stop();

  private:
    uint32_t speed{0};
    spi::Spi spi_comms;
    void build_command(uint8_t command, uint32_t& data, std::array<uint8_t, 5>& output);
    uint32_t reset_data();
    uint32_t reset_status();
};

}  // namespace motor_control