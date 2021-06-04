#pragma once
#include <memory>
#include <utility>

#include "firmware/host_comms_task.hpp"
#include "firmware/tasks.hpp"
#include "test/test_message_queue.hpp"

struct TaskBuilder {
    ~TaskBuilder() = default;

    static auto build() -> std::shared_ptr<TaskBuilder>;

    // Instances of this struct should only live in smart pointers and not
    // be passed around by-value
    TaskBuilder(const TaskBuilder&) = delete;
    auto operator=(const TaskBuilder&) -> TaskBuilder& = delete;
    TaskBuilder(TaskBuilder&&) noexcept = delete;
    auto operator=(TaskBuilder&&) noexcept -> TaskBuilder& = delete;

    auto get_host_comms_queue() -> TestMessageQueue<host_comms_task::Message>& {
        return host_comms_queue;
    }
    auto get_host_comms_task()
        -> host_comms_task::HostCommsTask<TestMessageQueue>& {
        return host_comms_task;
    }
    auto get_tasks_aggregator() -> tasks::Tasks<TestMessageQueue>& {
        return task_aggregator;
    }

  private:
    TaskBuilder();
    TestMessageQueue<host_comms_task::Message> host_comms_queue;
    host_comms_task::HostCommsTask<TestMessageQueue> host_comms_task;
    tasks::Tasks<TestMessageQueue> task_aggregator;
};
