#pragma once

#include <functional>
#include <string>

#include "common/core/logging.h"

namespace test_mocks {
class MockTimer {
  public:
    using Callback = std::function<void()>;
    MockTimer(const char* name, Callback&& callback, uint32_t period_ms)
        : name(name), callback{std::move(callback)}, period_ms(period_ms) {}
    auto operator=(MockTimer&) -> MockTimer& = delete;
    auto operator=(MockTimer&&) -> MockTimer&& = delete;
    MockTimer(MockTimer&) = delete;
    MockTimer(MockTimer&&) = delete;
    ~MockTimer() {}

    /** test harness interface
     * */
    const std::string& get_name() const { return name; }
    uint32_t get_period() const { return period_ms; }
    void fire() { callback(); }

    /*
    ** mock interface
    */
    bool is_running() { return running; }

    void update_callback(Callback&& new_callback) {
        callback = std::move(new_callback);
    }

    void update_period(uint32_t period) { period_ms = period; }

    void start() { running = true; }

    void stop() { running = false; }

  private:
    std::string name;
    Callback callback;
    uint32_t period_ms;
    bool running = false;
};

};  // namespace test_mocks
