#pragma once
#include "common/core/freertos_message_queue.hpp"
#include "rear-panel/core/messages.hpp"

namespace queue_client {

/**
 * Access to all the message queues in the system.
 */
struct QueueClient {
    /*
     //TODO(ryan): CORETASKS compile in and process the other basic tasks

     void send_eeprom_queue(const eeprom::task::TaskMessage& m);

     freertos_message_queue::FreeRTOSMessageQueue<i2c::writer::TaskMessage>*
         i2c3_queue{nullptr};
     freertos_message_queue::FreeRTOSMessageQueue<i2c::poller::TaskMessage>*
         i2c3_poller_queue{nullptr};
     freertos_message_queue::FreeRTOSMessageQueue<eeprom::task::TaskMessage>*
         eeprom_queue{nullptr};
     */
    void send_system_queue(const rearpanel::messages::SystemTaskMessage& m);
    freertos_message_queue::FreeRTOSMessageQueue<
        rearpanel::messages::SystemTaskMessage>* system_queue{nullptr};

    void send_light_control_queue(
        const rearpanel::messages::LightControlTaskMessage& m);
    freertos_message_queue::FreeRTOSMessageQueue<
        rearpanel::messages::LightControlTaskMessage>* light_control_queue{
        nullptr};

    void send_host_comms_queue(
        const rearpanel::messages::HostCommTaskMessage& m);
    freertos_message_queue::FreeRTOSMessageQueue<
        rearpanel::messages::HostCommTaskMessage>* host_comms_queue{nullptr};
};

/**
 * Access to the queues singleton
 * @return
 */
[[nodiscard]] auto get_main_queues() -> QueueClient&;

}  // namespace queue_client
