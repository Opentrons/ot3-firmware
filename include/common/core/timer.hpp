#pragma once

#include <functional>
/**
 * Concept and useful definitions for generic timers.
 * */
namespace timer {
using Callback = std::function<void()>;

template <typename TimerType>
concept Timer = requires(TimerType tt, Callback cb) {
    // Query that the timer is running
    { tt.is_running() } -> std::same_as<bool>;
    {tt.update_callback(std::move(cb))};
    {tt.update_period(static_cast<uint32_t>(10))};
    {tt.start()};
    {tt.stop()};
    {TimerType("hello", std::move(cb), static_cast<uint32_t>(0))};
};
};  // namespace timer
