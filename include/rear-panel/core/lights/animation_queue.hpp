#pragma once

#include <array>
#include <cstdint>
#include <optional>

#include "rear-panel/core/bin_msg_ids.hpp"

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

inline auto operator==(const Color &lhs, const Color &rhs) -> bool {
    return (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b) &&
           (lhs.w == rhs.w);
}

inline auto operator==(const Action &lhs, const Action &rhs) -> bool {
    return (lhs.color == rhs.color) && (lhs.transition == rhs.transition) &&
           (lhs.transition_time_ms == rhs.transition_time_ms);
}

template <size_t Size>
class AnimationQueue {
  private:
    static_assert(Size > 0, "AnimationQueue requires nonzero buffer size.");

  public:
    AnimationQueue() = default;

    auto start_animation(AnimationType type) -> void {
        _active_idx = 0;
        _animation = type;
    }

    auto get_next() -> std::optional<Action> {
        if (_active_idx >= _length && _animation == AnimationType::looping) {
            _active_idx = 0;
        }
        if (_active_idx >= _length) {
            return std::nullopt;
        }
        return _queue.at(_active_idx++);
    }

    auto add_step(const Action &action) -> bool {
        if (_length >= Size) {
            return false;
        }
        _queue.at(_length++) = action;
        return true;
    }

    auto clear() -> void {
        _length = 0;
        _active_idx = 0;
    }

  private:
    std::array<Action, Size> _queue{};
    size_t _length{0};
    size_t _active_idx{0};
    AnimationType _animation{AnimationType::single_shot};
};

/**
 * @brief The AnimationBuffer class is used to arrange light actions in an
 * ordered sequence to generate animations. Each Action is defined by an
 * RGBW color, a transition time, and a transition type.
 *
 * The AnimationBuffer holds both an active queue and a staging queue. New
 * actions are added to the staging queue linearly, and then when a new
 * animation is started the staging queue is swapped with the active queue.
 * This allows a new animation to be constructed in the background while the
 * current animation continues running.
 *
 * @tparam Size The maximum number of elements in the queue. Must be nonzero.
 */
template <size_t Size>
class AnimationBuffer {
  public:
    AnimationBuffer() : _a(), _b(), _active(&_a), _staging(&_b) {}

    auto get_next_active_step() -> std::optional<Action> {
        return _active->get_next();
    }

    /**
     * @brief Start a new animation. This function swaps the staging queue to
     * be the active queue and clears out the new staging queue.
     *
     * @param animation The type of animation.
     */
    auto start_staged_animation(AnimationType animation) -> void {
        // FIRST swap staging & active handles
        auto *tmp = _active;
        _active = _staging;
        _staging = tmp;
        // Start the new active queue, and clear the new staging queue
        _active->start_animation(animation);
        clear_staging();
    }

    /**
     * @brief Add a step to the staging queue
     *
     * @param action The action to add
     * @return true if the action was added, false if the queue is full.
     */
    auto add_to_staging(Action action) -> bool {
        return _staging->add_step(action);
    }

    /**
     * @brief Empty out the staging queue.
     *
     */
    auto clear_staging() -> void { _staging->clear(); }

  private:
    AnimationQueue<Size> _a, _b;
    AnimationQueue<Size> *_active, *_staging;
};

}  // namespace lights