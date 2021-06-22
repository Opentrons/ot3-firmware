/*
** Defines the tasks struct that holds pointers to all the different tasks in
*the system
*/

#pragma once

#include "common/core/message_queue.hpp"
#include "host_comms_task.hpp"
#include "messages.hpp"

namespace host_comms_task {
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<messages::HostCommsMessage>,
                      messages::HostCommsMessage>
class HostCommsTask;
}  // namespace host_comms_task

namespace tasks {
/* Container relating the RTOSTask for the implementation and the portable task
 * object */
template <typename RTOSHandle, class PortableTask>
struct Task {
    RTOSHandle handle;
    PortableTask& task;
};

/* Aggregator for initialized task objects that can be injected into those
 objects after they are created */
template <template <class> class QueueImpl>
struct Tasks {
    host_comms_task::HostCommsTask<QueueImpl>& comms;
};
}  // namespace tasks
