#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/stepper_motor/tmc2130_driver.hpp"
#include "motor-control/core/stepper_motor/tmc2160.hpp"
#include "motor-control/core/stepper_motor/tmc2160_driver.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "motor-control/core/tasks/tmc2160_motor_driver_task.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

template<typename DriverType, typename DefaultConfigs, size_t queue_size>
struct DriverContainer {
    test_mocks::MockMessageQueue<spi::tasks::TaskMessage, queue_size> spi_queue;
    DefaultConfigs driver_config;
    DriverType driver;
};

template<size_t queue_size = 10>
struct TMC2130Container {
    test_mocks::MockMessageQueue<tmc2130::tasks::TaskMessage> resp_queue{};
    test_mocks::MockMessageQueue<spi::tasks::TaskMessage, queue_size> spi_queue{};
    spi::writer::Writer<test_mocks::MockMessageQueue> spi_writer{};
    tmc2130::configs::TMC2130DriverConfig driver_config{
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
    tmc2130::driver::TMC2130<spi::writer::Writer<test_mocks::MockMessageQueue>, test_mocks::MockMessageQueue<tmc2130::tasks::TaskMessage>> driver{spi_writer, resp_queue, driver_config};

    TMC2130Container() {
        spi_writer.set_queue(&spi_queue);
    }
};

template<size_t queue_size = 10>
struct TMC2160Container {
    test_mocks::MockMessageQueue<tmc2160::tasks::TaskMessage> resp_queue{};
    test_mocks::MockMessageQueue<spi::tasks::TaskMessage, queue_size> spi_queue{};
    spi::writer::Writer<test_mocks::MockMessageQueue> spi_writer{};
    tmc2160::configs::TMC2160DriverConfig driver_config{
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
            .coolconf = {.sgt = 0b110},
            .glob_scale = {.global_scaler = 112}},
        .current_config = {
            .r_sense = 0.1,
            .v_sf = 0.325,
        }};
    tmc2160::driver::TMC2160<spi::writer::Writer<test_mocks::MockMessageQueue>, test_mocks::MockMessageQueue<tmc2160::tasks::TaskMessage>> driver{spi_writer, resp_queue, driver_config};

    TMC2160Container() {
        spi_writer.set_queue(&spi_queue);
    }
};


TEST_CASE("Setup a tmc2130 motor driver") {
    TMC2130Container subject{};

    GIVEN("a tmc2130 motor driver") {
        WHEN("Setup is called") {
            subject.driver.write_config();
            THEN("Registers have the configured values.") {
                auto register_config = subject.driver_config.registers;
                REQUIRE(subject.driver.get_gconf().value().en_pwm_mode ==
                        register_config.gconfig.en_pwm_mode);
                REQUIRE(subject.driver.get_register_map().ihold_irun.hold_current ==
                        register_config.ihold_irun.hold_current);
                REQUIRE(subject.driver.get_register_map().ihold_irun.run_current ==
                        register_config.ihold_irun.run_current);
                REQUIRE(
                    subject.driver.get_register_map().ihold_irun.hold_current_delay ==
                    register_config.ihold_irun.hold_current_delay);
                REQUIRE(subject.driver.get_chop_config().value().toff ==
                        register_config.chopconf.toff);
                REQUIRE(subject.driver.get_chop_config().value().hstrt ==
                        register_config.chopconf.hstrt);
                REQUIRE(subject.driver.get_chop_config().value().hend ==
                        register_config.chopconf.hend);
                REQUIRE(subject.driver.get_chop_config().value().tbl ==
                        register_config.chopconf.tbl);
                REQUIRE(subject.driver.get_chop_config().value().mres ==
                        register_config.chopconf.mres);
                REQUIRE(subject.driver.get_register_map().coolconf.sgt ==
                        register_config.coolconf.sgt);
            }
        }
    }

}

TEST_CASE("Setup a tmc2160 motor driver") {
    TMC2160Container subject{};

    GIVEN("a tmc2160 motor driver") {
        WHEN("Setup is called") {
            subject.driver.write_config();
            THEN("Registers have the configured values.") {
                auto register_config = subject.driver_config.registers;
                REQUIRE(subject.driver.get_gconf().value().en_pwm_mode ==
                        register_config.gconfig.en_pwm_mode);
                REQUIRE(subject.driver.get_register_map().ihold_irun.hold_current ==
                        register_config.ihold_irun.hold_current);
                REQUIRE(subject.driver.get_register_map().ihold_irun.run_current ==
                        register_config.ihold_irun.run_current);
                REQUIRE(
                    subject.driver.get_register_map().ihold_irun.hold_current_delay ==
                    register_config.ihold_irun.hold_current_delay);
                REQUIRE(subject.driver.get_chop_config().value().toff ==
                        register_config.chopconf.toff);
                REQUIRE(subject.driver.get_chop_config().value().hstrt ==
                        register_config.chopconf.hstrt);
                REQUIRE(subject.driver.get_chop_config().value().hend ==
                        register_config.chopconf.hend);
                REQUIRE(subject.driver.get_chop_config().value().tbl ==
                        register_config.chopconf.tbl);
                REQUIRE(subject.driver.get_chop_config().value().mres ==
                        register_config.chopconf.mres);
                REQUIRE(subject.driver.get_register_map().coolconf.sgt ==
                        register_config.coolconf.sgt);
                REQUIRE(subject.driver.get_register_map().glob_scale.global_scaler ==
                        register_config.glob_scale.global_scaler);
            }
        }
    }


}

TEST_CASE("Test spi timeout routine") {
    TMC2130Container tmc2130{};

    TMC2160Container tmc2160{};

    uint32_t data = 0;

    GIVEN("a request to write to spi via the motor driver") {
        auto resp1 = tmc2130.driver.write(tmc2130::registers::Registers::CHOPCONF, data);
        auto resp2 = tmc2160.driver.write(tmc2160::registers::Registers::CHOPCONF, data);
        WHEN("When response is successful the first time") {
            THEN("The queue is only written to once") {
                REQUIRE(tmc2130.spi_queue.get_size() == 1);
                REQUIRE(tmc2130.spi_queue.get_size() == 1);
                REQUIRE(resp1 == true);
                REQUIRE(resp2 == true);
            }
        }

        WHEN("When the response exceeds retries") {
            for (int i=0; i<=10; i++) {
                tmc2130.driver.write(tmc2130::registers::Registers::CHOPCONF, data);
                tmc2160.driver.write(tmc2160::registers::Registers::CHOPCONF, data);
            }
            auto final_write_1 = tmc2130.driver.write(tmc2130::registers::Registers::CHOPCONF, data);
            auto final_write_2 = tmc2160.driver.write(tmc2160::registers::Registers::CHOPCONF, data);
            THEN("Then the write is not successful") {
                REQUIRE(final_write_1 == false);
                REQUIRE(final_write_2 == false);
            }

        }
    }
}
