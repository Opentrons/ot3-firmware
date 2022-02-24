#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "sensors/core/callback_types.hpp"
#include "sensors/core/fdc1004.hpp"

namespace capacitance_callbacks {
using namespace fdc1004_utils;
using namespace can_ids;

template <message_writer_task::TaskClient CanClient>
struct ReadCapacitanceCallback
    : public sensor_callbacks::MultiRegisterCallback {
  public:
    ReadCapacitanceCallback(CanClient &can_client, float current_offset)
        : can_client{can_client}, current_offset{current_offset} {}

    void operator()(const sensor_callbacks::MaxMessageBuffer &MSB_buffer,
                    const sensor_callbacks::MaxMessageBuffer &LSB_buffer) override {
        uint32_t msb_data = 0x0;
        uint32_t lsb_data = 0x0;
        const auto *MSB_iter = MSB_buffer.cbegin();
        MSB_iter =
            bit_utils::bytes_to_int(MSB_iter, MSB_buffer.cend(), msb_data);
        const auto *LSB_iter = LSB_buffer.cbegin();
        LSB_iter =
            bit_utils::bytes_to_int(LSB_iter, LSB_buffer.cend(), lsb_data);
        measurement += (msb_data << MSB_SHIFT | lsb_data);
    }

    void operator()() override {
        uint32_t capacitive =
            convert(measurement, number_of_reads, current_offset);
        auto message = can_messages::ReadFromSensorResponse{
            {}, SensorType::capacitive, capacitive};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    void reset() {
        number_of_reads = 1;
        measurement = 0;
    }

    void set_number_of_reads(uint16_t number_of_reads) {
        number_of_reads = number_of_reads;
    }

    void set_offset(float new_offset) { current_offset = new_offset; }

  private:
    CanClient &can_client;
    float current_offset;
    uint32_t measurement = 0;
    uint16_t number_of_reads = 1;
};

template <message_writer_task::TaskClient CanClient>
struct ReadOffsetCallback : public sensor_callbacks::SingleRegisterCallback {
  public:
    ReadOffsetCallback(CanClient &can_client, float current_offset)
        : can_client{can_client}, current_offset{current_offset} {}

    void operator()(const sensor_callbacks::MaxMessageBuffer &buffer) override {
        uint16_t data = 0x0;
        const auto *iter = buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
    }

    void operator()() override {
        uint32_t offset =
            calculate_capdac(measurement, number_of_reads, current_offset);
        auto message = can_messages::ReadFromSensorResponse{
            {}, SensorType::capacitive, offset};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    void reset() {
        measurement = 0;
        number_of_reads = 1;
    }

    void set_number_of_reads(uint16_t number_of_reads) {
        number_of_reads = number_of_reads;
    }

    void set_offset(float new_offset) { current_offset = new_offset; }

  private:
    CanClient &can_client;
    float current_offset;
    uint32_t measurement = 0;
    uint16_t number_of_reads = 1;
};

// This struct should be used when the message handler
// class receives information it should handle (i.e. the device info)
struct InternalCallback : public sensor_callbacks::SingleRegisterCallback {
    std::array<uint16_t, 1> storage{};

    void operator()(const sensor_callbacks::MaxMessageBuffer &buffer) override {
        uint16_t data = 0x0;
        const auto *iter = buffer.cbegin();
        // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
        storage[0] = data;
    }

    void operator()() override {}

    auto get_storage() -> uint16_t { return storage[0]; }
};
};  // namespace capacitance_callbacks