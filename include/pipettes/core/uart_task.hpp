#pragma once

#include "common/hal/io.hpp"
#include "common/hal/motor.hpp"
#include "pipettes/core/communication.hpp"

#include <span>
#include <variant>


template <typename T, MotorProtocol Motor>
requires ReaderProtocol<T> && WriterProtocol<T>
class UartTask {
public:
    explicit UartTask(T uart, Motor motor) : uart(uart), motor(motor) {};

    void run() {
        constexpr auto stack_size = 100;
        static std::array<StackType_t, stack_size> stack;
        StaticTask_t data;

        xTaskCreateStatic(UartTask<T, Motor>::run, "USART Task", stack.size(),
                          this, 1, stack.data(), &data);
    };

private:
    static void run(void * parameter) {
        UartTask<T, Motor> & task = *static_cast<UartTask<T, Motor>*>(parameter);

        MessageReader messageReader;

        auto l = [&task](auto m) {task.handle(m);};

        for (;;) {
            auto message = messageReader.read_command(task.uart);
            std::visit(l, message);
        }
    }

    void handle(const pipette_messages::Stop& m) {  // NOLINT(misc-unused-parameters)
        motor.set_speed(0);
    }

    void handle(const pipette_messages::SetSpeed& m) {
        motor.set_speed(m.mm_sec);
    }

    void handle(const pipette_messages::GetSpeed &m) {

    }

    void handle(const std::monostate& m) {

    }

    T uart;
    Motor motor;
    MessageReader messageReader{};
};
