#pragma once

#include <array>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "sensors/core/fdc1004.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensors {
namespace tasks {
using namespace fdc1004;
using namespace can_ids;

/*
 * ReadCapacitanceCallback.
 *
 * Struct to take a capacitance reading. The first part of sensor
 * data is stored in the MSB register and the second part is in the
 * LSB register. The size of the data is 24 bits.
 */
template <message_writer_task::TaskClient CanClient, class I2CQueueWriter>
struct ReadCapacitanceCallback {
  public:
    ReadCapacitanceCallback(CanClient &can_client, I2CQueueWriter &i2c_writer,
                            hardware::SensorHardwareBase &hardware,
                            int32_t threshold, float current_offset)
        : can_client{can_client},
          i2c_writer{i2c_writer},
          hardware{hardware},
          current_offset{current_offset},
          zero_threshold{threshold} {}

    void handle_ongoing_response(i2c::messages::TransactionResponse &m) {
        static_cast<void>(bit_utils::bytes_to_int(
            m.read_buffer.cbegin(), m.read_buffer.cend(),
            polling_results[m.id.transaction_index]));
        if (m.id.transaction_index == 0) {
            return;
        }
        auto cap_raw_val =
            convert_reads(polling_results[0], polling_results[1]);
        auto cap_val = convert_capacitance(cap_raw_val, 1, current_offset);
        if (bind_sync) {
            if (cap_val > zero_threshold) {
                hardware.set_sync();
            } else {
                hardware.reset_sync();
            }
        }
        if (echoing) {
            can_client.send_can_message(
                can_ids::NodeId::host,
                can_messages::ReadFromSensorResponse{
                    .sensor = can_ids::SensorType::capacitive,
                    .sensor_data = cap_val});
        }
    }

    void handle_baseline_response(i2c::messages::TransactionResponse &m) {
        static_cast<void>(bit_utils::bytes_to_int(
            m.read_buffer.cbegin(), m.read_buffer.cend(),
            baseline_results[m.id.transaction_index]));
        if (m.id.transaction_index == 0) {
            return;
        }
        measurement += convert_reads(baseline_results[0], baseline_results[1]);
        if (!m.id.is_completed_poll) {
            return;
        }
        auto capacitance =
            convert_capacitance(measurement, number_of_reads, current_offset);
        auto message = can_messages::ReadFromSensorResponse{
            .sensor = SensorType::capacitive, .sensor_data = capacitance};
        can_client.send_can_message(can_ids::NodeId::host, message);
        if (capacitance > zero_threshold || capacitance < zero_threshold) {
            LOG("Capacitance %d exceeds zero threshold %d ", capacitance,
                zero_threshold);
            auto capdac = update_capdac(capacitance, current_offset);
            // convert back to pF
            current_offset = get_offset_pf(capdac);
            LOG("Setting offset to %d ", static_cast<int>(current_offset));
            uint16_t update = CONFIGURATION_MEASUREMENT |
                              POSITIVE_INPUT_CHANNEL | NEGATIVE_INPUT_CHANNEL |
                              capdac;
            i2c_writer.write(ADDRESS, update);
        }
    }

    void reset_limited() {
        number_of_reads = 1;
        measurement = 0;
    }

    void set_number_of_reads(uint16_t number_of_reads) {
        this->number_of_reads = number_of_reads;
    }

    auto get_offset() -> float { return current_offset; }

    void set_echoing(bool should_echo) { echoing = should_echo; }

    void set_bind_sync(bool should_bind) { bind_sync = should_bind; }

  private:
    CanClient &can_client;
    I2CQueueWriter &i2c_writer;
    hardware::SensorHardwareBase &hardware;
    float current_offset;
    int32_t zero_threshold;
    int32_t measurement = 0;
    uint16_t number_of_reads = 1;
    bool echoing = false;
    bool bind_sync = false;
    std::array<uint16_t, 2> baseline_results{};
    std::array<uint16_t, 2> polling_results{};
};
};  // namespace tasks
};  // namespace sensors
