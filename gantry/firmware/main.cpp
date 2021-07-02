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

static void vTaskCode(void *vParam);
static void spiTask(void *spiParam);
static void driveMotor();

uint8_t aTxBuffer[5] = {0x6B, 0, 0, 0, 0};
// uint8_t aTxBuffer[] = "Hi";
uint8_t aRxBuffer[5];
__IO uint32_t wTransferState = TRANSFER_WAIT;

static uint16_t Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2,
                          uint16_t BufferLength);

static void vTaskCode(void *vParam) {
    vParam = nullptr;

    constexpr auto timeout = 0xFFFF;
    constexpr auto MOVE = 'm';
    constexpr auto CHANGE = 'c';
    constexpr auto OK = "OK";

    uint8_t c = 0;
    UART_HandleTypeDef huart1 = MX_LPUART1_UART_Init();

    for (;;) {
        HAL_UART_Receive(&huart1, &c, 1, timeout);

        if (c == MOVE) {
            driveMotor();
            HAL_UART_Transmit(&huart1, (uint8_t *)OK, sizeof(OK), timeout);
        } else if (c == CHANGE) {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    }
}

static void spiTask(void *spiParam) {
    spiParam = nullptr;
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    //    MX_DMA_Init();
    SPI_HandleTypeDef hspi1 = MX_SPI1_Init();
    constexpr auto timeout = 0xFFFF;

    for (;;) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
        HAL_SPI_TransmitReceive(&hspi1, aTxBuffer, aRxBuffer, sizeof(aTxBuffer),
                                timeout);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
        vTaskDelay(20);
    }
}

static void driveMotor() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
    vTaskDelay(120);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
}

auto main() -> int {
    HardwareInit();
    /** Initializes the peripherals clocks
     */
    __HAL_RCC_GPIOA_CLK_ENABLE();  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    RCC_Peripheral_Clock_Select();
    MX_GPIOA_Init();
    __HAL_RCC_SPI2_CLK_ENABLE();

    //    xTaskCreateStatic(vTaskCode, "USART Task", stack.size(), nullptr, 1,
    //                      stack.data(), &data);

    xTaskCreateStatic(spiTask, "SPI Task", stack.size(), nullptr, 1,
                      stack.data(), &data);
    vTaskStartScheduler();

    return 0;
}

static uint16_t Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2,
                          uint16_t BufferLength) {
    while (BufferLength--) {
        if ((*pBuffer1) != *pBuffer2) {
            return BufferLength;
        }
        pBuffer1++;
        pBuffer2++;
    }

    return 0;
}