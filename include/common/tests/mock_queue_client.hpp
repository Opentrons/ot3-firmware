#pragma once

#include <vector>

#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_message_writer.hpp"
#include "eeprom/core/task.hpp"
#include "sensors/core/utils.hpp"

namespace mock_client {

/**
 * Access to all the message queues in the system.
 */
struct QueueClient
    : mock_message_writer::MockMessageWriter<test_mocks::MockMessageQueue> {
    test_mocks::MockMessageQueue<eeprom::task::TaskMessage>* eeprom_queue;
    test_mocks::MockMessageQueue<sensors::utils::TaskMessage>*
        environment_sensor_queue;
    test_mocks::MockMessageQueue<sensors::utils::TaskMessage>*
        capacitive_sensor_queue;
    test_mocks::MockMessageQueue<sensors::utils::TaskMessage>*
        pressure_sensor_queue;

    void send_eeprom_queue(const eeprom::task::TaskMessage& m) {
        eeprom_queue->try_write(m);
    }

    void send_environment_sensor_queue(const sensors::utils::TaskMessage& m) {
        environment_sensor_queue->try_write(m);
    }

    void send_capacitive_sensor_queue(const sensors::utils::TaskMessage& m) {
        capacitive_sensor_queue->try_write(m);
    }

    void send_pressure_sensor_queue(const sensors::utils::TaskMessage& m) {
        pressure_sensor_queue->try_write(m);
    }
};
}  // namespace mock_client
