#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32g4xx.h"
#include "stm32g4xx_hal_conf.h"
#include "task.h"
// clang-format on

#include "gantry/firmware/uart.hpp"


// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
UART_HandleTypeDef huart1;

static void vTaskCode(void* vParamater);

static void vTaskCode(void* vParamater) {
    vParamater = nullptr;

    constexpr auto timeout = 0xFFFF;

    uint8_t c = 0;

    for (;;) {
        HAL_UART_Receive(&huart1, &c, 1, timeout);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)

        HAL_UART_Transmit(&huart1, &c, 1, timeout);
    }
}


auto main() -> int {
    HardwareInit();
    /** Initializes the peripherals clocks
     */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }

    constexpr auto stack_size = 100;
    static std::array<StackType_t, stack_size> stack;

    // Internal FreeRTOS data structure
    static StaticTask_t
        data;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    huart1 = MX_USART1_UART_Init(huart1);
    xTaskCreateStatic(vTaskCode, "USART Task", stack.size(), nullptr, 1,
                      stack.data(), &data);

    vTaskStartScheduler();

    return 0;
}

