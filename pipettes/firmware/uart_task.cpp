#include "pipettes/firmware/uart_task.hpp"

#include "FreeRTOS.h"
#include "common/firmware/uart.h"
#include "common/hal/motor.hpp"
#include "firmware/common/spi_comms.hpp"
#include "firmware/common/uart_comms.hpp"
#include "pipettes/core/communication.hpp"
#include "pipettes/firmware/motor_control.hpp"
#include "task.h"

template <typename T, motor_protocol::MotorProtocol Motor>
requires io::WriterProtocol<T>
class MessageHandler {
  public:
    explicit MessageHandler(T &writer, Motor &motor,
                            communication::MessageWriter &message_writer)
        : writer(writer), motor(motor), message_writer(message_writer) {}

    void handle(const pipette_messages::ReceivedMessage &message) {
        auto handle = [this](auto m) { this->handle(m); };
        std::visit(handle, message);
    }

  private:
    void handle(const pipette_messages::Stop &m) {
        static_cast<void>(m);
        motor.stop();
    }

    void handle(const pipette_messages::Move &m) { motor.move(); }

    void handle(const pipette_messages::Status &m) {
        static_cast<void>(m);
        motor.get_status();
        pipette_messages::GetStatusResult message{motor.status, motor.data};
        message_writer.write(writer, message);
    }

    void handle(const pipette_messages::Setup &m) { motor.setup(); }

    void handle(const pipette_messages::SetSpeed &m) {
        motor.set_speed(m.mm_sec);
    }

    void handle(const pipette_messages::GetSpeed &m) {
        static_cast<void>(m);
        pipette_messages::GetSpeedResult message{motor.get_speed()};
        message_writer.write(writer, message);
    }

    void handle(const std::monostate &m) { static_cast<void>(m); }

    T &writer;
    Motor &motor;
    communication::MessageWriter &message_writer;
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static communication::MessageReader message_reader{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static communication::MessageWriter message_writer{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static spi::Spi spi_comms{};
static motor_control::MotorControl motor{spi_comms};

static void run(void *parameter) {
    parameter = nullptr;

    UART_HandleTypeDef huart1 = MX_LPUART1_UART_Init();
    uart_comms::Uart uart{&huart1};
    MessageHandler handler{uart, motor, message_writer};

    for (;;) {
        auto message = message_reader.read(uart);
        handler.handle(message);
    }
}

static constexpr auto stack_size = 200;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::array<StackType_t, stack_size> stack;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static StaticTask_t data{};

/**
 * Create the task.
 */
void uart_task::start() {
    xTaskCreateStatic(run, "USART Task", stack.size(), nullptr, 1, stack.data(),
                      &data);
}
