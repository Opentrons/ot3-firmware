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
    static void move();
    void setup();
    void get_status();
    void stop();

  private:
    uint32_t speed{0};
    spi::Spi spi_comms;
    static constexpr auto BUFFER_SIZE = 5;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;
    static void build_command(uint8_t command, const uint32_t& command_data,
                              BufferType& txBuffer);
    void reset_data();
    void reset_status();
};

}  // namespace motor_control