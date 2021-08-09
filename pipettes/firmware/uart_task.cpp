#include "pipettes/firmware/uart_task.hpp"

#include "FreeRTOS.h"
#include "common/core/motor.hpp"
#include "common/firmware/spi_comms.hpp"
#include "common/firmware/uart.h"
#include "common/firmware/uart_comms.hpp"
#include "motor-control/core/motor.hpp"
//#include "motor-control/core/spi.hpp"
#include "pipettes/core/communication.hpp"
#include "pipettes/core/uart_message_handler.hpp"
#include "task.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static communication::MessageReader message_reader{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static communication::MessageWriter message_writer{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static spi::Spi spi_comms{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static motor_class::Motor motor{spi_comms};

static void run(void *parameter) {
    parameter = nullptr;

    UART_HandleTypeDef huart1 = MX_LPUART1_UART_Init();
    uart_comms::Uart uart{&huart1};
    MessageHandler handler{uart, motor, message_writer};

    for (;;) {
        auto message = message_reader.read(uart);
        handler.handle_message(message);
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
