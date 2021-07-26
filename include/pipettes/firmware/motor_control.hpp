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

enum class Mode : uint8_t { WRITE = 0x80, READ = 0x0 };

constexpr auto command_byte(Mode mode, MotorRegisters motor_reg) -> uint8_t {
    return static_cast<uint8_t>(mode) | static_cast<uint8_t>(motor_reg);
}

class MotorControl {
  public:
    MotorControl(spi::Spi spi_comms) : spi_comms(spi_comms) {}
    void set_speed(uint32_t s) { speed = s; }
    [[nodiscard]] auto get_speed() const -> uint32_t { return speed; }
    static void move();
    void setup();
    void get_status();
    static void stop();
    [[nodiscard]] auto get_current_status() const -> uint8_t { return status; }
    [[nodiscard]] auto get_current_data() const -> uint32_t { return data; }

  private:
    uint8_t status = 0x0;
    uint32_t data = 0x0;
    uint32_t speed{0};
    spi::Spi::BufferType rxBuffer{0};
    spi::Spi spi_comms;
    static constexpr auto BUFFER_SIZE = 5;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;
    auto build_command(uint8_t command, const uint32_t& command_data)
        -> BufferType;
    void reset_data();
    void reset_status();
    static void process_buffer(const BufferType& rxBuffer, uint8_t& status,
                               uint32_t& data);
};

}  // namespace motor_control