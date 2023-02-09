#pragma once
#include <atomic>

namespace debouncer {

struct Debouncer {
    std::atomic_bool state = false;
    std::atomic_bool state_bounce = false;
    auto debounce_update(bool new_state) -> void {
        // only set the state if the bounce matches the current gpio_is_set
        // on the first state change it won't match but on the second tick it
        // will and we can set it to the new state.
        if (new_state == state_bounce) {
            state = new_state;
        }
        state_bounce = new_state;
    }
    auto debounce_state() -> bool { return state.load(); }
};

}  // namespace debouncer
