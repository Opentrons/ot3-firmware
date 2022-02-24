#pragma once

namespace sensor_callbacks {
using MaxMessageBuffer = std::array<uint8_t, 5>;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class SingleRegisterCallback {
  public:
    virtual void operator()(const MaxMessageBuffer &buffer) = 0;
    virtual void operator()() = 0;
    virtual ~SingleRegisterCallback() = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class MultiRegisterCallback {
  public:
    virtual auto operator()(const MaxMessageBuffer &buffer_a,
                            const MaxMessageBuffer &buffer_b) -> void = 0;
    virtual auto operator()() -> void = 0;
    virtual ~MultiRegisterCallback() = default;
};

}  // namespace sensor_callbacks
