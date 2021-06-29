#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32g4xx.h"
#include "stm32g4xx_hal_conf.h"
#include "task.h"
// clang-format on
#include "firmware/common/uart_comms.hpp"
#include "pipettes/core/communication.hpp"

#include "common/firmware/uart.h"

constexpr auto stack_size = 100;
static std::array<StackType_t, stack_size>
    stack;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// Internal FreeRTOS data structure
static StaticTask_t
    data;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static void vTaskCode(void* vParamater);

static void vTaskCode(void* vParamater) {
    vParamater = nullptr;

    constexpr auto timeout = 0xFFFF;

    uint8_t c = 0;
    UART_HandleTypeDef huart1 = MX_LPUART1_UART_Init();

    for (;;) {
        HAL_UART_Receive(&huart1, &c, 1, timeout);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)

        HAL_UART_Transmit(&huart1, &c, 1, timeout);
    }
}

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    xTaskCreateStatic(vTaskCode, "USART Task", stack.size(), nullptr, 1,
                      stack.data(), &data);

    vTaskStartScheduler();

    return 0;
}
