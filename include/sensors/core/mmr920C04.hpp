#pragma once

#include <optional>

#include "can/core/ids.hpp"
#include "sensors/core/mmr920C04_registers.hpp"
#include "sensors/core/sensors.hpp"

namespace mmr920C04 {

using namespace can_ids;
using namespace mmr920C04_registers;
using namespace sensors_registers;

template <class I2CQueueWriter, message_writer_task::TaskClient CanClient>
class MMR92C04 {
  public:
    MMR92C04(I2CQueueWriter &writer, CanClient &can_client)
        : writer(writer), can_client(can_client) {}

    /**
     * @brief Check if the MMR92C04 has been initialized.
     * @return true if the config registers have been written at least once,
     * false otherwise.
     */
    [[nodiscard]] auto initialized() const -> bool { return _initialized; }

    auto register_map() -> MMR920C04RegisterMap & { return _registers; }

    auto get_sensor_id() -> SensorType { return SensorType::pressure; }

    auto get_host_id() -> NodeId { return NodeId::host; }

    auto get_threshold() -> int32_t { return threshold; }

    auto set_threshold(int32_t new_threshold) -> void {
        threshold = new_threshold;
    }

    auto write(Registers reg, uint32_t command_data) -> void {
        writer.write(command_data, ADDRESS, reg);
    }

    auto read(Registers reg) -> void {
        writer.read(
            ADDRESS, [this, reg]() { send_to_can(this, reg); },
            [this, reg](auto message_a) { handle_data(message_a, this, reg); },
            reg);
    }

    auto poll_read(Registers reg, uint16_t number_reads, uint16_t delay)
        -> void {
        writer.single_register_poll(
            ADDRESS, number_reads, delay,
            [this, reg]() { send_to_can(this, reg); },
            [this, reg](auto message_a) { handle_data(message_a, this, reg); },
            reg);
    }

    auto write_config() -> bool {
        if (!reset(_registers.reset)) {
            return false;
        }
        if (!set_measure_mode4(_registers.measure_mode_4)) {
            return false;
        }
        _initialized = true;
        return true;
    }

    auto set_measure_mode1(MeasureMode1 reg) -> bool {
        if (set_register(reg)) {
            _registers.measure_mode_1 = reg;
            return true;
        }
        return false;
    }

    auto set_measure_mode2(MeasureMode2 reg) -> bool {
        if (set_register(reg)) {
            _registers.measure_mode_2 = reg;
            return true;
        }
        return false;
    }

    auto set_measure_mode3(MeasureMode3 reg) -> bool {
        if (set_register(reg)) {
            _registers.measure_mode_3 = reg;
            return true;
        }
        return false;
    }

    auto set_measure_mode4(MeasureMode4 reg) -> bool {
        if (set_register(reg)) {
            _registers.measure_mode_4 = reg;
            return true;
        }
        return false;
    }

    auto reset(Reset reg) -> bool {
        if (set_register(reg)) {
            _registers.reset = reg;
            return true;
        }
        return false;
    }

    auto read_pressure(Pressure reg) -> bool {
        if (read_register(reg)) {
            _registers.pressure = reg;
            return true;
        }
        return false;
    }

    auto read_pressure_low_pass(LowPassPressure reg) -> bool {
        if (read_register(reg)) {
            _registers.low_pass_pressure = reg;
            return true;
        }
        return false;
    }

    auto read_temperature(Temperature reg) -> bool {
        if (read_register(reg)) {
            _registers.temperature = reg;
            return true;
        }
        return false;
    }

    auto send_pressure() -> void {
        auto pressure = Pressure::to_pressure(_registers.pressure.reading);
        auto message = can_messages::ReadFromSensorResponse{
            .sensor = get_sensor_id(), .sensor_data = pressure};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_pressure_low_pass() -> void {
        auto pressure =
            LowPassPressure::to_pressure(_registers.low_pass_pressure.reading);
        auto message = can_messages::ReadFromSensorResponse{
            .sensor = get_sensor_id(), .sensor_data = pressure};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_temperature() -> void {
        auto temperature =
            Temperature::to_temperature(_registers.low_pass_pressure.reading);
        auto message = can_messages::ReadFromSensorResponse{
            .sensor = get_sensor_id(), .sensor_data = temperature};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_status() -> void {
        auto status = Status::to_status(_registers.status.reading);
        auto message = can_messages::ReadFromSensorResponse{
            .sensor = get_sensor_id(),
            .sensor_data = static_cast<int32_t>(status)};
        can_client.send_can_message(get_host_id(), message);
    }

    // callbacks
    static auto handle_data(const sensor_callbacks::MaxMessageBuffer &buffer,
                            MMR92C04 *instance, Registers reg) -> void {
        uint32_t data = 0x0;
        const auto *iter = buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
        switch (reg) {
            case Registers::PRESSURE_READ:
                instance->read_pressure(instance->get_register_map()->pressure);
                break;
            case Registers::LOW_PASS_PRESSURE_READ:
                instance->read_pressure_low_pass(
                    instance->get_register_map()->low_pass_pressure);
                break;
            case Registers::TEMPERATURE_READ:
                instance->read_temperature(
                    instance->get_register_map()->temperature);
                break;
            case Registers::STATUS:
                instance->read_status(instance->get_register_map()->status);
                break;
        }
    }

    static auto send_to_can(MMR92C04 *instance, Registers reg) {
        switch (reg) {
            case Registers::PRESSURE_READ:
                instance->send_pressure();
                break;
            case Registers::LOW_PASS_PRESSURE_READ:
                instance->send_pressure_low_pass();
                break;
            case Registers::TEMPERATURE_READ:
                instance->send_temperature();
                break;
            case Registers::STATUS:
                instance->send_status();
                break;
        }
    }

  private:
    MMR920C04RegisterMap _registers{};
    bool _initialized = false;
    int32_t threshold = 0x8;
    I2CQueueWriter &writer;
    CanClient &can_client;

    template <MMR920C04Register Reg>
    requires WritableRegister<Reg>
    auto set_register(Reg reg) -> bool {
        // Ignore the typical linter warning because we're only using
        // this on __packed structures that mimic hardware registers
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto value = *reinterpret_cast<RegisterSerializedTypeA *>(&reg);
        value &= Reg::value_mask;
        return write(Reg::address, value);
    }

    template <MMR920C04Register Reg>
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
        return RG(*reinterpret_cast<Reg *>(&ret.value()));
    }
};

}  // namespace mmr920C04