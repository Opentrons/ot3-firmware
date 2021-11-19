#pragma once

#include "common/core/bit_utils.hpp"
#include "motor_driver_config.hpp"
#include "spi.hpp"

namespace motor_driver {

using namespace motor_driver_config;

enum class Mode : uint8_t { WRITE = 0x80, READ = 0x0 };

constexpr auto command_byte(Mode mode, DriverRegisters motor_reg) -> uint8_t {
    return static_cast<uint8_t>(mode) | static_cast<uint8_t>(motor_reg);
}

/*
 * MotorDriver uses SPI communication to initialize, send and receive status and
 * data to and from the TMC2130 motor driver.
 */
template <spi::TMC2130Spi SpiDriver>
class MotorDriver {
  public:
    explicit MotorDriver(SpiDriver& spi, RegisterConfig conf)
        : spi_comms(spi), register_config(conf) {}

    void setup() {
        write(DriverRegisters::GCONF, register_config.gconf);
        write(DriverRegisters::IHOLD_IRUN, register_config.ihold_irun);
        write(DriverRegisters::CHOPCONF, register_config.chopconf);
        write(DriverRegisters::THIGH, register_config.thigh);
        write(DriverRegisters::COOLCONF, register_config.coolconf);

        process_buffer(rxBuffer, status, data);
    }

    void get_status() {
        reset_data();
        reset_status();
        read(DriverRegisters::DRVSTATUS, data);

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
        iter = bit_utils::int_to_bytes(command_data, iter, txBuffer.end());
        return txBuffer;
    }

    auto read(DriverRegisters motor_reg, uint32_t& command_data) -> BufferType {
        auto txBuffer =
            build_command(command_byte(Mode::READ, motor_reg), command_data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        return txBuffer;
    }

    auto write(DriverRegisters motor_reg, const uint32_t& command_data)
        -> BufferType {
        auto txBuffer =
            build_command(command_byte(Mode::WRITE, motor_reg), command_data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        return txBuffer;
    }

    void reset_data() { data = 0x0; }

    void reset_status() { status = 0x0; }

    static void process_buffer(const BufferType& rxBuffer, uint8_t& status,
                               uint32_t& data) {
        auto iter = rxBuffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, rxBuffer.cend(), status);
        // NOLINTNEXTLINE(clang-diagnostic-unused-result)
        iter = bit_utils::bytes_to_int(iter, rxBuffer.cend(), data);
    }

    SpiDriver spi_comms;
    RegisterConfig register_config;
};

}  // namespace motor_driver