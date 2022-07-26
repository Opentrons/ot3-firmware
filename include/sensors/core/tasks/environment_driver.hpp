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
#include "sensors/core/sensors.hpp"
#include "sensors/core/utils.hpp"


namespace sensors {

namespace tasks {

using namespace can::ids;

template <class I2CQueuePoller,
          can::message_writer_task::TaskClient CanClient, class OwnQueue>
class HDC3020 {
  public:
    HDC3020(I2CQueuePoller &poller,
            CanClient &can_client, OwnQueue &own_queue,
            const can::ids::SensorId &id)
        : poller(poller),
          can_client(can_client),
          own_queue(own_queue),
          sensor_id(id) {}

    [[nodiscard]] auto initialized() const -> bool { return _initialized; }

    auto register_map() -> hdc3020::HDC3020RegisterMap & {
        return _registers;
    }

    auto initialize() -> bool {
        poller.multi_register_poll(
            hdc3020::ADDRESS, hdc3020::Registers::AUTO_MEASURE_STATUS, 1,
            _registers.status.initialize, RESPONSE_SIZE, 1, MINIMUM_DELAY_MS,
            own_queue, build_environment_id(reg, tags))
        _initialized = true;
        return _initialized;
    }

    void trigger_on_demand(uint16_t number_of_reads = 1) {
        auto reg2 = mode_lookup(hdc3020::Registers::TRIGGER_ON_DEMAND_MODE);
        if (number_of_reads > 1) {
            std::array tags{utils::ResponseTag::IS_PART_OF_POLL, utils::ResponseTag::IS_BASELINE};
        } else {
            std::array tags{};
        }
        poller.multi_register_poll(
            hdc3020::ADDRESS, hdc3020::Registers::TRIGGER_ON_DEMAND_MODE, 1,
            mode_lookup(_registers.trigger_measurement), RESPONSE_SIZE, number_of_reads, MINIMUM_DELAY_MS,
            own_queue, build_environment_id(reg, tags));
    }

    void auto_measure_mode(hdc3020::Registers reg) {
        std::array tags{utils::ResponseTag::POLL_IS_CONTINUOUS};
        switch (reg) {
            case hdc3020::Registers::AUTO_MEASURE_1M2S:
                poller.continuous_multi_register_poll(
                    hdc3020::ADDRESS, reg, 1, mode_lookup(_registers.measure_mode_1m2s), RESPONSE_SIZE, 2000, own_queue,
                    build_environment_id(reg, tags));
                break;
            case hdc3020::Registers::AUTO_MEASURE_1M1S:
                poller.continuous_multi_register_poll(
                    hdc3020::ADDRESS, reg, 1, mode_lookup(_registers.measure_mode_1m1s), RESPONSE_SIZE, 1000, own_queue,
                    build_environment_id(reg, tags));
                break;
            case hdc3020::Registers::AUTO_MEASURE_2M1S:
                poller.continuous_multi_register_poll(
                    hdc3020::ADDRESS, reg, 1, mode_lookup(_registers.measure_mode_2m1s), RESPONSE_SIZE, 500, own_queue,
                    build_environment_id(reg, tags));
                break;
            case hdc3020::Registers::AUTO_MEASURE_4M1S:
                poller.continuous_multi_register_poll(
                    hdc3020::ADDRESS, reg, 1, mode_lookup(_registers.measure_mode_4m1s), RESPONSE_SIZE, 250, own_queue,
                    build_environment_id(reg, tags));
                break;
            case hdc3020::Registers::AUTO_MEASURE_10M1S:
                poller.continuous_multi_register_poll(
                    hdc3020::ADDRESS, reg, 1, mode_lookup(_registers.measure_mode_10m1s), RESPONSE_SIZE, 100, own_queue,
                    build_environment_id(reg, tags));
                break;
            default:
                break;
        }

    }

    std::tuple<int32_t> parse_hdc3020_data(int64_t data) {
        // TODO maybe we should send over an error
        // when the crc value detects an error
        auto humidity_crc = data & 0xFFFFFF000000;
        auto humidity = humidity_crc - (humidity_crc & CRC_8);
        auto temperature_crc = data & 0x000000FFFFFF;
        auto temperature = temperature_crc - (temperature_crc & CRC_8);
        return std::make_tuple(humidity, temperature);
    }

    void send_hdc3020_data(std::tuple<int32_t> humidity_temp) {
        auto hum_message = can::messages::ReadFromSensorResponse{
            .sensor = SensorType::humidity, .sensor_data = humidity_temp[0]};
        can_client.send_can_message(get_host_id(), hum_message);
        auto temp_message = can::messages::ReadFromSensorResponse{
            .sensor = SensorType::temperature, .sensor_data = humidity_temp[1]};
        can_client.send_can_message(get_host_id(), temp_message);
    }

    void handle_response(const i2c::messages::TransactionResponse &tm) {
        int64_t data = 0x0;
        const auto *iter = tm.read_buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, tm.read_buffer.cend(), data);
        auto humidity_temp = parse_hdc3020_data(data);
        switch (static_cast<hdc3020::Registers>(utils::reg_from_id(tm.id.token))) {
            case hdc3020::Registers::TRIGGER_ON_DEMAND_MODE:
                _registers.trigger_measurement.humidity = humidity_temp[0];
                _registers.trigger_measurement.temperature = humidity_temp[1];
            case hdc3020::Registers::AUTO_MEASURE_1M2S:
                _registers.measure_mode_1m2s.humidity = humidity_temp[0];
                _registers.measure_mode_1m2s.temperature = humidity_temp[1];
            case hdc3020::Registers::AUTO_MEASURE_1M1S:
                _registers.measure_mode_1m1s.humidity = humidity_temp[0];
                _registers.measure_mode_1m1s.temperature = humidity_temp[1];
            case hdc3020::Registers::AUTO_MEASURE_2M1S:
                _registers.measure_mode_2m1s.humidity = humidity_temp[0];
                _registers.measure_mode_2m1s.temperature = humidity_temp[1];
            case hdc3020::Registers::AUTO_MEASURE_4M1S:
                _registers.measure_mode_4m1s.humidity = humidity_temp[0];
                _registers.measure_mode_4m1s.temperature = humidity_temp[1];
            case hdc3020::Registers::AUTO_MEASURE_10M1S:
                _registers.measure_mode_10m1s.humidity = humidity_temp[0];
                _registers.measure_mode_10m1s.temperature = humidity_temp[1];
            default:
                if (report) {
                    send_hdc3020_data(humidity_temp);
                }
                break;
        }
    }

  private:
    hdc3020::HDC3020RegisterMap _registers{};
    bool _initialized = false;
    bool report = true;
    constexpr uint64_t CRC_8 = 0x31;
    constexpr uint8_t RESPONSE_SIZE = 6;
    constexpr uint16_t MINIMUM_DELAY_MS = 3;
    // (TODO: lc 7-26-2022) we need a can message that
    // allows a user to select the power consumption mode
    // to use for different commands.
    hdc3020::LowPowerMode POWER_MODE = LowPowerMode::ONE;
    I2CQueuePoller &poller;
    CanClient &can_client;
    OwnQueue &own_queue;
    const can::ids::SensorId &sensor_id;

    unsigned int build_environment_id(hdc3020::Registers reg, std::array tags) {
        return utils::build_id(hdc3020::ADDRESS, reg, utils::byte_from_tags(tags));
    }

    template <HDC3020CommandRegister Reg>
    hdc3020::Registers mode_lookup(Reg& primary_register) {
        return hdc3020::Registers(primary_register::mode_map[static_cast<uint8_t>(POWER_MODE)]);

    }
};

} // namespace tasks
} // namespace sensors