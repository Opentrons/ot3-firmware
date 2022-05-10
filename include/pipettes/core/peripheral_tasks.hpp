#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_timer.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/firmware/i2c_comms.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

namespace peripheral_tasks {

using I2CClient =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;
using I2CPollerClient =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>;
using SPIClient =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;

void start_tasks(i2c::hardware::I2CDeviceBase& i2c3_device,
                 i2c::hardware::I2CDeviceBase& i2c1_device,
                 spi::hardware::SpiDeviceBase& spi_device);

/**
 * Access to all tasks pipette peripheral tasks. This will be a singleton.
 */
struct Tasks {
    i2c::tasks::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>*
        i2c3_task{nullptr};
    i2c::tasks::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>*
        i2c1_task{nullptr};
    i2c::tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                              freertos_timer::FreeRTOSTimer>* i2c3_poller_task{
        nullptr};
    i2c::tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                              freertos_timer::FreeRTOSTimer>* i2c1_poller_task{
        nullptr};
    spi::tasks::Task<freertos_message_queue::FreeRTOSMessageQueue>* spi_task{
        nullptr};
};

/**
 * Access to all the message queues in the system.
 */
struct QueueClient {
    QueueClient();

    freertos_message_queue::FreeRTOSMessageQueue<i2c::writer::TaskMessage>*
        i2c3_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::writer::TaskMessage>*
        i2c1_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::poller::TaskMessage>*
        i2c3_poller_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::poller::TaskMessage>*
        i2c1_poller_queue{nullptr};

    freertos_message_queue::FreeRTOSMessageQueue<spi::tasks::TaskMessage>*
        spi_queue{nullptr};
};

/**
 * Access to the gear motor tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> Tasks&;

/**
 * Access to the queues singleton
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;

auto get_i2c3_writer() -> I2CClient&;

auto get_i2c1_writer() -> I2CClient&;

auto get_i2c1_poller_client() -> I2CPollerClient&;

auto get_i2c3_poller_client() -> I2CPollerClient&;

auto get_spi_writer() -> SPIClient&;

}  // namespace peripheral_tasks