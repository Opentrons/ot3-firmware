#pragma once

namespace spi_messages {
    using CallbackTypeDef = std::function<void(const spi::SpiDeviceBase::BufferType &)>;

    struct TaskMessage = {
        spi::SpiDeviceBase::BufferType txData;
        spi::SpiDeviceBase::BufferType rxData;
        CallbackTypeDef handle_callback;
        int _id;
    };


    template <template <class> class QueueImpl>
    requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
    struct SpiTransact {
        SpiTransact(QueueType& queue) : queue(queue) {}

        auto handle_spi_response() -> void {

        }
    };
}