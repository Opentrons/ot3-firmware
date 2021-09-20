#pragma <once>

#include "common/core/freertos_message_queue.hpp"

using namespace freertos_message_queue;

template<FreeRTOSGenericQueue GenericQueue>
class MotorInterruptHandler {
  public:
    explicit MotorInterruptHandler(GenericQueue queue) : queue(queue) {}

    MoveMessage* active_move() {
        return queue.peek();
    }

    bool space_available() {
        return queue.is_full_isr();
    }

    void finish_move() {
        queue.pop();
        is_done = true;
    }

    void reset() {
        queue.reset();
        inc = 0x0;
        steps = 0x0;
        is_done = true;
    }


  private:
    uint32_t inc = 0;
    uint32_t steps;
    bool is_done = false;
    GenericQueue queue;
    bool should_step() {
        return inc < steps;
    }
};