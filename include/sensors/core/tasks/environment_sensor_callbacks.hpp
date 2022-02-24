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
struct HumidityReadingCallback
    : public sensor_callbacks::SingleRegisterCallback {
  public:
    HumidityReadingCallback(CanClient &can_client) : can_client{can_client} {}

    void operator()(const sensor_callbacks::MaxMessageBuffer &buffer) override {
        uint16_t data = 0x0;
        const auto *iter = buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
        auto humidity = convert(data, SensorType::humidity);

        auto message = can_messages::ReadFromSensorResponse{
            {}, SensorType::humidity, static_cast<uint32_t>(humidity)};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    void operator()() override {}

  private:
    CanClient &can_client;
};

template <message_writer_task::TaskClient CanClient>
struct TemperatureReadingCallback
    : public sensor_callbacks::SingleRegisterCallback {
  public:
    TemperatureReadingCallback(CanClient &can_client)
        : can_client{can_client} {}

    void operator()(const sensor_callbacks::MaxMessageBuffer &buffer) override {
        uint16_t data = 0x0;
        const auto *iter = buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
        auto temperature = convert(data, SensorType::temperature);

        auto message = can_messages::ReadFromSensorResponse{
            {}, SensorType::temperature, static_cast<uint32_t>(temperature)};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    void operator()() override {}

  private:
    CanClient &can_client;
};

// TODO (lc: 02-24-2022 pull into its own shared file)
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
};

}  // namespace environment_sensor_callbacks