#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "sensors/core/callback_types.hpp"
#include "sensors/core/hdc2080.hpp"

namespace environment_sensor_callbacks {
using namespace hdc2080_utils;
using namespace can_ids;

template <message_writer_task::TaskClient CanClient>
struct HumidityReadingCallback {
  public:
    HumidityReadingCallback(CanClient &can_client) : can_client{can_client} {}

    void handle_data(const sensor_callbacks::MaxMessageBuffer &buffer) {
        uint16_t data = 0x0;
        const auto *iter = buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
        humidity = convert(data, SensorType::humidity);
    }

    void send_to_can() {
        auto message = can_messages::ReadFromSensorResponse{
            {}, SensorType::humidity, humidity};
        can_client.send_can_message(can_ids::NodeId::host, message);
        humidity = 0;
    }

  private:
    CanClient &can_client;
    uint32_t humidity = 0;
};

template <message_writer_task::TaskClient CanClient>
struct TemperatureReadingCallback {
  public:
    TemperatureReadingCallback(CanClient &can_client)
        : can_client{can_client} {}

    void handle_data(const sensor_callbacks::MaxMessageBuffer &buffer) {
        uint16_t data = 0x0;
        const auto *iter = buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
        temperature = convert(data, SensorType::temperature);

    }

    void send_to_can() {
        auto message = can_messages::ReadFromSensorResponse{
            {}, SensorType::temperature, temperature};
        can_client.send_can_message(can_ids::NodeId::host, message);
        temperature = 0;
    }

  private:
    CanClient &can_client;
    uint32_t temperature = 0;
};

// TODO (lc: 02-24-2022 pull into its own shared file)
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
};

}  // namespace environment_sensor_callbacks