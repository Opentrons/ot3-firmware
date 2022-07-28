#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "i2c/core/messages.hpp"
#include "sensors/core/hdc3020.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {

namespace tasks {

using namespace can::ids;

template <class I2CQueueWriter, class I2CQueuePoller,
          can::message_writer_task::TaskClient CanClient, class OwnQueue>
class HDC3020 {
  public:
    HDC3020(I2CQueueWriter &writer, I2CQueuePoller &poller,
            CanClient &can_client, OwnQueue &own_queue,
            const can::ids::SensorId &id)
        : writer(writer),
          poller(poller),
          can_client(can_client),
          own_queue(own_queue),
          sensor_id(id) {}

    [[nodiscard]] auto initialized() const -> bool { return _initialized; }

    auto register_map() -> hdc3020::HDC3020RegisterMap & { return _registers; }

    auto get_host_id() -> NodeId { return NodeId::host; }

    auto get_sensor_id() -> can::ids::SensorId { return sensor_id; }

    auto set_bind_flags(uint8_t binding) -> void {
        sensor_binding = static_cast<can::ids::SensorOutputBinding>(binding);
    }

    auto initialize() -> void {
        std::array<uint8_t, 2> write_buffer{
            static_cast<uint8_t>(hdc3020::Registers::AUTO_MEASURE_STATUS),
            _registers.status.initialize};
        writer.write(hdc3020::ADDRESS, write_buffer);
        _initialized = true;
    }

    void trigger_on_demand(uint16_t number_of_reads = 1) {
        std::array<utils::ResponseTag, 2> tags{};
        auto mode = mode_lookup(_registers.trigger_measurement);
        if (number_of_reads > 1) {
            tags[0] = utils::ResponseTag::IS_PART_OF_POLL;
            tags[1] = utils::ResponseTag::IS_BASELINE;
        }
        std::array<uint8_t, 2> command_buffer{
            static_cast<uint8_t>(hdc3020::Registers::TRIGGER_ON_DEMAND_MODE),
            mode};
        poller.single_register_poll(
            hdc3020::ADDRESS, command_buffer, RESPONSE_SIZE, number_of_reads,
            MINIMUM_DELAY_MS, own_queue,
            build_environment_id(hdc3020::Registers::TRIGGER_ON_DEMAND_MODE,
                                 tags));
    }

    void auto_measure_mode(hdc3020::Registers reg) {
        std::array tags{utils::ResponseTag::POLL_IS_CONTINUOUS};
        auto reg_as_int = static_cast<uint8_t>(reg);
        std::array<uint8_t, 2> command_buffer{reg_as_int};
        switch (reg) {
            case hdc3020::Registers::AUTO_MEASURE_1M2S:
                command_buffer[1] = mode_lookup(_registers.measure_mode_1m2s);
                poller.continuous_single_register_poll(
                    hdc3020::ADDRESS, command_buffer, RESPONSE_SIZE, 2000,
                    own_queue, build_environment_id(reg, tags));
                break;
            case hdc3020::Registers::AUTO_MEASURE_1M1S:
                command_buffer[1] = mode_lookup(_registers.measure_mode_1m1s);
                poller.continuous_single_register_poll(
                    hdc3020::ADDRESS, command_buffer, RESPONSE_SIZE, 1000,
                    own_queue, build_environment_id(reg, tags));
                break;
            case hdc3020::Registers::AUTO_MEASURE_2M1S:
                command_buffer[1] = mode_lookup(_registers.measure_mode_2m1s);
                poller.continuous_single_register_poll(
                    hdc3020::ADDRESS, command_buffer, RESPONSE_SIZE, 500,
                    own_queue, build_environment_id(reg, tags));
                break;
            case hdc3020::Registers::AUTO_MEASURE_4M1S:
                command_buffer[1] = mode_lookup(_registers.measure_mode_4m1s);
                poller.continuous_single_register_poll(
                    hdc3020::ADDRESS, command_buffer, RESPONSE_SIZE, 250,
                    own_queue, build_environment_id(reg, tags));
                break;
            case hdc3020::Registers::AUTO_MEASURE_10M1S:
                command_buffer[1] = mode_lookup(_registers.measure_mode_10m1s);
                poller.continuous_single_register_poll(
                    hdc3020::ADDRESS, command_buffer, RESPONSE_SIZE, 100,
                    own_queue, build_environment_id(reg, tags));
                break;
            default:
                break;
        }
    }

    auto check_data(uint32_t raw_humidity, uint32_t raw_temperature)
        -> std::tuple<int32_t, int32_t> {
        /*
         * This function uses the built in sensor CRC to validate that the data
         * is correct. For now, we aren't doing anything except removing the CRC
         * from the data. When we have errors, we should communicate that
         * from the firmware.
         *
         * TODO -> detect errors with CRC
         *
         * Because the data is stored in a uint32_t to support 24 bits of data,
         * which includes the CRC, we need to shift the raw data down before
         * converting to relative humidity/temperature in ºC.
         */
        auto humidity =
            (raw_humidity - (raw_humidity & CRC_8)) >> RAW_DATA_SIZE;
        auto temperature =
            (raw_temperature - (raw_temperature & CRC_8)) >> RAW_DATA_SIZE;
        return std::make_tuple(humidity, temperature);
    }

    void send_hdc3020_data(int32_t humidity, int32_t temperature) {
        auto fixed_humidity = convert_humidity_to_fixed_point(humidity);
        auto fixed_temperature =
            convert_temperature_to_fixed_point(temperature);
        auto hum_message = can::messages::ReadFromSensorResponse{
            .sensor = SensorType::humidity, .sensor_data = fixed_humidity};
        can_client.send_can_message(get_host_id(), hum_message);
        auto temp_message = can::messages::ReadFromSensorResponse{
            .sensor = SensorType::temperature,
            .sensor_data = fixed_temperature};
        can_client.send_can_message(get_host_id(), temp_message);
    }

    void send_status(const can::messages::PeripheralStatusRequest &m) {
        can_client.send_can_message(
            can::ids::NodeId::host,
            can::messages::PeripheralStatusResponse{
                .sensor = m.sensor,
                .sensor_id = get_sensor_id(),
                .status = static_cast<uint8_t>(initialized())});
    }

    void handle_response(const i2c::messages::TransactionResponse &tm) {
        uint32_t raw_humidity = 0x0;
        uint32_t raw_temperature = 0x0;
        const auto *iter = tm.read_buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, iter + 3, raw_humidity);
        iter = bit_utils::bytes_to_int(iter, tm.read_buffer.cend(),
                                       raw_temperature);

        if (raw_humidity != 0x0 || raw_temperature != 0x0) {
            auto humidity_temp = check_data(raw_humidity, raw_temperature);
            auto humidity = std::get<0>(humidity_temp);
            auto temperature = std::get<1>(humidity_temp);
            switch (static_cast<hdc3020::Registers>(
                utils::reg_from_id<uint8_t>(tm.id.token))) {
                case hdc3020::Registers::TRIGGER_ON_DEMAND_MODE:
                    _registers.trigger_measurement.humidity = humidity;
                    _registers.trigger_measurement.temperature = temperature;
                    break;
                case hdc3020::Registers::AUTO_MEASURE_1M2S:
                    _registers.measure_mode_1m2s.humidity = humidity;
                    _registers.measure_mode_1m2s.temperature = temperature;
                    break;
                case hdc3020::Registers::AUTO_MEASURE_1M1S:
                    _registers.measure_mode_1m1s.humidity = humidity;
                    _registers.measure_mode_1m1s.temperature = temperature;
                    break;
                case hdc3020::Registers::AUTO_MEASURE_2M1S:
                    _registers.measure_mode_2m1s.humidity = humidity;
                    _registers.measure_mode_2m1s.temperature = temperature;
                    break;
                case hdc3020::Registers::AUTO_MEASURE_4M1S:
                    _registers.measure_mode_4m1s.humidity = humidity;
                    _registers.measure_mode_4m1s.temperature = temperature;
                    break;
                case hdc3020::Registers::AUTO_MEASURE_10M1S:
                    _registers.measure_mode_10m1s.humidity = humidity;
                    _registers.measure_mode_10m1s.temperature = temperature;
                    break;
                default:
                    break;
            }
            if (sensor_binding == can::ids::SensorOutputBinding::report) {
                // TODO we need to store the humidity/temp values on
                // eeprom at some point. TBD on implementation details.
                send_hdc3020_data(humidity, temperature);
            }
        }
    }

  private:
    hdc3020::HDC3020RegisterMap _registers{};
    bool _initialized = false;
    const uint32_t CRC_8 = 0x3100;
    const uint8_t RESPONSE_SIZE = 6;
    const uint16_t MINIMUM_DELAY_MS = 3;
    // A temperature and humidity reading consists of 2 bytes of data.
    const uint8_t RAW_DATA_SIZE = 16;
    // hopefully this is 65535
    const uint16_t MAXIMUM_DATA_SIZE = (1 << RAW_DATA_SIZE) - 1;
    // (TODO: lc 7-26-2022) we need a can message that
    // allows a user to select the power consumption mode
    // to use for different commands.
    hdc3020::LowPowerMode POWER_MODE{1};
    can::ids::SensorOutputBinding sensor_binding{2};
    I2CQueueWriter &writer;
    I2CQueuePoller &poller;
    CanClient &can_client;
    OwnQueue &own_queue;
    const can::ids::SensorId &sensor_id;

    template <std::ranges::range Tags>
    auto build_environment_id(hdc3020::Registers reg, Tags tags)
        -> unsigned int {
        return utils::build_id(hdc3020::ADDRESS, static_cast<uint8_t>(reg),
                               utils::byte_from_tags(tags));
    }

    template <hdc3020::HDC3020CommandRegister Reg>
    auto mode_lookup(Reg &) -> uint8_t {
        return Reg::mode_map[static_cast<uint8_t>(POWER_MODE)];
    }

    auto convert_humidity_to_fixed_point(int32_t humidity) -> int32_t {
        float rh_percent = humidity * (1.0 / MAXIMUM_DATA_SIZE) * 100.0;
        LOG("Relative humidity: %.4f", rh_percent);
        return convert_to_fixed_point(rh_percent, 16);
    }

    auto convert_temperature_to_fixed_point(int32_t temperature) -> int32_t {
        float temp_celsius =
            175.0 * (temperature * (1.0 / MAXIMUM_DATA_SIZE)) - 45.0;
        LOG("Temperature in celsius: %.4f", temp_celsius);
        return convert_to_fixed_point(temp_celsius, 16);
    }
};

}  // namespace tasks
}  // namespace sensors