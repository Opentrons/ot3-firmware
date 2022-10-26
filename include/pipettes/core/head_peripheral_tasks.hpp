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

/**
 * Peripheral Tasks
 *
 * These tasks will start the three main peripherals used on the
 * pipette board -- SPI2, I2C1 and I2C3. None of the tasks in this file
 * should communicate with CAN tasks directly.
 */
namespace head_peripheral_tasks {

using I2CClient =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;
using I2CPollerClient =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>;
using SPIClient =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;

void start_tasks(i2c::hardware::I2CBase& i2c3_interface,
                 spi::hardware::SpiDeviceBase& spi2_device,
                 spi::hardware::SpiDeviceBase& spi3_device);

/**
 * Access to all the pipette peripheral tasks. This will be a singleton.
 */
struct Tasks {
    i2c::tasks::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>*
        i2c3_task{nullptr};
    i2c::tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                              freertos_timer::FreeRTOSTimer>* i2c3_poller_task{
        nullptr};
    spi::tasks::Task<freertos_message_queue::FreeRTOSMessageQueue>* spi2_task{
        nullptr};
    spi::tasks::Task<freertos_message_queue::FreeRTOSMessageQueue>* spi3_task{
        nullptr};
};

/**
 * Access to all the pipette peripheral task queues.
 *
 * Since these tasks do not communicate with the CAN client at all, we
 * don't need to inherit from it.
 */
struct QueueClient {
    QueueClient();

    freertos_message_queue::FreeRTOSMessageQueue<i2c::writer::TaskMessage>*
        i2c3_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::poller::TaskMessage>*
        i2c3_poller_queue{nullptr};

    freertos_message_queue::FreeRTOSMessageQueue<spi::tasks::TaskMessage>*
        spi2_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<spi::tasks::TaskMessage>*
        spi3_queue{nullptr};
};

/**
 * Access to the peripheral tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> Tasks&;

/**
 * Access to the peripheral tasks queues singleton
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;

/**
 * Access to i2c3 client global defined in the cpp file.
 * @return
 */
auto get_i2c3_client() -> I2CClient&;

/**
 * Access to i2c3 poller client global defined in the cpp file.
 * @return
 */
auto get_i2c3_poller_client() -> I2CPollerClient&;

/**
 * Access to spi client global defined in the cpp file.
 * @return
 */
auto get_spi3_client() -> SPIClient&;

/**
 * Access to spi client global defined in the cpp file.
 * @return
 */
auto get_spi2_client() -> SPIClient&;

}  // namespace head_peripheral_tasks