#pragma once

#include <vector>

#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_message_writer.hpp"
#include "pipettes/core/tasks/eeprom_task.hpp"

namespace mock_client {

/**
 * Access to all the message queues in the system.
 */
struct QueueClient
    : mock_message_writer::MockMessageWriter<test_mocks::MockMessageQueue> {
    test_mocks::MockMessageQueue<eeprom_task::TaskMessage>* eeprom_queue;

    void send_eeprom_queue(const eeprom_task::TaskMessage& m) {
        eeprom_queue->try_write(m);
    }
};
}  // namespace mock_client