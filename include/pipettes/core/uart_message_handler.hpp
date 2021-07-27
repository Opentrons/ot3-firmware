#pragma once

#include <variant>

#include "common/core/io.hpp"
#include "common/core/motor.hpp"
#include "pipettes/core/communication.hpp"
#include "pipettes/core/pipette_messages.hpp"

template <io::WriterProtocol Writer, motor_protocol::MotorProtocol Motor>
class MessageHandler {
  public:
    explicit MessageHandler(Writer &writer, Motor &motor,
                            communication::MessageWriter &message_writer)
        : writer(writer), motor(motor), message_writer(message_writer) {}

    void handle_message(const pipette_messages::ReceivedMessage &message) {
        auto handle = [this](auto m) { this->handle(m); };
        std::visit(handle, message);
    }

  private:
    void handle(const pipette_messages::Stop &m) {
        static_cast<void>(m);
        motor.stop();
    }

    void handle(const pipette_messages::Move &m) {
        static_cast<void>(m);
        motor.move();
    }

    void handle(const pipette_messages::Status &m) {
        static_cast<void>(m);
        motor.get_status();
        pipette_messages::GetStatusResult message{motor.get_current_status(),
                                                  motor.get_current_data()};
        message_writer.write(writer, message);
    }

    void handle(const pipette_messages::Setup &m) {
        static_cast<void>(m);
        motor.setup();
    }

    void handle(const pipette_messages::SetSpeed &m) {
        motor.set_speed(m.mm_sec);
    }

    void handle(const pipette_messages::GetSpeed &m) {
        static_cast<void>(m);
        pipette_messages::GetSpeedResult message{motor.get_speed()};
        message_writer.write(writer, message);
    }

    void handle(const std::monostate &m) { static_cast<void>(m); }

    Writer &writer;
    Motor &motor;
    communication::MessageWriter &message_writer;
};
