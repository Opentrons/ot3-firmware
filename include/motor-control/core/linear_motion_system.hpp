#pragma once

#include <concepts>

namespace lms {

template <typename MC>
concept MotorMechanicalConfig = requires(MC mc) {
    {mc.mm_per_rev};
};

struct BeltConfig {
    constexpr BeltConfig(float pitch, float count)
        : belt_pitch(pitch), pulley_tooth_count(count) {}
    float belt_pitch;          // mm/count
    float pulley_tooth_count;  // count/rev
    const float mm_per_rev = belt_pitch * pulley_tooth_count;
};

struct LeadScrewConfig {
    constexpr LeadScrewConfig(float pitch) : lead_screw_pitch(pitch) {}
    float lead_screw_pitch;  // mm/rev
    const float mm_per_rev = lead_screw_pitch;
};

template <MotorMechanicalConfig MEConfig>
class LinearMotionSystemConfig {
  public:
    explicit LinearMotionSystemConfig(MEConfig& mech_config,
                                      float steps_per_rev, float microstep)
        : mech_config(mech_config),
          steps_per_rev(steps_per_rev),
          microstep(microstep) {}

    float get_steps_per_mm() {
        return (steps_per_rev * microstep) / (mech_config.mm_per_rev);
    }

    MEConfig& mech_config;
    float steps_per_rev;
    float microstep;
};

template <class Config>
concept LMSConfig = requires(Config config) {
    {config.get_steps_per_mm()};
};

}  // namespace lms
