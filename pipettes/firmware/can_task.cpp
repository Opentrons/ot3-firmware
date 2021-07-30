#include "common/firmware/can_task.hpp"

#include <array>

#include "FreeRTOS.h"
#include "can/core/message_writer.hpp"
#include "common/core/bit_utils.hpp"
#include "can/core/messages.hpp"
#include "can/core/parse.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "common/firmware/can.h"
#include "task.h"
#include "message_buffer.h"



// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static MessageBufferHandle_t message_buffer;
static constexpr auto message_buffer_size = 1024;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static auto message_buffer_backing = std::array<uint8_t, message_buffer_size>{};
static StaticMessageBuffer_t message_buffer_struct;


static void Error_Handler();

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static FDCAN_RxHeaderTypeDef RxHeader;
static constexpr auto buffsize = 68;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static auto buff = std::array<uint8_t, buffsize>{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern FDCAN_HandleTypeDef fdcan1;
//
// template <typename... MessageTypes>
// class MessageHandler {
//  public:
//    explicit MessageHandler() {}
//
//    void handle(FDCAN_HandleTypeDef *can_handle, can_ids::MessageId
//    message_id,
//                can_messages::BodyType &body) {
//        auto message = parser.parse(message_id, body);
//        auto handle = [this, can_handle](auto m) {
//            this->handle(m, can_handle);
//        };
//        std::visit(handle, message);
//    }
//
//  private:
//    void handle(const can_messages::MoveRequest &m,
//                FDCAN_HandleTypeDef *can_handle) {
//        static_cast<void>(m);
//    }
//
//    void handle(const can_messages::SetSpeedRequest &m,
//                FDCAN_HandleTypeDef *can_handle) {
//        speed = m.mm_sec;
//    }
//
//    void handle(const can_messages::SetupRequest &m,
//                FDCAN_HandleTypeDef *can_handle) {}
//
//    void handle(const can_messages::GetStatusRequest &m,
//                FDCAN_HandleTypeDef *can_handle) {
//        TxHeader.Identifier = static_cast<uint32_t>(m.id);
//        TxHeader.IdType = FDCAN_EXTENDED_ID;
//        TxHeader.TxFrameType = FDCAN_DATA_FRAME;
//        TxHeader.DataLength = FDCAN_DLC_BYTES_5;
//        TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
//        TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
//        TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
//        TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
//        TxHeader.MessageMarker = 0;
//
//        auto response = can_messages::GetStatusResponse{1, 2};
//
//        auto sp = BodyType(buff);
//        response.serialize(sp);
//
//        HAL_FDCAN_AddMessageToTxFifoQ(&can, &TxHeader, buff.data());
//    }
//
//    void handle(const can_messages::GetSpeedRequest &m,
//                FDCAN_HandleTypeDef *can_handle) {
//        TxHeader.Identifier = static_cast<uint32_t>(m.id);
//        TxHeader.IdType = FDCAN_EXTENDED_ID;
//        TxHeader.TxFrameType = FDCAN_DATA_FRAME;
//        TxHeader.DataLength = FDCAN_DLC_BYTES_4;
//        TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
//        TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
//        TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
//        TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
//        TxHeader.MessageMarker = 0;
//
//        auto response = can_messages::GetSpeedResponse{speed};
//
//        auto sp = BodyType(buff);
//        response.serialize(sp);
//
//        HAL_FDCAN_AddMessageToTxFifoQ(&can, &TxHeader, buff.data());
//    }
//
//    void handle(const std::monostate &m, FDCAN_HandleTypeDef *can_handle) {
//        static_cast<void>(m);
//    }
//
//    can_parse::Parser<MessageTypes...> parser;
//
//    uint32_t speed{0};
//};

// static auto message_handler =
//    MessageHandler<can_messages::MoveRequest, can_messages::SetSpeedRequest,
//                   can_messages::SetupRequest, can_messages::GetStatusRequest,
//                   can_messages::GetSpeedRequest>{};



/**
 *
 * @param hfdcan pointer to a can handle
 * @param fifo either FDCAN_RX_FIFO0 or FDCAN_RX_FIFO1
 */
static void try_read_from_fifo(FDCAN_HandleTypeDef* hfdcan, uint32_t fifo) {
    if (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, fifo) > 0) {
        // Put the message data after the arbitration id
        auto start_of_data = buff.data() + sizeof(uint32_t);

        if (HAL_FDCAN_GetRxMessage(hfdcan, fifo, &RxHeader, start_of_data) != HAL_OK) {
            Error_Handler();
        }

        // Convert the length from HAL to integer
        auto data_length = static_cast<uint32_t>(HalCanBus::convert_length(RxHeader.DataLength));
        auto message_length = data_length + sizeof(data_length);

        // Write the arbitration id into the buffer
        bit_utils::int_to_bytes(RxHeader.Identifier, buff.begin(), start_of_data);

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        auto written_bytes = xMessageBufferSendFromISR(message_buffer, buff.data(), message_length, &xHigherPriorityTaskWoken);

        if (written_bytes == message_length) {
            // see https://www.freertos.org/xMessageBufferSendFromISR.html for explanation
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}


void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo0ITs) {
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0) {
        try_read_from_fifo(hfdcan, FDCAN_RX_FIFO0);
    }
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo1ITs) {
    if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != 0) {
        try_read_from_fifo(hfdcan, FDCAN_RX_FIFO1);
    }
}
//
//static auto message = can_messages::GetSpeedResponse{1234};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static auto otbuff = std::array<uint8_t, buffsize>{};


static void run(void *parameter) {
    parameter = nullptr;

    message_buffer = xMessageBufferCreateStatic(
        message_buffer_size,message_buffer_backing.data(),&message_buffer_struct );

    if (MX_FDCAN1_Init(&fdcan1) != HAL_OK) {
        Error_Handler();
    }

//    auto writer = can_message_writer::MessageWriter(cc);

    if (HAL_FDCAN_ActivateNotification(&fdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, // | FDCAN_IT_RX_FIFO1_NEW_MESSAGE,
                                       0) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_FDCAN_Start(&fdcan1) != HAL_OK) {
        Error_Handler();
    }

    const auto xBlockTime = portMAX_DELAY;


    for (;;) {

        auto read_bytes = xMessageBufferReceive(message_buffer,
                                               otbuff.data(),
                                               otbuff.size(),
                                               xBlockTime);
        if (read_bytes > 0) {
            uint32_t arb = 0;
            auto p = bit_utils::bytes_to_int(otbuff.begin(), otbuff.end() + read_bytes, arb);
            auto x = p;
        }

        //        if (HAL_FDCAN_GetRxFifoFillLevel(&can, FDCAN_RX_FIFO0) > 0) {
        //            // Send CAN RX data over debug UART
        //            HAL_FDCAN_GetRxMessage(&can, FDCAN_RX_FIFO0, &RxHeader,
        //                                   buff.data());
        //
        //            BodyType bod{buff};
        //            message_handler.handle(&can,
        //            static_cast<can_ids::MessageId>(RxHeader.Identifier),
        //            bod);
        //
        //            TxHeader.Identifier = RxHeader.Identifier;
        //            TxHeader.DataLength = RxHeader.DataLength;
        //            TxHeader.FDFormat = RxHeader.FDFormat;
        //
        //            if (HAL_FDCAN_AddMessageToTxFifoQ(&can, &TxHeader,
        //            buff.data()) !=
        //                HAL_OK) {
        //                // Transmission request Error
        //                Error_Handler();
        //            }
        //        }
//        writer.write(NodeId::host, message);
//        vTaskDelay(1000);
    }
}

static constexpr auto stack_size = 300;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::array<StackType_t, stack_size> stack;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static StaticTask_t data{};

/**
 * Create the task.
 */
void can_task::start() {
    xTaskCreateStatic(run, "CAN Task", stack.size(), nullptr, 1, stack.data(), &data);
}

void Error_Handler() {
    for (;;) {
        HAL_GPIO_TogglePin(
            GPIOA,         // NOLINTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            GPIO_PIN_5);   // NOLINTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        vTaskDelay(1000);  // NOLINTLINE(cppcoreguidelines-avoid-magic-numbers)
    }
}