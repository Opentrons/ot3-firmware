
#include "common/firmware/uart_task.hpp"

#include "FreeRTOS.h"
#include "common/firmware/spi_comms.hpp"
#include "common/firmware/uart.h"
#include "common/firmware/uart_comms.hpp"
#include "gantry/core/communication.hpp"
#include "gantry/core/uart_message_handler.hpp"
#include "motor-control/core/motor.hpp"
#include "task.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static gantry_communication::MessageReader message_reader{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static gantry_communication::MessageWriter message_writer{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static spi::Spi spi_comms{};

struct motion_controller::HardwareConfig GPIOConfig {
    .direction = {.port = GPIOB, .pin = GPIO_PIN_1},
    .step = {.port = GPIOA, .pin = GPIO_PIN_8},
    .enable = {.port = GPIOA, .pin = GPIO_PIN_10},
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static motor_class::Motor motor{spi_comms, GPIOConfig};

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
