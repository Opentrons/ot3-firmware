#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32g4xx.h"
#include "stm32g4xx_hal_conf.h"
#include "task.h"
// clang-format on

#include "common/firmware/spi.h"
#include "common/firmware/uart.h"

constexpr auto stack_size = 100;
static std::array<StackType_t, stack_size>
    stack;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// Internal FreeRTOS data structure
static StaticTask_t
    data;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static void uartTask(void *vParam);
static void spiTask(void *spiParam);

static const int BUFFER_SIZE = 5;
static const int SPI_TASK_DELAY = 20;
static const uint8_t COMMAND = 0x6B;

static void uartTask(void *vParam) {
    static_cast<void>(vParam);

    constexpr auto timeout = 0xFFFF;

    uint8_t c = 0;
    UART_HandleTypeDef huart1 = MX_LPUART1_UART_Init();

    for (;;) {
        HAL_UART_Receive(&huart1, &c, 1, timeout);
        HAL_UART_Transmit(&huart1, &c, 1, timeout);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    }
}

static void spiTask(void *spiParam) {
    static_cast<void>(spiParam);

    std::array<uint8_t, BUFFER_SIZE> aTxBuffer = {COMMAND, 0, 0, 0, 0};
    std::array<uint8_t, BUFFER_SIZE> aRxBuffer{};
    Set_CS_Pin();
    //        MX_DMA_Init();
    SPI_HandleTypeDef hspi2 = MX_SPI2_Init();
    constexpr auto timeout = 0xFFFF;

    for (;;) {
        Reset_CS_Pin();
        HAL_SPI_TransmitReceive(&hspi2, aTxBuffer.data(), aRxBuffer.data(),
                                BUFFER_SIZE, timeout);
        Set_CS_Pin();
        vTaskDelay(SPI_TASK_DELAY);
    }
}

auto main() -> int {
    HardwareInit();
    /** Initializes the peripherals clocks
     */
    RCC_Peripheral_Clock_Select();
    MX_GPIOA_Init();

    //    xTaskCreateStatic(uartTask, "USART Task", stack.size(), nullptr, 1,
    //                      stack.data(), &data);

    xTaskCreateStatic(spiTask, "SPI Task", stack.size(), nullptr, 1,
                      stack.data(), &data);
    vTaskStartScheduler();

    return 0;
}
