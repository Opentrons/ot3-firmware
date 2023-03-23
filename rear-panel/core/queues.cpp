#include "rear-panel/core/queues.hpp"

static auto queues = queue_client::QueueClient{};

// TODO(ryan): CORETASKS compile in and process the other basic tasks
/*
void queue_client::QueueClient::send_eeprom_queue(
    const eeprom::task::TaskMessage& m) {
    eeprom_queue->try_write(m);
}
*/

void queue_client::QueueClient::send_host_comms_queue(
    const rearpanel::messages::HostCommTaskMessage& m) {
    if (host_comms_queue) {
        host_comms_queue->try_write(m);
    }
}

void queue_client::QueueClient::send_system_queue(
    const rearpanel::messages::SystemTaskMessage& m) {
    if (system_queue) {
        system_queue->try_write(m);
    }
}

void queue_client::QueueClient::send_light_control_queue(
    const rearpanel::messages::LightControlTaskMessage& m) {
    if (light_control_queue) {
        light_control_queue->try_write(m);
    }
}

void queue_client::QueueClient::send_hardware_queue(
    const rearpanel::messages::HardwareTaskMessage& m) {
    if (hardware_queue) {
        hardware_queue->try_write(m);
    }
}

/**
 * Access to the queues singleton
 * @return
 */
auto queue_client::get_main_queues() -> QueueClient& { return queues; }
