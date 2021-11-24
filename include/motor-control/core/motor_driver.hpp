#pragma once

#include "common/core/bit_utils.hpp"
#include "motor_driver_config.hpp"
#include "spi.hpp"

namespace motor_driver {

using namespace motor_driver_config;

enum class Mode : uint8_t { WRITE = 0x80, READ = 0x0 };

constexpr auto command_byte(Mode mode, DriverRegisters::Addresses motor_reg)
    -> uint8_t {
    return static_cast<uint8_t>(mode) | static_cast<uint8_t>(motor_reg);
}

/*
 * MotorDriver uses SPI communication to initialize, send and receive status and
 * data to and from the TMC2130 motor driver.
 */
class MotorDriver {
  public:
    spi::TMC2130Spi::BufferType rxBuffer{0};

    explicit MotorDriver(spi::TMC2130Spi& spi, RegisterConfig conf)
        : spi_comms(spi), register_config(conf) {}

    void setup() {
        write(DriverRegisters::Addresses::GCONF, register_config.gconf);
        write(DriverRegisters::Addresses::IHOLD_IRUN,
              register_config.ihold_irun);
        write(DriverRegisters::Addresses::CHOPCONF, register_config.chopconf);
        write(DriverRegisters::Addresses::THIGH, register_config.thigh);
        write(DriverRegisters::Addresses::COOLCONF, register_config.coolconf);
    }

    auto read(DriverRegisters::Addresses motor_reg, uint32_t& command_data)
        -> spi::TMC2130Spi::BufferType {
        auto txBuffer =
            build_command(command_byte(Mode::READ, motor_reg), command_data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        return txBuffer;
    }

    auto write(DriverRegisters::Addresses motor_reg,
               const uint32_t& command_data) -> spi::TMC2130Spi::BufferType {
        auto txBuffer =
            build_command(command_byte(Mode::WRITE, motor_reg), command_data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        return txBuffer;
    }

  private:
    auto build_command(uint8_t command, const uint32_t& command_data)
        -> spi::TMC2130Spi::BufferType {
        // need to pass in data parameter and use int_to_bytes here
        auto txBuffer = spi::TMC2130Spi::BufferType{0};
        auto* iter = txBuffer.begin();
        iter = bit_utils::int_to_bytes(command, iter, txBuffer.end());
        // NOLINTNEXTLINE(clang-diagnostic-unused-result)
        iter = bit_utils::int_to_bytes(command_data, iter, txBuffer.end());
        return txBuffer;
    }

    spi::TMC2130Spi& spi_comms;
    RegisterConfig register_config;
};

}  // namespace motor_driver
