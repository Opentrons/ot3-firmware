#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32g4xx.h"
#include "stm32g4xx_hal_conf.h"
#include "task.h"
// clang-format on

#include "pipettes/firmware/uart_task.hpp"
#include "common/firmware/uart.h"
#include "firmware/common/uart_comms.hpp"
#include "common/firmware/spi.h"
#include "pipettes/core/communication.hpp"

constexpr auto stack_size = 100;
static std::array<StackType_t, stack_size>
    stack;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// Internal FreeRTOS data structure
static StaticTask_t
    data;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static void spiTask(void* spiParam);

static const int BUFFER_SIZE = 5;
static const int SPI_TASK_DELAY = 20;
static const uint8_t COMMAND = 0x6B;

static void spiTask(void* spiParam) {
    spiParam = nullptr;
    std::array<uint8_t, BUFFER_SIZE> aTxBuffer = {COMMAND, 0, 0, 0, 0};
    std::array<uint8_t, BUFFER_SIZE> aRxBuffer{};
    Set_CS_Pin();
    //        MX_DMA_Init();
    SPI_HandleTypeDef hspi1 = MX_SPI1_Init();
    constexpr auto timeout = 0xFFFF;

    for (;;) {
        Reset_CS_Pin();
        HAL_SPI_TransmitReceive(&hspi1, aTxBuffer.data(), aRxBuffer.data(),
                                BUFFER_SIZE, timeout);
        Set_CS_Pin();
        vTaskDelay(SPI_TASK_DELAY);
    }
}

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();

    uart_task::start();

    xTaskCreateStatic(spiTask, "SPI Task", stack.size(), nullptr, 1,
                      stack.data(), &data);
    vTaskStartScheduler();

    return 0;
}
