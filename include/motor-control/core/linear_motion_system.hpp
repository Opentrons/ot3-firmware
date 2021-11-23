#pragma once

#include <concepts>

namespace lms {

struct BeltConfig {
    float belt_pitch;          // mm/count
    float pulley_tooth_count;  // count/rev
    [[nodiscard]] constexpr auto get_mm_per_rev() const -> float {
        return belt_pitch * pulley_tooth_count;
    }
};

struct LeadScrewConfig {
    float lead_screw_pitch;  // mm/rev
    [[nodiscard]] constexpr auto get_mm_per_rev() const -> float {
        return lead_screw_pitch;
    }
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
    [[nodiscard]] constexpr auto get_steps_per_mm() const -> float {
        return (steps_per_rev * microstep) / (mech_config.get_mm_per_rev());
    }
};

}  // namespace lms
