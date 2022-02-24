#include "catch2/catch.hpp"
#include "common/simulation/spi.hpp"
#include "motor-control/core/motor_driver.hpp"
#include "motor-control/core/motor_driver_config.hpp"
#include "motor-control/core/tmc2130_registers.hpp"

using namespace motor_driver_config;

TEST_CASE("Setup") {
    auto spi = sim_spi::SimTMC2130Spi{};
    tmc2130::TMC2130RegisterMap register_config{
        .gconfig = {.en_pwm_mode = 1},
        .ihold_irun = {.hold_current = 0x2,
                       .run_current = 0x2,
                       .hold_current_delay = 0x7},
        .tpowerdown = {},
        .tcoolthrs = {.threshold = 0},
        .thigh = {.threshold = 0xFFFFF},
        .chopconf =
            {.toff = 0x5, .hstrt = 0x5, .hend = 0x3, .tbl = 0x2, .mres = 0x3},
        .coolconf = {.sgt = 0b110}};

    auto subject = motor_driver::MotorDriver{spi, register_config};

    GIVEN("Driver") {
        WHEN("Setup is called") {
            subject.setup();
            THEN("Registers have the configured values.") {
                REQUIRE(subject.tmc2130.get_gconf().value().en_pwm_mode ==
                        register_config.gconfig.en_pwm_mode);
                REQUIRE(subject.tmc2130.get_register_map()
                            .ihold_irun.hold_current ==
                        register_config.ihold_irun.hold_current);
                REQUIRE(
                    subject.tmc2130.get_register_map().ihold_irun.run_current ==
                    register_config.ihold_irun.run_current);
                REQUIRE(subject.tmc2130.get_register_map()
                            .ihold_irun.hold_current_delay ==
                        register_config.ihold_irun.hold_current_delay);
                REQUIRE(subject.tmc2130.get_chop_config().value().toff ==
                        register_config.chopconf.toff);
                REQUIRE(subject.tmc2130.get_chop_config().value().hstrt ==
                        register_config.chopconf.hstrt);
                REQUIRE(subject.tmc2130.get_chop_config().value().hend ==
                        register_config.chopconf.hend);
                REQUIRE(subject.tmc2130.get_chop_config().value().tbl ==
                        register_config.chopconf.tbl);
                REQUIRE(subject.tmc2130.get_chop_config().value().mres ==
                        register_config.chopconf.mres);
                REQUIRE(subject.tmc2130.get_register_map().coolconf.sgt ==
                        register_config.coolconf.sgt);
            }
        }
    }
}
