#pragma once

#include <concepts>
#include <numbers>

namespace lms {

struct BeltConfig {
    float pulley_diameter;  // mm
    [[nodiscard]] constexpr auto get_mm_per_rev() const -> float {
        return static_cast<float>(pulley_diameter * std::numbers::pi);
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
    MEConfig mech_config{};
    float steps_per_rev{};
    float microstep{};
    float encoder_ppr{0.0};
    float gear_ratio{1.0};
    [[nodiscard]] constexpr auto get_steps_per_mm() const -> float {
        return (steps_per_rev * microstep * gear_ratio) /
               (mech_config.get_mm_per_rev());
    }
    [[nodiscard]] constexpr auto get_um_per_step() const -> float {
        return (mech_config.get_mm_per_rev()) /
               (steps_per_rev * microstep * gear_ratio) * 1000;
    }
    [[nodiscard]] constexpr auto get_encoder_pulses_per_mm() const -> float {
        return (encoder_ppr * 4.0 * gear_ratio) /
               (mech_config.get_mm_per_rev());
    }
    [[nodiscard]] constexpr auto get_encoder_um_per_pulse() const -> float {
        return (mech_config.get_mm_per_rev()) /
                (encoder_ppr * 4.0 * gear_ratio) * 1000.0;
    }
};

}  // namespace lms
