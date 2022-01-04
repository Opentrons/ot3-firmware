#pragma once

#include <vector>

#include "motor-control/core/tasks/move_status_reporter_task.hpp"

namespace test_mocks {

struct MockMoveStatusReporterClient {
    void send_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m) {
        messages.push_back(m);
    }

    std::vector<move_status_reporter_task::TaskMessage> messages{};
};

}  // namespace test_mocks