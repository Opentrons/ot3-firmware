#include "catch2/catch.hpp"
#include "common/core/io.hpp"
#include "pipettes/core/uart_message_handler.hpp"
#include "pipettes/tests/mock_message_queue.hpp"
#include "pipettes/tests/test_motor_control.hpp"
#include "pipettes/tests/test_spi_comms.hpp"

using namespace test_motor_control;
using namespace test_spi;
using namespace mock_message_queue;

template <typename T, std::size_t L>
struct DataWriter {
    void write(std::span<uint8_t>& p) {
        for (auto i : p) {
            *iter++ = i;
        }
    }

    std::array<T, L> buff{};
    typename std::array<T, L>::iterator iter = buff.begin();
};

SCENARIO("messages can control motor") {
    GIVEN("set speed and stop command") {
        communication::MessageWriter message_writer{};
        TestSpi testSpi{};
        MockMessageQueue<Move> queue;

        TestMotor motor{testSpi, queue};
        DataWriter<uint8_t, 8> dw{};
        MessageHandler handler{dw, motor, message_writer};
        pipette_messages::ReceivedMessage message;

        WHEN("the set speed message is handled") {
            message = pipette_messages::SetSpeed{0x10};
            handler.handle_message(message);
            THEN("the speed should become 0x10") {
                REQUIRE(motor.motion_controller.get_speed() == 0x10);
            }
        }

        WHEN("the stop message is handled") {
            message = pipette_messages::Stop{};
            handler.handle_message(message);
            THEN("the speed should become zero") {
                REQUIRE(motor.motion_controller.get_speed() == 0x0);
            }
        }
    }

    GIVEN("move command") {
        communication::MessageWriter message_writer{};
        TestSpi testSpi{};
        MockMessageQueue<Move> queue;
        TestMotor motor{testSpi, queue};
        DataWriter<uint8_t, 8> dw{};
        MessageHandler handler{dw, motor, message_writer};
        pipette_messages::ReceivedMessage message;
        message = CanMove{100};
        handler.handle_message(message);

        WHEN("the message is handled") {
            THEN("the motor should move") {
                REQUIRE(motor.motion_controller.get_speed() == 10000);
            }
        }
    }

    GIVEN("status command") {
        communication::MessageWriter message_writer{};
        TestSpi testSpi{};
        MockMessageQueue<Move> queue;
        TestMotor motor{testSpi, queue};
        DataWriter<uint8_t, 8> dw{};
        MessageHandler handler{dw, motor, message_writer};
        pipette_messages::ReceivedMessage message;
        message = pipette_messages::Status{};
        handler.handle_message(message);

        WHEN("the message is handled") {
            THEN("the writer should write the motor driver status and data") {
                pipette_messages::GetStatusResult m{0x00, 0x00};
                DataWriter<uint8_t, 8> w{};
                message_writer.write(w, m);
                REQUIRE(dw.buff == w.buff);
            }
        }
    }

    GIVEN("setup command") {
        communication::MessageWriter message_writer{};
        MockMessageQueue<Move> queue;
        TestSpi testSpi{};
        TestMotor motor{testSpi, queue};
        DataWriter<uint8_t, 8> dw{};
        MessageHandler handler{dw, motor, message_writer};
        pipette_messages::ReceivedMessage message;
        message = pipette_messages::Setup{};
        handler.handle_message(message);

        WHEN("the message is handled") {
            THEN("the state of the motor driver should be updated") {
                //                REQUIRE(motor.driver.state == "setup");
            }
        }
    }

    GIVEN("get speed command") {
        communication::MessageWriter message_writer{};
        TestSpi testSpi{};
        MockMessageQueue<Move> queue;
        TestMotor motor{testSpi, queue};
        DataWriter<uint8_t, 8> dw{};
        MessageHandler handler{dw, motor, message_writer};
        pipette_messages::ReceivedMessage message;
        message = pipette_messages::GetSpeed{};
        handler.handle_message(message);

        WHEN("the message is handled") {
            THEN("the writer should write the speed result") {
                pipette_messages::GetSpeedResult m{0x00};
                DataWriter<uint8_t, 8> w{};
                message_writer.write(w, m);
                REQUIRE(dw.buff == w.buff);
            }
        }
    }
}

SCENARIO("process_buffer works") {
    GIVEN("a 5 byte buffer") {
        auto rxBuffer = test_motor_control::BufferType{1, 2, 3, 4, 5};
        uint8_t status = 0x0;
        uint32_t data = 0x0;
        typedef TestSpi testSpi;
        TestMotorDriver<testSpi>::process_buffer(rxBuffer, status, data);
        WHEN("process_buffer is called") {
            THEN("the status and data are updated") {
                REQUIRE(status == 0x01);
                REQUIRE(data == 0x2030405);
            }
        }
    }
}
