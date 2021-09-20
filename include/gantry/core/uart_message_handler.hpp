#pragma once

#include "common/core/io.hpp"
#include "common/core/motor.hpp"
#include "communication.hpp"
#include "gantry_messages.hpp"

template <io::WriterProtocol Writer, motor_protocol::MotorProtocol Motor>
class MessageHandler {
  public:
    explicit MessageHandler(Writer &writer, Motor &motor,
                            gantry_communication::MessageWriter &message_writer)
        : writer(writer), motor(motor), message_writer(message_writer) {}

    void handle_message(const gantry_messages::ReceivedMessage &message) {
        auto handle = [this](auto m) { this->handle(m); };
        std::visit(handle, message);
    }

  private:
    void handle(const gantry_messages::Stop &m) {
        static_cast<void>(m);
        motor.motion_controller.stop();
    }

    void handle(const gantry_messages::Move &m) {
        static_cast<void>(m);
        motor.motion_controller.move();
    }

    void handle(const gantry_messages::Status &m) {
        static_cast<void>(m);
        motor.driver.get_status();
        gantry_messages::GetStatusResult message{
            motor.driver.get_current_status(), motor.driver.get_current_data()};
        message_writer.write(writer, message);
    }

    void handle(const gantry_messages::Setup &m) {
        static_cast<void>(m);
        motor.driver.setup();
    }

    void handle(const gantry_messages::SetSpeed &m) {
        motor.motion_controller.set_speed(m.mm_sec);
    }

    void handle(const gantry_messages::GetSpeed &m) {
        static_cast<void>(m);
        gantry_messages::GetSpeedResult message{
            motor.motion_controller.get_speed()};
        message_writer.write(writer, message);
    }

    void handle(const std::monostate &m) { static_cast<void>(m); }

    Writer &writer;
    Motor &motor;
    gantry_communication::MessageWriter &message_writer;
};
