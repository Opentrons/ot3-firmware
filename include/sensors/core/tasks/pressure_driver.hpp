#pragma once

#include <cmath>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "common/core/sensor_buffer.hpp"
#include "i2c/core/messages.hpp"
#include "sensors/core/mmr920.hpp"
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
class MMR920 {
  public:
    MMR920(I2CQueueWriter &writer, I2CQueuePoller &poller,
           CanClient &can_client, OwnQueue &own_queue,
           sensors::hardware::SensorHardwareBase &hardware,
           const can::ids::SensorId &id,
           const sensors::mmr920::SensorVersion version,
           std::array<float, SENSOR_BUFFER_SIZE> *sensor_buffer)
        : writer(writer),
          poller(poller),
          can_client(can_client),
          own_queue(own_queue),
          hardware(hardware),
          sensor_id(id),
          sensor_version(version),
          sensor_buffer(sensor_buffer) {}

    /**
     * @brief Check if the MMR92 has been initialized.
     * @return true if the config registers have been written at least once,
     * false otherwise.
     */
    [[nodiscard]] auto initialized() const -> bool { return _initialized; }

    auto register_map() -> mmr920::MMR920RegisterMap & { return _registers; }

    auto get_sensor_type() -> SensorType { return SensorType::pressure; }

    auto get_host_id() -> NodeId { return NodeId::host; }

    void set_filter(uint8_t should_filter) {
        filter_setting = static_cast<mmr920::FilterSetting>(should_filter);
    }

    void set_echoing(bool should_echo) {
        echoing = should_echo;
        if (should_echo) {
            sensor_buffer_index = 0;  // reset buffer index
        }
    }

    void set_bind_sync(bool should_bind) {
        bind_sync = should_bind;
        hardware.reset_sync();
    }

    void set_max_bind_sync(bool should_bind) {
        max_pressure_sync = should_bind;
        hardware.reset_sync();
    }

    auto get_threshold() -> int32_t { return threshold_pascals; }

    auto set_threshold(float threshold_pa,
                       can::ids::SensorThresholdMode from_mode,
                       uint32_t message_index, bool send_threshold = true)
        -> void {
        threshold_pascals = threshold_pa;
        if (send_threshold) {
            auto message = can::messages::SensorThresholdResponse{
                .message_index = message_index,
                .sensor = SensorType::pressure,
                .sensor_id = sensor_id,
                .threshold =
                    convert_to_fixed_point(threshold_pascals, S15Q16_RADIX),
                .mode = from_mode};
            can_client.send_can_message(can::ids::NodeId::host, message);
        }
    }

    auto write(mmr920::Registers reg, uint8_t command_data) -> bool {
        return writer.write(mmr920::ADDRESS, static_cast<uint8_t>(reg),
                            command_data);
    }

    auto transact(mmr920::Registers reg) -> bool {
        std::array reg_buf{static_cast<uint8_t>(reg)};
        return writer.transact(
            mmr920::ADDRESS, reg_buf, 3, own_queue,
            utils::build_id(mmr920::ADDRESS, static_cast<uint8_t>(reg)));
    }

    auto write_config() -> bool {
        if (!reset(_registers.reset)) {
            return false;
        }
        // default measurement mode is MODE 4. 1 sample every 3.21 msec.
        set_measure_mode(measurement_mode_rate);
        _initialized = true;
        return true;
    }

    auto poll_limited_pressure(uint16_t number_reads, uint8_t tags) -> void {
        auto mode_delay_with_buffer =
            MeasurementTimings[static_cast<int>(measurement_mode_rate)] +
            DEFAULT_DELAY_BUFFER;
        auto command_data =
            build_register_command(_registers.low_pass_pressure_command);
        if (filter_setting == mmr920::FilterSetting::NO_FILTER) {
            command_data = build_register_command(_registers.pressure_command);
        }
        total_baseline_reads = number_reads;

        poller.single_register_poll(
            mmr920::ADDRESS, command_data, 3, number_reads,
            mode_delay_with_buffer, own_queue,
            utils::build_id(mmr920::ADDRESS, command_data, tags));
    }

    auto poll_limited_temperature(uint16_t number_reads, uint8_t tags) -> void {
        auto mode_delay_with_buffer =
            MeasurementTimings[static_cast<int>(measurement_mode_rate)] +
            DEFAULT_DELAY_BUFFER;
        auto command_data =
            build_register_command(_registers.temperature_command);
        total_baseline_reads = number_reads;

        poller.single_register_poll(
            mmr920::ADDRESS, command_data, 3, number_reads,
            mode_delay_with_buffer, own_queue,
            utils::build_id(mmr920::ADDRESS, command_data, tags));
    }

    auto poll_continuous_pressure(uint8_t tags) -> void {
        // in milliseconds
        uint16_t mode_delay_with_buffer =
            MeasurementTimings[static_cast<int>(measurement_mode_rate)] +
            DEFAULT_DELAY_BUFFER;
        max_pressure_required_readings =
            MAX_PRESSURE_TIME_MS / mode_delay_with_buffer;
        max_pressure_consecutive_readings = 0;
        auto command_data =
            build_register_command(_registers.low_pass_pressure_command);
        if (filter_setting == mmr920::FilterSetting::NO_FILTER) {
            command_data = build_register_command(_registers.pressure_command);
        }

        poller.continuous_single_register_poll(
            mmr920::ADDRESS, command_data, 3, mode_delay_with_buffer, own_queue,
            utils::build_id(mmr920::ADDRESS, command_data, tags));
    }

    auto poll_continuous_temperature(uint8_t tags) -> void {
        auto mode_delay_with_buffer =
            MeasurementTimings[static_cast<int>(measurement_mode_rate)] +
            DEFAULT_DELAY_BUFFER;
        auto command_data =
            build_register_command(_registers.temperature_command);

        poller.continuous_single_register_poll(
            mmr920::ADDRESS, command_data, 3, mode_delay_with_buffer, own_queue,
            utils::build_id(mmr920::ADDRESS, command_data, tags));
    }

    auto set_measure_mode(mmr920::MeasurementRate rate) -> bool {
        measurement_mode_rate = rate;
        switch (rate) {
            case mmr920::MeasurementRate::MEASURE_1:
                if (set_register(_registers.measure_mode_1)) {
                    return true;
                }
                return false;
            case mmr920::MeasurementRate::MEASURE_2:
                if (set_register(_registers.measure_mode_2)) {
                    return true;
                }
                return false;
            case mmr920::MeasurementRate::MEASURE_3:
                if (set_register(_registers.measure_mode_3)) {
                    return true;
                }
                return false;
            case mmr920::MeasurementRate::MEASURE_4:
                if (set_register(_registers.measure_mode_4)) {
                    return true;
                }
                return false;
            default:
                return false;
        }
    }

    auto reset(mmr920::Reset reg) -> bool {
        if (set_register(reg)) {
            _registers.reset = reg;
            return true;
        }
        return false;
    }

    auto save_pressure(int32_t data) -> bool {
        _registers.pressure_result.reading = data;
        LOG("Updated pressure reading is %u",
            _registers.pressure_result.reading);
        return true;
    }

    auto sensor_buffer_log(float data) -> void {
        if (sensor_buffer_index == SENSOR_BUFFER_SIZE) {
            sensor_buffer_index = 0;
        }

        (*sensor_buffer).at(sensor_buffer_index) = response_pressure;
        sensor_buffer_index++;
    }

    auto save_temperature(int32_t data) -> bool {
        _registers.temperature_result.reading = data;
        LOG("Updated temperature reading is %u",
            _registers.temperature_result.reading);
        return true;
    }

    auto read_status(uint8_t data) -> bool {
        _registers.status_result.reading = data;
        return true;
    }

    auto send_status(uint32_t message_index) -> void {
        auto status =
            mmr920::StatusResult::to_status(_registers.status_result.reading);
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
        writer.write(mmr920::ADDRESS,
                     static_cast<uint8_t>(mmr920::Registers::RESET));
    }

    void stop_continuous_polling(uint32_t transaction_id, uint8_t reg_id) {
        poller.continuous_single_register_poll(
            mmr920::ADDRESS, reg_id, 3, STOP_DELAY, own_queue, transaction_id);
    }

    void send_accumulated_sensor_data(uint32_t message_index) {
#ifdef USE_SENSOR_MOVE
        for (int i = 0; i < SENSOR_BUFFER_SIZE; i++) {
            // send over buffer adn then clear buffer values
            current_index = (i + sensor_buffer_index) % SENSOR_BUFFER_SIZE;

            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ReadFromSensorResponse{
                    .message_index = message_index,
                    .sensor = can::ids::SensorType::pressure,
                    .sensor_id = sensor_id,
                    .sensor_data =
                        mmr920::reading_to_fixed_point((*sensor_buffer).at(current_index))});
            if (i % 10 == 0) {
                // slow it down so the can buffer doesn't choke
                vTaskDelay(50);
            }
            (*sensor_buffer).at(current_index) = 0;
        }
#else
        std::ignore = message_index;
#endif
    }

    auto handle_ongoing_pressure_response(i2c::messages::TransactionResponse &m)
        -> void {
        if (!bind_sync && !echoing && !max_pressure_sync) {
            auto reg_id = utils::reg_from_id<mmr920::Registers>(m.id.token);
            stop_continuous_polling(m.id.token, static_cast<uint8_t>(reg_id));
        }

        bool echo_this_time = echoing;

        // Pressure is always a three-byte value
        static_cast<void>(bit_utils::bytes_to_int(m.read_buffer.cbegin(),
                                                  m.read_buffer.cend(),
                                                  temporary_data_store));

        uint32_t shifted_data_store = temporary_data_store >> 8;

        save_pressure(shifted_data_store);
        auto pressure = mmr920::PressureResult::to_pressure(
            _registers.pressure_result.reading, sensor_version);

        if (max_pressure_sync) {
            sensor_buffer_log(pressure);
            bool this_tick_over_threshold =
                std::fabs(pressure - current_pressure_baseline_pa) >=
                mmr920::get_max_pressure_reading(sensor_version);
            bool over_threshold = false;
            if (this_tick_over_threshold) {
                max_pressure_consecutive_readings =
                    std::min(max_pressure_consecutive_readings + 1,
                             max_pressure_required_readings);
                over_threshold = (max_pressure_consecutive_readings ==
                                  max_pressure_required_readings);
                echo_this_time = true;
            } else {
                max_pressure_consecutive_readings = 0;
            }
            if (over_threshold) {
                hardware.set_sync();
                can_client.send_can_message(
                    can::ids::NodeId::host,
                    can::messages::ErrorMessage{
                        .message_index = m.message_index,
                        .severity = can::ids::ErrorSeverity::unrecoverable,
                        .error_code = can::ids::ErrorCode::over_pressure});
            } else {
                hardware.reset_sync();
            }
        }
        if (bind_sync) {
            if (std::fabs(pressure - current_pressure_baseline_pa) >
                threshold_pascals) {
                hardware.set_sync();
            } else {
                hardware.reset_sync();
            }
        }

        if (echo_this_time) {
            auto response_pressure = pressure - current_pressure_baseline_pa;
#ifdef USE_SENSOR_MOVE
            if (sensor_buffer_index < SENSOR_BUFFER_SIZE) {
                (*sensor_buffer).at(sensor_buffer_index) = response_pressure;
                sensor_buffer_index++;
            }
#else
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ReadFromSensorResponse{
                    .message_index = m.message_index,
                    .sensor = can::ids::SensorType::pressure,
                    .sensor_id = sensor_id,
                    .sensor_data =
                        mmr920::reading_to_fixed_point(response_pressure)});
#endif
        }
    }

    auto handle_ongoing_temperature_response(
        i2c::messages::TransactionResponse &m) -> void {
        if (!bind_sync && !echoing) {
            auto reg_id = utils::reg_from_id<mmr920::Registers>(m.id.token);
            stop_continuous_polling(m.id.token, static_cast<uint8_t>(reg_id));
        }

        // Pressure is always a three-byte value
        static_cast<void>(bit_utils::bytes_to_int(m.read_buffer.cbegin(),
                                                  m.read_buffer.cend(),
                                                  temporary_data_store));

        uint32_t shifted_data_store = temporary_data_store >> 8;

        save_temperature(shifted_data_store);

        if (echoing) {
            auto temperature = mmr920::TemperatureResult::to_temperature(
                _registers.temperature_result.reading);
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ReadFromSensorResponse{
                    .message_index = m.message_index,
                    .sensor = can::ids::SensorType::pressure_temperature,
                    .sensor_id = sensor_id,
                    .sensor_data =
                        mmr920::reading_to_fixed_point(temperature)});
        }
    }

    auto handle_baseline_pressure_response(
        i2c::messages::TransactionResponse &m) -> void {
        static_cast<void>(bit_utils::bytes_to_int(m.read_buffer.cbegin(),
                                                  m.read_buffer.cend(),
                                                  temporary_data_store));

        uint32_t shifted_data_store = temporary_data_store >> 8;

        auto pressure = mmr920::PressureResult::to_pressure(shifted_data_store,
                                                            sensor_version);
        pressure_running_total += pressure;

        if (!m.id.is_completed_poll) {
            return;
        }

        auto current_pressure_baseline_pa =
            pressure_running_total / total_baseline_reads;
        auto pressure_fixed_point =
            mmr920::reading_to_fixed_point(current_pressure_baseline_pa);

        // FIXME This should be tied to the set threshold
        // command so we can completely remove the base line sensor
        // request from all sensors!
        if (utils::tag_in_token(m.id.token,
                                utils::ResponseTag::IS_THRESHOLD_SENSE)) {
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::BaselineSensorResponse{
                    .message_index = m.message_index,
                    .sensor = can::ids::SensorType::pressure,
                    .offset_average = pressure_fixed_point});
            set_threshold(current_pressure_baseline_pa,
                          can::ids::SensorThresholdMode::auto_baseline,
                          m.message_index, false);
        } else {
            auto message = can::messages::ReadFromSensorResponse{
                .message_index = m.message_index,
                .sensor = SensorType::pressure,
                .sensor_id = sensor_id,
                .sensor_data = pressure_fixed_point};
            can_client.send_can_message(can::ids::NodeId::host, message);
        }
        pressure_running_total = 0x0;
    }

    auto handle_baseline_temperature_response(
        i2c::messages::TransactionResponse &m) -> void {
        static_cast<void>(bit_utils::bytes_to_int(m.read_buffer.cbegin(),
                                                  m.read_buffer.cend(),
                                                  temporary_data_store));

        uint32_t shifted_data_store = temporary_data_store >> 8;

        auto temperature =
            mmr920::TemperatureResult::to_temperature(shifted_data_store);
        temperature_running_total += temperature;

        if (!m.id.is_completed_poll) {
            return;
        }

        auto current_temperature_baseline =
            temperature_running_total / total_baseline_reads;
        auto offset_fixed_point =
            mmr920::reading_to_fixed_point(current_temperature_baseline);
        if (echoing) {
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::BaselineSensorResponse{
                    .message_index = m.message_index,
                    .sensor = can::ids::SensorType::pressure_temperature,
                    .offset_average = offset_fixed_point});
        }
        temperature_running_total = 0x0;
    }

    auto get_can_client() -> CanClient & { return can_client; }

  private:
    I2CQueueWriter &writer;
    I2CQueuePoller &poller;
    CanClient &can_client;
    OwnQueue &own_queue;
    hardware::SensorHardwareBase &hardware;
    const can::ids::SensorId &sensor_id;
    const sensors::mmr920::SensorVersion sensor_version;

    mmr920::MMR920RegisterMap _registers{};
    mmr920::FilterSetting filter_setting =
        mmr920::FilterSetting::LOW_PASS_FILTER;

    static constexpr std::array<float, 4> MeasurementTimings{0.405, 0.81, 1.62,
                                                             3.24};  // in msec
    static constexpr float DEFAULT_DELAY_BUFFER =
        1.0;  // in msec (TODO might need to change to fit in uint16_t)
    static constexpr uint16_t STOP_DELAY = 0;

    /**
     * Time required before raising a Max Pressure error. The pressure must
     * exceed the threshold for the entirety of this period.
     */
    static constexpr uint16_t MAX_PRESSURE_TIME_MS = 200;
#ifdef USE_PRESSURE_MOVE
    mmr920::MeasurementRate measurement_mode_rate =
        mmr920::MeasurementRate::MEASURE_1;
#else
    mmr920::MeasurementRate measurement_mode_rate =
        mmr920::MeasurementRate::MEASURE_4;
#endif

    bool _initialized = false;
    bool echoing = false;
    bool bind_sync = false;
    bool max_pressure_sync = false;

    float pressure_running_total = 0;
    float temperature_running_total = 0;
    uint16_t total_baseline_reads = 1;

    float current_pressure_baseline_pa = 0;
    float current_temperature_baseline = 0;

    size_t max_pressure_consecutive_readings = 0;
    size_t max_pressure_required_readings = 0;

    // TODO(fs, 2022-11-11): Need to figure out a realistic threshold. Pretty
    // sure this is an arbitrarily large number to enable continuous reads.
    float threshold_pascals = 100.0F;
    float offset_average = 0;

    uint32_t temporary_data_store = 0x0;

    template <mmr920::MMR920CommandRegister Reg>
    requires registers::WritableRegister<Reg>
    auto build_register_command(Reg &reg) -> uint8_t {
        return static_cast<uint8_t>(reg.address);
    }

    template <mmr920::MMR920CommandRegister Reg>
    requires registers::WritableRegister<Reg>
    auto set_register(Reg &reg) -> bool {
        auto value =
            // Ignore the typical linter warning because we're only using
            // this on __packed structures that mimic hardware registers
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            *reinterpret_cast<mmr920::RegisterSerializedTypeA *>(&reg);
        value &= Reg::value_mask;
        return write(Reg::address, value);
    }
    std::array<float, SENSOR_BUFFER_SIZE> *sensor_buffer;
    uint16_t sensor_buffer_index = 0;
};

}  // namespace tasks
}  // namespace sensors
