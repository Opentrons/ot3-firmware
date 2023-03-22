#pragma once

#include <array>
#include <cstdint>
#include <optional>

#include "rear-panel/core/bin_msg_ids.hpp"
#include "rear-panel/core/double_buffer.hpp"

namespace lights {

using Transition = rearpanel::ids::LightTransitionType;
using AnimationType = rearpanel::ids::LightAnimationType;

struct Color {
    // Colors are expressed as a range of [0, 1.0]
    double r = 0, g = 0, b = 0, w = 0;
};

struct Action {
    Color color;
    Transition transition;
    uint32_t transition_time_ms;
};

inline bool operator==(const Color &lhs, const Color &rhs) {
    return (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b) &&
           (lhs.w == rhs.w);
}

inline bool operator==(const Action &lhs, const Action &rhs) {
    return (lhs.color == rhs.color) && (lhs.transition == rhs.transition) &&
           (lhs.transition_time_ms == rhs.transition_time_ms);
}

/**
 * @brief The AnimationQueue class is used to arrange light actions in an
 * ordered sequence to generate animations. Each Action is defined by an
 * RGBW color, a transition time, and a transition type.
 *
 * The AnimationQueue holds both an active queue and a staging queue. New
 * actions are added to the staging queue linearly, and then when a new
 * animation is started the staging queue is swapped with the active queue.
 * This allows a new animation to be constructed in the background while the
 * current animation continues running.
 *
 * @tparam Size The maximum number of elements in the queue. Must be nonzero.
 */
template <size_t Size>
class AnimationQueue {
  private:
    static_assert(Size > 0, "AnimationQueue requires nonzero buffer size.");

  public:
    AnimationQueue()
        : _actions{},
          _active_idx(0),
          _staging_idx(0),
          _animation(AnimationType::single_shot) {}

    auto get_next_active_step() -> std::optional<Action> {
        if ((!get_active_idx(_active_idx).has_value()) &&
            _animation == AnimationType::looping) {
            _active_idx = 0;
        }

        return get_active_idx(_active_idx++);
    }

    /**
     * @brief Start a new animation. This function swaps the staging queue to
     * be the active queue and clears out the new staging queue.
     *
     * @param animation The type of animation.
     */
    auto start_staged_animation(AnimationType animation) -> void {
        // Before swapping, cap off the staging queue with a nullopt to
        // ensure it won't overrun.
        if (_staging_idx < Size) {
            _actions.committed()->at(0) = std::nullopt;
        }
        _actions.swap();
        _active_idx = 0;
        _animation = animation;
        clear_staging();
    }

    /**
     * @brief Add a step to the staging queue
     *
     * @param action The action to add
     * @return true if the action was added, false if the queue is full.
     */
    auto add_to_staging(Action action) -> bool {
        if (_staging_idx >= Size) {
            return false;
        }
        _actions.accessible()->at(_staging_idx++) = action;
        return true;
    }

    /**
     * @brief Empty out the staging queue.
     *
     */
    auto clear_staging() -> void {
        _staging_idx = 0;
        _actions.accessible()->at(0) = std::nullopt;
    }

  private:
    auto get_active_idx(size_t idx) -> std::optional<Action> {
        if (idx >= Size) {
            return std::nullopt;
        }
        return _actions.committed()->at(idx);
    }

    double_buffer::DoubleBuffer<std::optional<Action>, Size> _actions;
    size_t _active_idx, _staging_idx;
    AnimationType _animation;  // Current active animation type
};

}  // namespace lights