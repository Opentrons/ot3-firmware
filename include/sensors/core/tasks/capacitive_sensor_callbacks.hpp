#pragma once

#include <array>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "sensors/core/fdc1004.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {
using namespace fdc1004;
using namespace can::ids;

/*
 * ReadCapacitanceCallback.
 *
 * Struct to take a capacitance reading. The first part of sensor
 * data is stored in the MSB register and the second part is in the
 * LSB register. The size of the data is 24 bits.
 */
template <can::message_writer_task::TaskClient CanClient, class I2CQueueWriter>
struct ReadCapacitanceCallback {
  public:
    ReadCapacitanceCallback(CanClient &can_client, I2CQueueWriter &i2c_writer,
                            hardware::SensorHardwareBase &hardware,
                            can::ids::SensorId &id)
        : can_client{can_client},
          i2c_writer{i2c_writer},
          hardware{hardware},
          sensor_id{id} {}

    void handle_ongoing_response(i2c::messages::TransactionResponse &m) {
        static_cast<void>(bit_utils::bytes_to_int(
            m.read_buffer.cbegin(), m.read_buffer.cend(),

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            polling_results[m.id.transaction_index]));
        if (m.id.transaction_index == 0) {
            return;
        }
        auto capacitance = convert_capacitance(
            convert_reads(polling_results[0], polling_results[1]), 1,
            current_offset_pf);
        // TODO (lc 1-2-2022) we should figure out a better strategy for adjusting
        // the capdac. I am leaving this commented out as a reminder.
//        auto new_offset = update_offset(capacitance, current_offset_pf);
//        set_offset(new_offset);
        if (bind_sync) {
            if (capacitance > zero_threshold_pf) {
                hardware.set_sync();
            } else {
                hardware.reset_sync();
            }
        }
        if (echoing) {
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ReadFromSensorResponse{
                    .sensor = can::ids::SensorType::capacitive,
                    .sensor_id = sensor_id,
                    .sensor_data = convert_to_fixed_point(capacitance, 16)});
        }
    }

    void handle_baseline_response(i2c::messages::TransactionResponse &m) {
        static_cast<void>(bit_utils::bytes_to_int(
            m.read_buffer.cbegin(), m.read_buffer.cend(),
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            baseline_results[m.id.transaction_index]));
        if (m.id.transaction_index == 0) {
            return;
        }
        measurement += convert_reads(baseline_results[0], baseline_results[1]);
        if (!m.id.is_completed_poll) {
            return;
        }
        auto capacitance = convert_capacitance(measurement, number_of_reads,
                                               current_offset_pf);
        if (utils::tag_in_token(m.id.token,
                                utils::ResponseTag::IS_THRESHOLD_SENSE)) {
            set_threshold(capacitance + next_autothreshold_pf,
                          can::ids::SensorThresholdMode::auto_baseline);
        } else {
            auto message = can::messages::ReadFromSensorResponse{
                .sensor = SensorType::capacitive,
                .sensor_id = sensor_id,
                .sensor_data = convert_to_fixed_point(capacitance, 16)};
            can_client.send_can_message(can::ids::NodeId::host, message);
        }
        // TODO (lc 1-2-2022) we should figure out a better strategy for adjusting
        // the capdac. I am leaving this commented out as a reminder.
//        auto new_offset = update_offset(capacitance, current_offset_pf);
//        set_offset(new_offset);
    }

    void reset_limited() {
        number_of_reads = 1;
        measurement = 0;
    }

    void set_number_of_reads(uint16_t number_of_reads) {
        this->number_of_reads = number_of_reads;
    }

    [[nodiscard]] auto get_offset() const -> float { return current_offset_pf; }

    void set_echoing(bool should_echo) { echoing = should_echo; }

    void set_bind_sync(bool should_bind) {
        bind_sync = should_bind;
        hardware.reset_sync();
    }

    void set_offset(float new_offset) {
        if (new_offset != current_offset_pf) {
            auto capdac_raw = get_capdac_raw(new_offset);
            std::array offset{CONFIGURATION_MEASUREMENT,
                              device_configuration_msb(capdac_raw),
                              device_configuration_lsb(capdac_raw)};
            i2c_writer.write(ADDRESS, offset);
            current_offset_pf = new_offset;
            std::array configuration_data{FDC_CONFIGURATION, SAMPLE_RATE_MSB,
                                          SAMPLE_RATE_LSB};
            i2c_writer.write(ADDRESS, configuration_data);
        }
    }

    void initialize() {
        current_offset_pf = -1;
        set_offset(0);
    }

    auto prime_autothreshold(float next_autothresh_pf) -> void {
        next_autothreshold_pf = next_autothresh_pf;
    }

    auto set_threshold(float threshold_pf,
                       can::ids::SensorThresholdMode from_mode) -> void {
        zero_threshold_pf = threshold_pf;
        auto message = can::messages::SensorThresholdResponse{
            .sensor = SensorType::capacitive,
            .sensor_id = sensor_id,
            .threshold = convert_to_fixed_point(zero_threshold_pf, 16),
            .mode = from_mode};
        can_client.send_can_message(can::ids::NodeId::host, message);
    }

    [[nodiscard]] auto get_threshold() const -> float {
        return zero_threshold_pf;
    }

  private:
    CanClient &can_client;
    I2CQueueWriter &i2c_writer;
    hardware::SensorHardwareBase &hardware;
    can::ids::SensorId sensor_id;
    float current_offset_pf = 0;
    float zero_threshold_pf = 30;
    float next_autothreshold_pf = 0;
    int32_t measurement = 0;
    uint16_t number_of_reads = 1;
    bool echoing = false;
    bool bind_sync = false;
    std::array<uint16_t, 2> baseline_results{};
    std::array<uint16_t, 2> polling_results{};
};
};  // namespace tasks
};  // namespace sensors
