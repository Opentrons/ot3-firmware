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
    float lead_screw_pitch;      // mm/rev
    float gear_reduction_ratio;  // large teeth / small teeth
    [[nodiscard]] constexpr auto get_mm_per_rev() const -> float {
        return lead_screw_pitch / gear_reduction_ratio;
    }
};

struct GearBoxConfig {
    float gear_diameter;         // mm
    float gear_reduction_ratio;  // large teeth / small teeth
    [[nodiscard]] constexpr auto get_mm_per_rev() const -> float {
        return static_cast<float>((gear_diameter * std::numbers::pi) /
                                  gear_reduction_ratio);
    }
};

template <typename MC>
concept MotorMechanicalConfig = requires {
    std::is_same_v<MC, BeltConfig> || std::is_same_v<MC, LeadScrewConfig> ||
        std::is_same_v<MC, GearBoxConfig>;
};

template <MotorMechanicalConfig MEConfig>
struct LinearMotionSystemConfig {
    MEConfig mech_config{};
    float steps_per_rev{};
    float microstep{};
    float encoder_pulses_per_rev{0.0};
    [[nodiscard]] constexpr auto get_usteps_per_mm() const -> float {
        return (steps_per_rev * microstep) / (mech_config.get_mm_per_rev());
    }
    [[nodiscard]] constexpr auto get_usteps_per_um() const -> float {
        return (steps_per_rev * microstep) /
               (mech_config.get_mm_per_rev() * 1000.0);
    }
    [[nodiscard]] constexpr auto get_um_per_step() const -> float {
        return (mech_config.get_mm_per_rev()) / (steps_per_rev * microstep) *
               1000;
    }
    [[nodiscard]] constexpr auto get_encoder_pulses_per_mm() const -> float {
        return (encoder_pulses_per_rev * 4.0) / (mech_config.get_mm_per_rev());
    }
    [[nodiscard]] constexpr auto get_encoder_um_per_pulse() const -> float {
        if (encoder_pulses_per_rev == 0.0) {
            return 0.0;
        }
        return (mech_config.get_mm_per_rev()) / (encoder_pulses_per_rev * 4.0) *
               1000.0;
    }
};

}  // namespace lms
