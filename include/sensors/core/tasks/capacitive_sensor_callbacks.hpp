#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "sensors/core/callback_types.hpp"
#include "sensors/core/fdc1004.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"

namespace capacitance_callbacks {
using namespace fdc1004_utils;
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
                            sensor_hardware::SensorHardwareBase &hardware,
                            int32_t threshold, float current_offset)
        : can_client{can_client},
          i2c_writer{i2c_writer},
          hardware{hardware},
          current_offset{current_offset},
          zero_threshold{threshold} {}

    void handle_data_ongoing(
        const sensor_callbacks::MaxMessageBuffer &MSB_buffer,
        const sensor_callbacks::MaxMessageBuffer &LSB_buffer) {
        auto cap_raw_val = capacitance_from_buffers(MSB_buffer, LSB_buffer);
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

    void handle_data_limited(
        const sensor_callbacks::MaxMessageBuffer &MSB_buffer,
        const sensor_callbacks::MaxMessageBuffer &LSB_buffer) {
        measurement += capacitance_from_buffers(MSB_buffer, LSB_buffer);
    }

    void send_to_can() {
        int32_t capacitance =
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
    uint32_t capacitance_from_buffers(
        const sensor_callbacks::MaxMessageBuffer &MSB_buffer,
        const sensor_callbacks::MaxMessageBuffer &LSB_buffer) {
        uint16_t msb_data = 0x0;
        uint16_t lsb_data = 0x0;
        const auto *MSB_iter = MSB_buffer.cbegin();
        MSB_iter =
            bit_utils::bytes_to_int(MSB_iter, MSB_buffer.cend(), msb_data);
        const auto *LSB_iter = LSB_buffer.cbegin();
        LSB_iter =
            bit_utils::bytes_to_int(LSB_iter, LSB_buffer.cend(), lsb_data);
        return convert_reads(msb_data, lsb_data);
    }

    CanClient &can_client;
    I2CQueueWriter &i2c_writer;
    sensor_hardware::SensorHardwareBase &hardware;
    float current_offset;
    int32_t zero_threshold;
    int32_t measurement = 0;
    uint16_t number_of_reads = 1;
    bool echoing = false;
    bool bind_sync = false;
};

// This struct should be used when the message handler
// class receives information it should handle (i.e. the device info)
struct InternalCallback {
    std::array<uint16_t, 1> storage{};

    void handle_data(const sensor_callbacks::MaxMessageBuffer &buffer) {
        uint16_t data = 0x0;
        const auto *iter = buffer.cbegin();
        // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
        storage[0] = data;
    }

    void send_to_can() {}

    auto get_storage() -> uint16_t { return storage[0]; }
};
};  // namespace capacitance_callbacks
