#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.hpp"
#include "sensors/core/callback_types.hpp"
#include "sensors/core/mmr920C04_registers.hpp"

namespace pressure_sensor_callbacks {
using namespace mmr920C04_utils;
using namespace can_ids;
/*
 * ReadPressureCallback.
 *
 * Struct to take a capacitance reading. The first part of sensor
 * data is stored in the MSB register and the second part is in the
 * LSB register. The size of the data is 24 bits.
 */
template <message_writer_task::TaskClient CanClient, class I2CQueueWriter>
struct ReadPressureCallback {
  public:
    ReadPressureCallback(CanClient &can_client, I2CQueueWriter &i2c_writer,
                         uint32_t threshold, float current_offset)
        : can_client{can_client},
          i2c_writer{i2c_writer},
          current_offset{current_offset},
          zero_threshold{threshold} {}

    void handle_data(const sensor_callbacks::MaxMessageBuffer &MSB_buffer,
                     const sensor_callbacks::MaxMessageBuffer &LSB_buffer) {
        uint16_t msb_data = 0x0;
        uint16_t lsb_data = 0x0;
        const auto *MSB_iter = MSB_buffer.cbegin();
        MSB_iter =
            bit_utils::bytes_to_int(MSB_iter, MSB_buffer.cend(), msb_data);
        const auto *LSB_iter = LSB_buffer.cbegin();
        LSB_iter =
            bit_utils::bytes_to_int(LSB_iter, LSB_buffer.cend(), lsb_data);
        measurement += (msb_data << MSB_SHIFT | lsb_data);
    }

    void send_to_can() {
        uint32_t pressure =
            convert_pressure(measurement, number_of_reads, current_offset);
        auto message = can_messages::ReadFromSensorResponse{
            {}, SensorType::pressure, pressure};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    void reset() {
        number_of_reads = 1;
        measurement = 0;
    }

    void set_number_of_reads(uint16_t number_of_reads) {
        this->number_of_reads = number_of_reads;
    }

    auto get_offset() -> float { return current_offset; }

  private:
    CanClient &can_client;
    I2CQueueWriter &i2c_writer;
    float current_offset;
    uint32_t zero_threshold;
    uint32_t measurement = 0;
    uint16_t number_of_reads = 1;
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

} // pressure_sensor_callbacks