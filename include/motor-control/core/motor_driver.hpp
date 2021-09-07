#pragma once

#include "common/core/bit_utils.hpp"
#include "spi.hpp"

namespace motor_driver {

/**
 * Implementation of the Motor concept.
 *
 */

enum class DriverRegisters : uint8_t {
    GCONF = 0x00,
    GSTAT = 0x01,
    IOIN = 0x04,
    IHOLD_IRUN = 0x10,
    TPOWERDOWN = 0x11,
    TPWMTHRS = 0x13,
    TCOOLTHRS = 0x14,
    THIGH = 0x15,
    XDIRECT = 0x2D,
    VDCMIN = 0x33,
    CHOPCONF = 0x6C,
    COOLCONF = 0x6D,
    DCCTRL = 0x6E,
    DRVSTATUS = 0x6F,
    PWMCONF = 0x70,
    ENCM_CTRL = 0x72
};

enum class Mode : uint8_t { WRITE = 0x80, READ = 0x0 };

constexpr auto command_byte(Mode mode, DriverRegisters motor_reg) -> uint8_t {
    return static_cast<uint8_t>(mode) | static_cast<uint8_t>(motor_reg);
}

template <typename SpiDriver>
requires spi::TMC2130Spi<SpiDriver>
class MotorDriver {
  public:
    MotorDriver(SpiDriver& spi) : spi_comms(spi) {}

    void setup() {
        constexpr uint32_t gconf_data = 0x01;
        constexpr uint32_t ihold_irun_data = 0x1010;
        constexpr uint32_t chopconf = 0x8008;

        //        constexpr uint32_t gconf_data = 0x04;
        //        constexpr uint32_t ihold_irun_data = 0x71703;
        //        constexpr uint32_t chopconf = 0x101D5;
        //        constexpr uint32_t thigh = 0xFFFFF;
        //        constexpr uint32_t coolconf = 0x60000;

        auto txBuffer = build_command(
            command_byte(Mode::WRITE, DriverRegisters::GCONF), gconf_data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        txBuffer = build_command(
            command_byte(Mode::WRITE, DriverRegisters::IHOLD_IRUN),
            ihold_irun_data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        txBuffer = build_command(
            command_byte(Mode::WRITE, DriverRegisters::CHOPCONF), chopconf);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        process_buffer(rxBuffer, status, data);
    }

    void get_status() {
        reset_data();
        reset_status();
        auto txBuffer = build_command(
            command_byte(Mode::READ, DriverRegisters::DRVSTATUS), data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        process_buffer(rxBuffer, status, data);
    }

    [[nodiscard]] auto get_current_status() const -> uint8_t { return status; }

    [[nodiscard]] auto get_current_data() const -> uint32_t { return data; }

  private:
    uint8_t status = 0x0;
    uint32_t data = 0x0;

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

    SpiDriver spi_comms;
};

}  // namespace motor_driver