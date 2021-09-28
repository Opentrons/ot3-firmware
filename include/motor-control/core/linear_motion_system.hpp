#pragma once

#include <concepts>

namespace lms {

struct BeltConfig {
    float belt_pitch;          // mm/count
    float pulley_tooth_count;  // count/rev
    constexpr float get_mm_per_rev() const {
        return belt_pitch * pulley_tooth_count;
    }
};

struct LeadScrewConfig {
    float lead_screw_pitch;  // mm/rev
    constexpr float get_mm_per_rev() const { return lead_screw_pitch; }
};

template <typename MC>
concept MotorMechanicalConfig = requires {
    std::is_same_v<MC, BeltConfig> || std::is_same_v<MC, LeadScrewConfig>;
};

template <MotorMechanicalConfig MEConfig>
struct LinearMotionSystemConfig {
    MEConfig mech_config;
    float steps_per_rev;
    float microstep;
    constexpr float get_steps_per_mm() const {
        return (steps_per_rev * microstep) / (mech_config.get_mm_per_rev());
    }
};

}  // namespace lms
