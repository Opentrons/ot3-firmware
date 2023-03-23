#pragma once

#include <algorithm>
#include <optional>

#include "rear-panel/core/lights/animation_math.hpp"
#include "rear-panel/core/lights/animation_queue.hpp"

namespace lights {

/**
 * @brief Acts as an interface for an AnimationBuffer.
 *
 * @note This class if NOT thread safe - it accesses the same queue in
 * multiple functions and, if they are called from different threads,
 * race conditions may arise. If this class is accessed from different
 * threads, wrap access to all member functions with a mutex.
 *
 */
template <size_t Size>
class AnimationHandler {
  public:
    AnimationHandler()
        : _step_starting_color{.r = 0, .g = 0, .b = 0, .w = 0},
          _most_recent_color{.r = 0, .g = 0, .b = 0, .w = 0},
          _current_step{std::nullopt},
          _timer_ms(0),
          _queue() {}

    /**
     * @brief Advance the animation and get the next color setting.
     *
     * @param time_increment_ms The amount of time that has passed, in
     * milliseconds.
     * @return Color
     */
    auto animate(uint32_t time_increment_ms) -> Color {
        Color ret = _step_starting_color;
        if (_current_step.has_value()) {
            // Get the power level for the current timer
            ret = lights::math::color_interpolate(
                _step_starting_color, _current_step.value().color,
                _current_step.value().transition, _timer_ms,
                _current_step.value().transition_time_ms);
            // We are in a step, calculate the next increment
            _timer_ms += time_increment_ms;
            // Check if this step is over
            if (_timer_ms >= _current_step.value().transition_time_ms) {
                _step_starting_color = _current_step.value().color;
                // Signal that we need a new message next time through
                _current_step = std::nullopt;
            }
        } else {
            // We were not in an active animation last update, but
            // one may have started since then. Try to get a new step.
            //
            // Because the color is set to _step_starting_color, which
            // is the *ending* color of the previous step, we are implicitly
            // doing the time=0 step here.
            _timer_ms = time_increment_ms;
            _current_step = _queue.get_next_active_step();
        }
        _most_recent_color = ret;
        return ret;
    }

    auto start_staged_animation(AnimationType animation) -> void {
        _timer_ms = 0;
        _current_step = std::nullopt;
        // If we were in the middle of animating a step, we cache the most
        // recent setting to be the "starting" value for the step that's about
        // to start in order to eliminate any possible jitter.
        _step_starting_color = _most_recent_color;
        _queue.start_staged_animation(animation);
    }

    auto add_to_staging(const Action &action) -> bool {
        return _queue.add_to_staging(action);
    }

    auto clear_staging() -> void { _queue.clear_staging(); }

  private:
    Color _step_starting_color;
    Color _most_recent_color;
    std::optional<Action> _current_step;
    uint32_t _timer_ms;
    AnimationBuffer<Size> _queue;
};

}  // namespace lights