#pragma once

#include "common/core/bit_utils.hpp"
#include "common/core/spi.hpp"
#include "motor_driver_config.hpp"

namespace motor_driver {

using namespace motor_driver_config;

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

    auto read(DriverRegisters::Addresses motor_reg, uint32_t command_data)
        -> uint32_t {
        auto txBuffer = spi::TMC2130Spi::BufferType{};
        spi::TMC2130Spi::build_command(txBuffer, spi::TMC2130Spi::Mode::READ,
                                       motor_reg, command_data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        uint32_t response;
        auto iter = rxBuffer.cbegin() + 1;
        iter = bit_utils::bytes_to_int(iter, rxBuffer.cend(), response);
        return response;
    }

    auto write(DriverRegisters::Addresses motor_reg, uint32_t command_data)
        -> spi::TMC2130Spi::BufferType {
        auto txBuffer = spi::TMC2130Spi::BufferType{};
        spi::TMC2130Spi::build_command(txBuffer, spi::TMC2130Spi::Mode::WRITE,
                                       motor_reg, command_data);
        spi_comms.transmit_receive(txBuffer, rxBuffer);
        return txBuffer;
    }

  private:
    spi::TMC2130Spi& spi_comms;
    RegisterConfig register_config;
};

}  // namespace motor_driver
