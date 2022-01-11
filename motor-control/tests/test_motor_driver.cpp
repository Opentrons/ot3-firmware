#include "catch2/catch.hpp"
#include "motor-control/core/motor_driver.hpp"
#include "common/simulation/spi.hpp"
#include "motor-control/core/motor_driver_config.hpp"

using namespace motor_driver_config;

TEST_CASE("Setup") {
    auto spi = sim_spi::SimTMC2130Spi{};
    auto register_config = RegisterConfig{};
    register_config.gconf = 1;
    register_config.ihold_irun = 2;
    register_config.chopconf = 3;
    register_config.coolconf = 4;
    register_config.thigh = 5;
    auto subject = motor_driver::MotorDriver{spi, register_config};

    GIVEN("Driver") {
        WHEN("Setup is called") {
            subject.setup();
            THEN("Registers have the configured values.") {
                REQUIRE(subject.read(DriverRegisters::Addresses::GCONF, 0) ==register_config.gconf);
                REQUIRE(subject.read(DriverRegisters::Addresses::IHOLD_IRUN, 0)==
                      register_config.ihold_irun);
                REQUIRE(subject.read(DriverRegisters::Addresses::CHOPCONF, 0) == register_config.chopconf);
                REQUIRE(subject.read(DriverRegisters::Addresses::THIGH, 0) == register_config.thigh);
                REQUIRE(subject.read(DriverRegisters::Addresses::COOLCONF, 0) ==  register_config.coolconf);
            }
        }
    }
}

