#include <array>

#include "catch2/catch.hpp"
#include "common/simulation/spi.hpp"

SCENARIO("build_command works") {
    auto arr = spi::SpiDeviceBase::BufferType{};
    GIVEN("a message buffer") {
        WHEN("called with write mode") {
            spi::SpiDeviceBase::build_command(
                arr, spi::SpiDeviceBase::Mode::WRITE, 0x12, 0x01020304);
            THEN("the write bit is set") { REQUIRE(arr[0] == 0x92); }
            THEN("the data is correct.") {
                REQUIRE(arr[1] == 0x1);
                REQUIRE(arr[2] == 0x2);
                REQUIRE(arr[3] == 0x3);
                REQUIRE(arr[4] == 0x4);
            }
        }

        WHEN("called read write mode") {
            spi::SpiDeviceBase::build_command(
                arr, spi::SpiDeviceBase::Mode::READ, 0x12, 0xDEADBEEF);
            THEN("the write bit is not set") { REQUIRE(arr[0] == 0x12); }
            THEN("the data is correct.") {
                REQUIRE(arr[1] == 0xDE);
                REQUIRE(arr[2] == 0xAD);
                REQUIRE(arr[3] == 0xBE);
                REQUIRE(arr[4] == 0xEF);
            }
        }
    }
}

SCENARIO("simulator works") {
    auto transmit = spi::SpiDeviceBase::BufferType{};
    auto receive = spi::SpiDeviceBase::BufferType{};

    GIVEN("a register write command") {
        auto subject = sim_spi::SimSpiDeviceBase{};
        subject.build_command(transmit, spi::SpiDeviceBase::Mode::WRITE, 0x01,
                              0xBEEFDEAD);
        subject.transmit_receive(transmit, receive);

        WHEN("called with a read command") {
            spi::SpiDeviceBase::build_command(
                transmit, spi::SpiDeviceBase::Mode::READ, 0x01, 0);
            subject.transmit_receive(transmit, receive);
            THEN("the data is what was written") {
                REQUIRE(receive[1] == 0xBE);
                REQUIRE(receive[2] == 0xEF);
                REQUIRE(receive[3] == 0xDE);
                REQUIRE(receive[4] == 0xAD);
            }
        }
    }

    GIVEN("a registter map") {
        auto register_map =
            sim_spi::SimSpiDeviceBase::RegisterMap{{1, 2}, {2, 3}};
        auto subject = sim_spi::SimSpiDeviceBase{register_map};

        WHEN("called with a read command") {
            spi::SpiDeviceBase::build_command(
                transmit, spi::SpiDeviceBase::Mode::READ, 0x01, 0);
            subject.transmit_receive(transmit, receive);
            subject.transmit_receive(transmit, receive);
            THEN("the data is was in the register map") {
                REQUIRE(receive[1] == 0x0);
                REQUIRE(receive[2] == 0x0);
                REQUIRE(receive[3] == 0x0);
                REQUIRE(receive[4] == 0x2);
            }

            spi::SpiDeviceBase::build_command(
                transmit, spi::SpiDeviceBase::Mode::READ, 0x02, 0);
            subject.transmit_receive(transmit, receive);
            subject.transmit_receive(transmit, receive);
            THEN("the data is was in the register map") {
                REQUIRE(receive[1] == 0x0);
                REQUIRE(receive[2] == 0x0);
                REQUIRE(receive[3] == 0x0);
                REQUIRE(receive[4] == 0x3);
            }
        }
    }
}
