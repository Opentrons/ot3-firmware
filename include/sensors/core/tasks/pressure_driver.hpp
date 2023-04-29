#pragma once

#include <cmath>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "i2c/core/messages.hpp"
#include "sensors/core/mmr920C04.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/sensors.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {

namespace tasks {

using namespace can::ids;

/**
 * Pressure sensor driver class. It takes in a i2c writer queue and
 * can client queue so that it can handle sensor readings internally.
 *
 * @tparam I2CQueueWriter
 * @tparam CanClient
 */

template <class I2CQueueWriter, class I2CQueuePoller,
          can::message_writer_task::TaskClient CanClient, class OwnQueue>
class MMR920C04 {
  public:
    MMR920C04(I2CQueueWriter &writer, I2CQueuePoller &poller,
              CanClient &can_client, OwnQueue &own_queue,
              sensors::hardware::SensorHardwareBase &hardware,
              const can::ids::SensorId &id)
        : writer(writer),
          poller(poller),
          can_client(can_client),
          own_queue(own_queue),
          hardware(hardware),
          sensor_id(id) {}

    /**
     * @brief Check if the MMR92C04 has been initialized.
     * @return true if the config registers have been written at least once,
     * false otherwise.
     */
    [[nodiscard]] auto initialized() const -> bool { return _initialized; }

    auto register_map() -> mmr920C04::MMR920C04RegisterMap & {
        return _registers;
    }

    auto get_sensor_type() -> SensorType { return SensorType::pressure; }

    auto get_host_id() -> NodeId { return NodeId::host; }

    void set_filter(uint8_t should_filter) { filter_setting = static_cast<mmr920C04::FilterSetting>(should_filter); }

    void set_echoing(bool should_echo) { echoing = should_echo; }

    void set_bind_sync(bool should_bind) {
        bind_sync = should_bind;
        hardware.reset_sync();
    }

    auto get_threshold() -> int32_t { return threshold_pascals; }

    auto set_threshold(float threshold_pa,
                       can::ids::SensorThresholdMode from_mode,
                       uint32_t message_index) -> void {
        threshold_pascals = threshold_pa;
        auto message = can::messages::SensorThresholdResponse{
            .message_index = message_index,
            .sensor = SensorType::pressure,
            .sensor_id = sensor_id,
            .threshold =
                convert_to_fixed_point(threshold_pascals, S15Q16_RADIX),
            .mode = from_mode};
        can_client.send_can_message(can::ids::NodeId::host, message);
    }

    auto write(mmr920C04::Registers reg, uint8_t command_data) -> bool {
        return writer.write(mmr920C04::ADDRESS, static_cast<uint8_t>(reg),
                     command_data);
    }

    auto transact(mmr920C04::Registers reg) -> bool {
        std::array reg_buf{static_cast<uint8_t>(reg)};
        return writer.transact(
            mmr920C04::ADDRESS, reg_buf, 4, own_queue,
            utils::build_id(mmr920C04::ADDRESS, static_cast<uint8_t>(reg)));
    }

    auto write_config() -> bool {
        if (!reset(_registers.reset)) {
            return false;
        }
        _initialized = true;
        return true;
    }

    auto poll_limited_pressure(uint16_t number_reads, uint8_t tags) -> void {
        auto mode_delay_with_buffer = MeasurementTimings[static_cast<int>(measurement_mode_rate)] + DEFAULT_DELAY_BUFFER;
        auto command_data = build_register_command(_registers.low_pass_pressure_command);
        if (filter_setting == mmr920C04::FilterSetting::NO_FILTER) {
            command_data = build_register_command(_registers.pressure_command);
        }
        total_baseline_reads = number_reads;

        set_measure_mode(measurement_mode_rate);
        poller.single_register_poll(
            mmr920C04::ADDRESS, command_data, 3, number_reads, mode_delay_with_buffer, own_queue,
            utils::build_id(mmr920C04::ADDRESS, command_data, tags)
        );
    }

    auto poll_limited_temperature(uint16_t number_reads, uint8_t tags) -> void {
        auto mode_delay_with_buffer = MeasurementTimings[static_cast<int>(measurement_mode_rate)] + DEFAULT_DELAY_BUFFER;
        auto command_data = build_register_command(_registers.temperature_command);
        total_baseline_reads = number_reads;

        set_measure_mode(measurement_mode_rate);
        poller.single_register_poll(
            mmr920C04::ADDRESS, command_data, 3, number_reads, mode_delay_with_buffer, own_queue,
            utils::build_id(mmr920C04::ADDRESS, command_data, tags)
        );
    }

    auto poll_continuous_pressure(uint8_t tags) -> void {
        auto mode_delay_with_buffer = MeasurementTimings[static_cast<int>(measurement_mode_rate)] + DEFAULT_DELAY_BUFFER;
        auto command_data = build_register_command(_registers.low_pass_pressure_command);
        if (filter_setting == mmr920C04::FilterSetting::NO_FILTER) {
            command_data = build_register_command(_registers.pressure_command);
        }

        set_measure_mode(measurement_mode_rate);
        poller.continuous_single_register_poll(
            mmr920C04::ADDRESS, command_data, 3, mode_delay_with_buffer, own_queue, utils::build_id(mmr920C04::ADDRESS, command_data, tags));
    }

    auto poll_continuous_temperature(uint8_t tags) -> void {
        auto mode_delay_with_buffer = MeasurementTimings[static_cast<int>(measurement_mode_rate)] + DEFAULT_DELAY_BUFFER;
        auto command_data = build_register_command(_registers.temperature_command);

        set_measure_mode(measurement_mode_rate);
        poller.continuous_single_register_poll(
            mmr920C04::ADDRESS, command_data, 3, mode_delay_with_buffer, own_queue, utils::build_id(mmr920C04::ADDRESS, command_data, tags));
    }

    auto set_measure_mode(mmr920C04::MeasurementRate rate) -> bool {
        measurement_mode_rate = rate;
        switch (rate) {
            case mmr920C04::MeasurementRate::MEASURE_1:
                if (set_register(_registers.measure_mode_1)) {

                    return true;
                }
                return false;
            case mmr920C04::MeasurementRate::MEASURE_2:
                if (set_register(_registers.measure_mode_2)) {
                    return true;
                }
                return false;
            case mmr920C04::MeasurementRate::MEASURE_3:
                if (set_register(_registers.measure_mode_3)) {
                    return true;
                }
                return false;
            case mmr920C04::MeasurementRate::MEASURE_4:
                if (set_register(_registers.measure_mode_4)) {
                    return true;
                }
                return false;
            default:
                return false;
        }
    }

    auto reset(mmr920C04::Reset reg) -> bool {
        if (set_register(reg)) {
            _registers.reset = reg;
            return true;
        }
        return false;
    }

    auto save_pressure(int32_t data) -> bool {
        _registers.pressure_result.reading = data;
        LOG("Updated pressure reading is %u", _registers.pressure_result.reading);
        return true;
    }

    auto save_temperature(int32_t data) -> bool {
        _registers.temperature_result.reading = data;
        LOG("Updated temperature reading is %u", _registers.temperature_result.reading);
        return true;
    }

    auto read_status(uint8_t data) -> bool {
        _registers.status_result.reading = data;
        return true;
    }

    auto send_status(uint32_t message_index) -> void {
        auto status = mmr920C04::StatusResult::to_status(_registers.status_result.reading);
        auto message = can::messages::ReadFromSensorResponse{
            .message_index = message_index,
            .sensor = get_sensor_type(),
            .sensor_id = sensor_id,
            .sensor_data = static_cast<int32_t>(status)};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_threshold(uint32_t message_index) -> void {
        auto message = can::messages::SensorThresholdResponse{
            .message_index = message_index,
            .sensor = get_sensor_type(),
            .sensor_id = sensor_id,
            .threshold = get_threshold()};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_peripheral_response(uint32_t message_index) -> void {
        auto message = can::messages::PeripheralStatusResponse{
            .message_index = message_index,
            .sensor = get_sensor_type(),
            .sensor_id = sensor_id,
            .status = initialized()};
        can_client.send_can_message(get_host_id(), message);
    }

    void reset_readings() {
        writer.write(mmr920C04::ADDRESS,
                     static_cast<uint8_t>(mmr920C04::Registers::RESET));
    }

    void stop_continuous_polling(uint8_t transaction_index, uint8_t reg_id) {
        poller.continuous_single_register_poll(
            fdc1004::ADDRESS, reg_id, 3,
            STOP_DELAY, own_queue, transaction_index);
    }

    auto handle_ongoing_response(i2c::messages::TransactionResponse &m) -> void {
        auto reg_id = utils::reg_from_id<mmr920C04::Registers>(m.id.token);
        if (!bind_sync && !echoing) {
            stop_continuous_polling(m.id.transaction_index, static_cast<uint8_t>(reg_id));
            reset_readings();
        }

        // Pressure is always a three-byte value
        static_cast<void>(bit_utils::bytes_to_int(
            m.read_buffer.cbegin(), m.read_buffer.cend(), temporary_data_store));

        auto pressure_read = reg_id == mmr920C04::Registers::LOW_PASS_PRESSURE_READ || reg_id == mmr920C04::Registers::PRESSURE_READ;
        auto temperature_read = reg_id == mmr920C04::Registers::TEMPERATURE_READ;
        if (pressure_read) {
            save_pressure(temporary_data_store);
        }
        if (temperature_read) {
            save_temperature(temporary_data_store);
        }
        if (bind_sync && pressure_read) {
            auto pressure = mmr920C04::PressureResult::to_pressure(temporary_data_store);
            if (std::fabs(pressure) - std::fabs(current_pressure_baseline_pa) > threshold_pascals) {
                hardware.set_sync();
            } else {
                hardware.reset_sync();
            }
        }
        if (echoing && pressure_read) {
            auto pressure = mmr920C04::PressureResult::to_pressure(temporary_data_store);
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ReadFromSensorResponse{
                    .message_index = m.message_index,
                    .sensor = can::ids::SensorType::pressure,
                    .sensor_id = sensor_id,
                    .sensor_data =
                        mmr920C04::reading_to_fixed_point(pressure)});
        }
        if (echoing && temperature_read) {
            auto temperature = mmr920C04::PressureResult::to_pressure(temporary_data_store);
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ReadFromSensorResponse{
                    .message_index = m.message_index,
                    .sensor = can::ids::SensorType::pressure_temperature,
                    .sensor_id = sensor_id,
                    .sensor_data =
                        mmr920C04::reading_to_fixed_point(temperature)});
        }
    }

    auto handle_baseline_response(i2c::messages::TransactionResponse &m) -> void {
        static_cast<void>(bit_utils::bytes_to_int(
            m.read_buffer.cbegin(), m.read_buffer.cend(), temporary_data_store));

        auto reg_id = utils::reg_from_id<mmr920C04::Registers>(m.id.token);

        auto pressure_read = reg_id == mmr920C04::Registers::LOW_PASS_PRESSURE_READ || reg_id == mmr920C04::Registers::PRESSURE_READ;
        auto temperature_read = reg_id == mmr920C04::Registers::TEMPERATURE_READ;
        if (pressure_read) {
            auto pressure = mmr920C04::PressureResult::to_pressure(temporary_data_store);
            pressure_running_total += pressure;
        }
        if (temperature_read) {
            auto temperature = mmr920C04::TemperatureResult::to_temperature(temporary_data_store);
            temperature_running_total += temperature;
        }
        if (!m.id.is_completed_poll) {
            return;
        }
        reset_readings();
    

        if (pressure_read) {
            auto current_pressure_baseline_pa = pressure_running_total / total_baseline_reads;
            auto offset_fixed_point =
                   mmr920C04::reading_to_fixed_point(current_pressure_baseline_pa);
            // FIXME This should be tied to the set threshold
            // command so we can completely remove the base line sensor
            // request from all sensors!
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::BaselineSensorResponse{
                .message_index = m.message_index,
                .sensor = can::ids::SensorType::pressure,
                .offset_average = offset_fixed_point});
            pressure_running_total = 0x0;
        }
        if (temperature_read) {
            auto current_temperature_baseline = temperature_running_total / total_baseline_reads;
            auto offset_fixed_point =
                    mmr920C04::reading_to_fixed_point(current_temperature_baseline);
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::BaselineSensorResponse{
                .message_index = m.message_index,
                .sensor = can::ids::SensorType::pressure_temperature,
                .offset_average = offset_fixed_point});
            temperature_running_total = 0x0;
        }
    }

    auto get_can_client() -> CanClient & { return can_client; }

  private:
    I2CQueueWriter &writer;
    I2CQueuePoller &poller;
    CanClient &can_client;
    OwnQueue &own_queue;
    hardware::SensorHardwareBase &hardware;
    const can::ids::SensorId &sensor_id;

    mmr920C04::MMR920C04RegisterMap _registers{};
    mmr920C04::FilterSetting filter_setting = mmr920C04::FilterSetting::LOW_PASS_FILTER;
    
    static constexpr float MeasurementTimings[] = {3.1, 6.1, 12.2, 24.3}; // in msec
    static constexpr float DEFAULT_DELAY_BUFFER = 1.0; // in msec (TODO might need to change to fit in uint16_t)
    static constexpr uint16_t STOP_DELAY = 0;
    mmr920C04::MeasurementRate measurement_mode_rate = mmr920C04::MeasurementRate::MEASURE_4;

    bool _initialized = false;
    bool echoing = false;
    bool bind_sync = false;

    float pressure_running_total = 0;
    float temperature_running_total = 0;
    uint16_t total_baseline_reads = 0x8;
    // TODO(fs, 2022-11-11): Need to figure out a realistic threshold. Pretty
    // sure this is an arbitrarily large number to enable continuous reads.
    float current_pressure_baseline_pa = 0;
    float current_temperature_baseline = 0;
    float threshold_pascals = 100.0F;
    float offset_average = 0;

    int32_t temporary_data_store = 0x0;

    template <mmr920C04::MMR920C04CommandRegister Reg>
    requires registers::WritableRegister<Reg>
    auto build_register_command(Reg &reg) -> uint8_t {
        auto value =
            // Ignore the typical linter warning because we're only using
            // this on __packed structures that mimic hardware registers
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            *reinterpret_cast<mmr920C04::RegisterSerializedTypeA *>(&reg);
        value &= Reg::value_mask;
        return value;
    }

    template <mmr920C04::MMR920C04CommandRegister Reg>
    requires registers::WritableRegister<Reg>
    auto set_register(Reg &reg) -> bool {
        auto value =
            // Ignore the typical linter warning because we're only using
            // this on __packed structures that mimic hardware registers
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            *reinterpret_cast<mmr920C04::RegisterSerializedTypeA *>(&reg);
        value &= Reg::value_mask;
        return write(Reg::address, value);
    }

};

}  // namespace tasks
}  // namespace sensors
