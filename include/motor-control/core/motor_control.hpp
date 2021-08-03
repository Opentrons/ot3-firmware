#pragma once

#include "common/core/bit_utils.hpp"
#include "common/firmware/motor.h"
#include "spi.hpp"

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

template <spi::SpiProtocol Spi>
class MotorControl {
  public:
    MotorControl(Spi& spi) : spi_comms(spi) {}

    void set_speed(uint32_t s) { speed = s; }
    [[nodiscard]] auto get_speed() const -> uint32_t { return speed; }

    void move() {
        Set_Enable_Pin();
        Set_Direction();
        const int tries = 10000;
        for (int i = 0; i < tries; i++) {
            Set_Step();
            Reset_Step();
        }
    }

    void setup() {
        // GCONF 0x01
        // IHOLD_IRUN 0x1010
        // CHOPCONF 0x8008
        constexpr uint32_t gconf_data = 0x01;
        constexpr uint32_t ihold_irun_data = 0x1010;
        constexpr uint32_t chopconf = 0x8008;
        auto txBuffer = build_command(
            command_byte(Mode::WRITE, MotorRegisters::GCONF), gconf_data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        txBuffer =
            build_command(command_byte(Mode::WRITE, MotorRegisters::IHOLD_IRUN),
                          ihold_irun_data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        txBuffer = build_command(
            command_byte(Mode::WRITE, MotorRegisters::CHOPCONF), chopconf);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        process_buffer(rxBuffer, status, data);
    }

    void get_status() {
        reset_data();
        reset_status();
        auto txBuffer = build_command(
            command_byte(Mode::READ, MotorRegisters::DRVSTATUS), data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        process_buffer(rxBuffer, status, data);
    }

    void stop() {
        Reset_Step();
        Reset_Enable_Pin();
    }

    [[nodiscard]] auto get_current_status() const -> uint8_t { return status; }

    [[nodiscard]] auto get_current_data() const -> uint32_t { return data; }

  private:
    uint8_t status = 0x0;
    uint32_t data = 0x0;
    uint32_t speed{0};

    static constexpr auto BUFFER_SIZE = 5;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;
    BufferType rxBuffer{0};

    auto build_command(uint8_t command, const uint32_t& command_data)
        -> BufferType {
        // need to pass in data parameter and use int_to_bytes here
        auto txBuffer = BufferType{0};
        auto iter = txBuffer.begin();
        iter = bit_utils::int_to_bytes(command, iter, txBuffer.end());
        // NOLINTNEXTLINE(clang-diagnostic-unused-result)
        bit_utils::int_to_bytes(command_data, iter, txBuffer.end());
        return txBuffer;
    }

    void reset_data() { data = 0x0; }

    void reset_status() { status = 0x0; }

    static void process_buffer(const BufferType& rxBuffer, uint8_t& status,
                               uint32_t& data) {
        auto iter = rxBuffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, rxBuffer.cend(), status);
        // NOLINTNEXTLINE(clang-diagnostic-unused-result)
        bit_utils::bytes_to_int(iter, rxBuffer.cend(), data);
    }

    Spi spi_comms;
};

}  // namespace motor_control