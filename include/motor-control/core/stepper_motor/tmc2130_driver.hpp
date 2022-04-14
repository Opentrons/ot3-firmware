/**
 * @file tmc2130.hpp
 * @brief Interface to control a TMC2130 IC
 */
#pragma once

#include <cmath>
#include <concepts>
#include <cstdint>
#include <functional>
#include <numbers>
#include <optional>

#include "common/core/bit_utils.hpp"
#include "spi/core/utils.hpp"
#include "spi/core/writer.hpp"
#include "tmc2130.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

namespace tmc2130 {

namespace driver {

using namespace std::numbers;
using namespace spi::writer;
using namespace tmc2130::registers;
using namespace tmc2130::configs;

template <class Writer, class TaskQueue>
class TMC2130 {
  public:
    TMC2130(Writer& spi_manager, TaskQueue& task_queue,
            TMC2130DriverConfig& conf)
        : _registers(conf.registers),
          _current_config(conf.current_config),
          _spi_manager(spi_manager),
          _task_queue(task_queue),
          _initialized(false) {}

    TMC2130(const TMC2130& c) = delete;
    TMC2130(const TMC2130&& c) = delete;
    auto operator=(const TMC2130& c) = delete;
    auto operator=(const TMC2130&& c) = delete;
    ~TMC2130() = default;

    auto read(Registers addr, uint32_t command_data) -> void {
        uint8_t converted_addr = static_cast<uint8_t>(addr);
        _spi_manager.read(converted_addr, command_data, _task_queue,
                          converted_addr);
    }

    auto write(Registers addr, uint32_t command_data) -> bool {
        uint8_t converted_addr = static_cast<uint8_t>(addr);
        return _spi_manager.write(converted_addr, command_data, _task_queue,
                                  converted_addr);
    }

    auto write_config() -> bool {
        if (!set_gconf(_registers.gconfig)) {
            return false;
        }
        if (!set_current_control(_registers.ihold_irun)) {
            return false;
        }
        if (!set_power_down_delay(
                PowerDownDelay::reg_to_seconds(_registers.tpowerdown.time))) {
            return false;
        }
        if (!set_cool_threshold(_registers.tcoolthrs)) {
            return false;
        }
        if (!set_thigh(_registers.thigh)) {
            return false;
        }
        if (!set_chop_config(_registers.chopconf)) {
            return false;
        }
        if (!set_cool_config(_registers.coolconf)) {
            return false;
        }
        _initialized = true;
        return true;
    }

    [[nodiscard]] auto convert_to_tmc2130_current_value(uint32_t c) const
        -> uint32_t {
        constexpr const float SMALL_R = 0.02;
        constexpr auto SQRT_TWO = sqrt2;
        auto FLOAT_CONSTANT = static_cast<float>(
            SQRT_TWO * 32.0 * (_current_config.r_sense + SMALL_R) /
            _current_config.v_sf);
        auto fixed_point_constant = static_cast<uint32_t>(
            FLOAT_CONSTANT * static_cast<float>(1LL << 16));
        uint64_t val = static_cast<uint64_t>(fixed_point_constant) *
                       static_cast<uint64_t>(c);
        auto new_val = static_cast<uint32_t>(val >> 32);
        return new_val - 1;
    }

    /**
     * @brief Check if the TMC2130 has been initialized.
     * @return true if the registers have been written at least once,
     * false otherwise.
     */
    [[nodiscard]] auto initialized() const -> bool { return _initialized; }

    auto handle_spi_read(Registers addr,
                         const spi::utils::MaxMessageBuffer& rxBuffer)
        -> uint32_t {
        uint32_t response = 0;
        const auto* iter = rxBuffer.cbegin();                      // NOLINT
        iter = bit_utils::bytes_to_int(iter + 1, rxBuffer.cend(),  // NOLINT
                                       response);
        switch (addr) {
            case Registers::GCONF:
                _registers.gconfig = GConfig{response};
                return response;
            case Registers::GSTAT:
                _registers.gstat = GStatus{response};
                return response;
            case Registers::CHOPCONF:
                _registers.chopconf = ChopConfig{response};
                return response;
            case Registers::DRVSTATUS:
                _registers.drvstatus = DriveStatus{response};
                return response;
            default:
                return response;
        }
    }

    auto handle_spi_write(Registers addr,
                          const spi::utils::MaxMessageBuffer& rxBuffer)
        -> void {
        switch (addr) {
            case Registers::GCONF:
            case Registers::GSTAT:
            case Registers::CHOPCONF:
            case Registers::DRVSTATUS:
            default:
                break;
        }
    }

    // FUNCTIONS TO SET INDIVIDUAL REGISTERS

    /**
     * @brief Update GCONF register
     * @param reg New configuration register to set
     * @param policy Instance of abstraction policy to use
     * @return True if new register was set succesfully, false otherwise
     */
    auto set_gconf(GConfig reg) -> bool {
        reg.enc_commutation = 0;
        reg.test_mode = 0;
        if (set_register(reg)) {
            _registers.gconfig = reg;
            return true;
        }
        return false;
    }

    /**
     * @brief Update IHOLDIRUN register
     * @param reg New configuration register to set
     * @param policy Instance of abstraction policy to use
     * @return True if new register was set succesfully, false otherwise
     */
    auto set_current_control(CurrentControl reg) -> bool {
        reg.bit_padding_1 = 0;
        reg.bit_padding_2 = 0;
        if (set_register(reg)) {
            _registers.ihold_irun = reg;
            return true;
        }
        return false;
    }
    /**
     * @brief Update TPOWERDOWN register
     * @param reg New configuration register to set
     * @param policy Instance of abstraction policy to use
     * @return True if new register was set succesfully, false otherwise
     */
    auto set_power_down_delay(double time) -> bool {
        PowerDownDelay temp_reg = {.time =
                                       PowerDownDelay::seconds_to_reg(time)};
        if (set_register(temp_reg)) {
            _registers.tpowerdown = temp_reg;
            return true;
        }
        return false;
    }

    /**
     * @brief Update TCOOLTHRSH register
     * @param reg New configuration register to set
     * @param policy Instance of abstraction policy to use
     * @return True if new register was set succesfully, false otherwise
     */
    auto set_cool_threshold(TCoolThreshold reg) -> bool {
        if (set_register(reg)) {
            _registers.tcoolthrs = reg;
            return true;
        }
        return false;
    }
    /**
     * @brief Update THIGH register
     * @param reg New configuration register to set
     * @param policy Instance of abstraction policy to use
     * @return True if new register was set succesfully, false otherwise
     */
    auto set_thigh(THigh reg) -> bool {
        if (set_register(reg)) {
            _registers.thigh = reg;
            return true;
        }
        return false;
    }

    /**
     * @brief Update CHOPCONF register
     * @param reg New configuration register to set
     * @param policy Instance of abstraction policy to use
     * @return True if new register was set succesfully, false otherwise
     */
    auto set_chop_config(ChopConfig reg) -> bool {
        if (set_register(reg)) {
            _registers.chopconf = reg;
            return true;
        }
        return false;
    }

    /**
     * @brief Update COOLCONF register
     * @param reg New configuration register to set
     * @param policy Instance of abstraction policy to use
     * @return True if new register was set succesfully, false otherwise
     */
    auto set_cool_config(CoolConfig reg) -> bool {
        // Assert that bits that MUST be 0 are actually 0
        reg.padding_1 = 0;
        reg.padding_2 = 0;
        reg.padding_3 = 0;
        reg.padding_4 = 0;
        if (set_register(reg)) {
            _registers.coolconf = reg;
            return true;
        }
        return false;
    }

    /**
     * @brief Get the current GCONF register status. This register can
     * be read, so this function gets it from the actual device.
     */
    [[nodiscard]] auto get_gconf() -> std::optional<GConfig> {
        auto ret = read_register<GConfig>();
        if (ret.has_value()) {
            _registers.gconfig = ret.value();
        }
        return ret;
    }
    /**
     * @brief Get the general status register
     */
    [[nodiscard]] auto get_gstatus() -> GStatus {
        auto ret = read_register<GStatus>();
        if (ret.has_value()) {
            return ret.value();
        }
        return GStatus{.driver_error = 1};
    }
    /**
     * @brief Get the current CHOPCONF register status. This register can
     * be read, so this function gets it from the actual device.
     */
    [[nodiscard]] auto get_chop_config() -> std::optional<ChopConfig> {
        auto ret = read_register<ChopConfig>();
        if (ret.has_value()) {
            _registers.chopconf = ret.value();
        }
        return ret;
    }

    /**
     * @brief Get the current DRV_STATUS register reading. Contains
     * information on the current error & stallguard status of the IC.
     * @return The register, or nothing if the register couldn't be read.
     */
    [[nodiscard]] auto get_driver_status() -> std::optional<DriveStatus> {
        return read_register<DriveStatus>();
    }

    /**
     * @brief Get the register map
     */
    [[nodiscard]] auto get_register_map() -> TMC2130RegisterMap& {
        return _registers;
    }

  private:
    /**
     * @brief Set a register on the TMC2130
     *
     * @tparam Reg The type of register to set
     * @tparam Policy Abstraction class for actual writing
     * @param[in] policy Instance of the abstraction policy to use
     * @param[in] reg The register to write
     * @return True if the register could be written, false otherwise.
     * Attempts to write to an unwirteable register will throw a static
     * assertion.
     */
    template <TMC2130Register Reg>
    requires WritableRegister<Reg>
    auto set_register(Reg& reg) -> bool {
        // Ignore the typical linter warning because we're only using
        // this on __packed structures that mimic hardware registers
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto value = *reinterpret_cast<RegisterSerializedTypeA*>(&reg);
        value &= Reg::value_mask;
        return write(Reg::address, value);
    }
    /**
     * @brief Read a register on the TMC2130
     *
     * @tparam Reg The type of register to read
     * @tparam Policy Abstraction class for actual writing
     * @param[in] policy Instance of the abstraction policy to use
     * @return The contents of the register, or nothing if the register
     * can't be read.
     */
    template <TMC2130Register Reg>
    requires ReadableRegister<Reg>
    auto read_register(uint32_t data) -> std::optional<Reg> {
        using RT = std::optional<RegisterSerializedType>;
        using RG = std::optional<Reg>;

        auto ret = RT(data);
        if (!ret.has_value()) {
            return RG();
        }
        // Ignore the typical linter warning because we're only using
        // this on __packed structures that mimic hardware registers
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return RG(*reinterpret_cast<Reg*>(&ret.value()));
    }

    TMC2130RegisterMap _registers = {};
    TMC2130MotorCurrentConfig _current_config = {};
    Writer& _spi_manager;
    TaskQueue& _task_queue;
    bool _initialized;
};

}  // namespace driver

}  // namespace tmc2130
#pragma GCC diagnostic pop
