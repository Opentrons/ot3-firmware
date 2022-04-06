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
#include "tmc2130_config.hpp"
#include "tmc2130_registers.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

namespace tmc2130 {

using namespace std::numbers;

// using namespace tmc2130_registers;

class TMC2130 {
  public:
    static constexpr size_t MESSAGE_LEN = 5;
    // The type of a single TMC2130 message.
    using MessageT = std::array<uint8_t, MESSAGE_LEN>;

    spi::SpiDeviceBase::BufferType rxBuffer{0};

    TMC2130() = delete;
    TMC2130(const TMC2130DriverConfig& conf, spi::SpiDeviceBase& queue)
        : _registers(conf.registers),
          _current_config(conf.current_config),
          _spi_queue(queue),
          _initialized(false) {}

    /**
     * @brief Build a message to send over SPI
     * @param[in] addr The address to write to
     * @param[in] mode The mode to use, either WRITE or READ
     * @param[in] val The contents to write to the address (0 if this is a read)
     * @return An array with the contents of the message, or nothing if
     * there was an error
     */
    static auto build_message(Registers addr, spi::SpiDeviceBase::Mode mode,
                              RegisterSerializedType val) -> MessageT {
        MessageT buffer = {0};
        auto* iter = buffer.begin();
        auto addr_byte = static_cast<uint8_t>(addr);
        addr_byte |= static_cast<uint8_t>(mode);
        iter = bit_utils::int_to_bytes(addr_byte, iter, buffer.end());
        iter = bit_utils::int_to_bytes(val, iter, buffer.end());
        if (iter != buffer.end()) {
            return MessageT();
        }
        return buffer;
    }

    /**
     * @brief Build a message to send over SPI
     * @param[in] addr The address to write to
     * @param[in] mode The mode to use, either WRITE or READ
     * @param[in] val The contents to write to the address (0 if this is a read)
     * @return An array with the contents of the message, or nothing if
     * there was an error
     */
    static auto build_queue_message(txBuffer, rxBuffer) -> MessageT {
        return SpiTransact{txBuffer, rxBuffer, callback};
    }

    auto read(Registers addr, uint32_t command_data) -> uint32_t {
        auto txBuffer =
            build_message(addr, spi::SpiDeviceBase::Mode::READ, command_data);
        // A read requires two transmissions. The second returns the data in the
        // register from the first transmission.
        _spi_queue.write(build_queue_message(txBuffer, rxBuffer));
        _spi_queue.write(build_queue_message(txBuffer, rxBuffer));

        // Extract data bytes after the address.
        uint32_t response = 0;
        const auto* iter = rxBuffer.cbegin();                      // NOLINT
        iter = bit_utils::bytes_to_int(iter + 1, rxBuffer.cend(),  // NOLINT
                                       response);
        return response;
    }

    auto write(Registers addr, uint32_t command_data) -> bool {
        auto txBuffer =
            build_message(addr, spi::SpiDeviceBase::Mode::WRITE, command_data);
        auto response = _spi_comms.transmit_receive(txBuffer, rxBuffer);
        return response;
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

    auto write_config(const TMC2130RegisterMap& registers) -> bool {
        if (!set_gconf(registers.gconfig)) {
            return false;
        }
        if (!set_current_control(registers.ihold_irun)) {
            return false;
        }
        if (!set_power_down_delay(
                PowerDownDelay::reg_to_seconds(registers.tpowerdown.time))) {
            return false;
        }
        if (!set_cool_threshold(registers.tcoolthrs)) {
            return false;
        }
        if (!set_thigh(registers.thigh)) {
            return false;
        }
        if (!set_chop_config(registers.chopconf)) {
            return false;
        }
        if (!set_cool_config(registers.coolconf)) {
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

    //callback
    static auto handle_data(const sensor_callbacks::MaxMessageBuffer &buffer,
                            TMC2130 *instance) {

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
    auto set_register(Reg reg) -> bool {
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
    auto read_register() -> std::optional<Reg> {
        using RT = std::optional<RegisterSerializedType>;
        using RG = std::optional<Reg>;

        uint32_t data{0};
        auto ret = RT(read(Reg::address, data));
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
    spi::SpiDeviceBase& _spi_queue;
    bool _initialized;
};

}  // namespace tmc2130
#pragma GCC diagnostic pop
