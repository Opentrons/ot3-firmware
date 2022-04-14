#include "catch2/catch.hpp"
#include "motor-control/core/stepper_motor/tmc2130_driver.hpp"
#include "spi/core/writer.hpp"

#include "common/tests/mock_message_queue.hpp"


TEST_CASE("Setup a tmc2130 motor driver") {
    auto fake_queue =
    auto spi_writer = spi::writer::Writer{};
    tmc2130::TMC2130DriverConfig driver_config{
        .registers = {.gconfig = {.en_pwm_mode = 1},
                      .ihold_irun = {.hold_current = 0x2,
                                     .run_current = 0x2,
                                     .hold_current_delay = 0x7},
                      .tpowerdown = {},
                      .tcoolthrs = {.threshold = 0},
                      .thigh = {.threshold = 0xFFFFF},
                      .chopconf = {.toff = 0x5,
                                   .hstrt = 0x5,
                                   .hend = 0x3,
                                   .tbl = 0x2,
                                   .mres = 0x3},
                      .coolconf = {.sgt = 0b110}},
        .current_config = {
            .r_sense = 0.1,
            .v_sf = 0.325,
        }};

    auto subject = tmc2130::driver::TMC2130{spi_writer, task_queue, driver_config};

    GIVEN("a tmc2130 motor driver") {
        WHEN("Setup is called") {
            subject.setup();
            THEN("Registers have the configured values.") {
                auto register_config = driver_config.registers;
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
