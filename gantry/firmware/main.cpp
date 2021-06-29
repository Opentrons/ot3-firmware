#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32g4xx.h"
#include "stm32g4xx_hal_conf.h"
#include "task.h"
// clang-format on

#include "common/firmware/uart.h"
#include "common/firmware/spi.h"

constexpr auto stack_size = 100;
static std::array<StackType_t, stack_size>
    stack;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// Internal FreeRTOS data structure
static StaticTask_t
    data;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static void vTaskCode(void* vParam);
static void spiTask(void* spiParam);

uint8_t aTxBuffer[] = "Hi";
uint8_t aRxBuffer[BUFFERSIZE];
__IO uint32_t wTransferState = TRANSFER_WAIT;

static uint16_t Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint16_t BufferLength);



static void vTaskCode(void* vParam) {
    vParam = nullptr;

    constexpr auto timeout = 0xFFFF;

    uint8_t c = 0;
    UART_HandleTypeDef huart1 = MX_LPUART1_UART_Init();

    for (;;) {
        HAL_UART_Receive(&huart1, &c, 1, timeout);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)

        HAL_UART_Transmit(&huart1, &c, 1, timeout);
    }
}

static void spiTask(void* spiParam) {
    spiParam = nullptr;
    MX_GPIO_Init();
    MX_DMA_Init();
    SPI_HandleTypeDef hspi1 = MX_SPI1_Init();
    constexpr auto timeout = 0xFFFF;

    /*##-1- Start the Full Duplex Communication process ########################*/
    /* While the SPI in TransmitReceive process, user can transmit data through
       "aTxBuffer" buffer & receive data through "aRxBuffer" */
    for (;;) {
        HAL_SPI_Transmit(&hspi1, (uint8_t *)aTxBuffer, BUFFERSIZE, timeout);
        vTaskDelay(2);
    }
//    if (HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)aTxBuffer, (uint8_t *)aRxBuffer, BUFFERSIZE) != HAL_OK)
//    {
//        /* Transfer error in transmission process */
//        Spi_Error_Handler();
//    }
//
//    /*##-2- Wait for the end of the transfer ###################################*/
//    /*  Before starting a new communication transfer, you must wait the callback call
//        to get the transfer complete confirmation or an error detection.
//        For simplicity reasons, this example is just waiting till the end of the
//        transfer, but application may perform other tasks while transfer operation
//        is ongoing. */
//    while (wTransferState == TRANSFER_WAIT)
//    {
//    }
//
//    switch (wTransferState)
//    {
//        case TRANSFER_COMPLETE :
//            /*##-3- Compare the sent and received buffers ##############################*/
//            if (Buffercmp((uint8_t *)aTxBuffer, (uint8_t *)aRxBuffer, BUFFERSIZE))
//            {
//                /* Processing Error */
//                Spi_Error_Handler();
//            }
//            break;
//        default :
//            Spi_Error_Handler();
//            break;
//    }
}
auto main() -> int {
    HardwareInit();
    /** Initializes the peripherals clocks
     */
    RCC_Peripheral_Clock_Select();
    __HAL_RCC_SPI2_CLK_ENABLE();

//    xTaskCreateStatic(vTaskCode, "USART Task", stack.size(), nullptr, 1,
//                      stack.data(), &data);

    xTaskCreateStatic(spiTask, "SPI Task", stack.size(), nullptr, 1,
                      stack.data(), &data);
    vTaskStartScheduler();

    return 0;
}

static uint16_t Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint16_t BufferLength)
{
    while (BufferLength--)
    {
        if ((*pBuffer1) != *pBuffer2)
        {
            return BufferLength;
        }
        pBuffer1++;
        pBuffer2++;
    }

    return 0;
}