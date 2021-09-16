#pragma once

#include <concepts>

template <typename MC>
concept MotorMechanicalConfig = requires(MC mc) {
    {mc.mm_per_rev};
};

struct BeltConfig {
    constexpr BeltConfig(float pitch, float count)
        : belt_pitch(pitch), pulley_tooth_count(count) {}
    float belt_pitch;          // mm per count
    float pulley_tooth_count;  // count per rev
    const float mm_per_rev = belt_pitch * pulley_tooth_count;
};

struct LeadScrewConfig {
    constexpr LeadScrewConfig(float pitch) : lead_screw_pitch(pitch) {}
    float lead_screw_pitch;  // mm per rev
    const float mm_per_rev = lead_screw_pitch;
};

template <MotorMechanicalConfig MEConfig>
class LinearSystemConfig {
  public:
    explicit LinearSystemConfig(MEConfig& mech_config, float steps_per_rev,
                                float microstep)
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
concept LinearMechanicalConfig = requires(Config config) {
    {config.get_steps_per_mm()};
};