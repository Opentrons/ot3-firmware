#pragma once

#include "common/core/message_queue.hpp"
#include "ot_utils/freertos/freertos_timer.hpp"
#include "rear-panel/core/messages.hpp"

namespace light_control_task {
namespace timer {

using UpdateMessage = rearpanel::messages::UpdateLightControlMessage;
using TaskMessage = rearpanel::messages::LightControlTaskMessage;

template <class Queue>
requires MessageQueue<Queue, TaskMessage>
class LightControlTimer {
  public:
    LightControlTimer(Queue& queue, int period_ms)
        : _queue{queue},
          _timer(
              "LightTask", [ThisPtr = this] { ThisPtr->timer_callback(); },
              period_ms) {}

    auto start() -> void { _timer.start(); }

    auto stop() -> void { _timer.stop(); }

  private:
    auto timer_callback() -> void { _queue.try_write(UpdateMessage()); }

    Queue& _queue;
    ot_utils::freertos_timer::FreeRTOSTimer _timer;
};

}  // namespace timer
}  // namespace light_control_task