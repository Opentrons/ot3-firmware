#pragma once

#include "common/hal/io.hpp"
#include "common/hal/motor.hpp"
#include "pipettes/core/communication.hpp"

#include <span>
#include <variant>

namespace uart_task {

template<typename T, motor_protocol::MotorProtocol Motor>
requires io::ReaderProtocol<T> && io::WriterProtocol <T>
class UartTask {
public:
    explicit UartTask(T uart, Motor motor) : uart(uart), motor(motor) {};

    /**
     * Start the uart task.
     */
    void run() {
        constexpr auto stack_size = 100;
        static std::array <StackType_t, stack_size> stack;
        StaticTask_t data;

        // Create the task passing `this` as the parameter.
        xTaskCreateStatic(UartTask < T, Motor > ::run, "USART Task", stack.size(),
                          this, 1, stack.data(), &data);
    };

private:
    /**
     * The entry point of the freertos task
     * @param parameter a pointer to a UartTask.
     */
    static void run(void *parameter) {
        UartTask<T, Motor> &task = *static_cast<UartTask<T, Motor> *>(parameter);

        communication::MessageReader messageReader;

        auto handle = [&task](auto m) { task.handle(m); };

        for (;;) {
            // Read a message and dispatch to handlers.
            auto message = messageReader.read(task.uart);
            std::visit(handle, message);
        }
    }

    void handle(const pipette_messages::Stop &m) {
        static_cast<void>(m)
        motor.set_speed(0);
    }

    void handle(const pipette_messages::SetSpeed &m) {
        motor.set_speed(m.mm_sec);
    }

    void handle(const pipette_messages::GetSpeed &m) {

    }

    void handle(const std::monostate &m) {

    }

    T uart;
    Motor motor;
};

}