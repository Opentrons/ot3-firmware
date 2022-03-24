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

    void handle_data(const sensor_callbacks::MaxMessageBuffer &buffer_lsb, const sensor_callbacks::MaxMessageBuffer &buffer_msb) {
        uint8_t lsb = 0x0;
        uint8_t msb = 0x0;
        const auto *lsbiter = buffer_lsb.cbegin();
        lsbiter = bit_utils::bytes_to_int(lsbiter, buffer_lsb.cend(), lsb);
        const auto *msbiter = buffer_msb.cbegin();
        msbiter = bit_utils::bytes_to_int(msbiter, buffer_msb.cend(), msb);
        LOG("humidity Data received from buffer %d %d", lsb, msb);
        uint16_t data = static_cast<uint16_t>(msb) << 8 | lsb;
        humidity = convert(data, SensorType::humidity);
        LOG("Handling data data received %d", humidity);
    }

    void send_to_can() {
        LOG("Sending humidity data %d", humidity);
        auto message = can_messages::ReadFromSensorResponse{
            .sensor = SensorType::humidity, .sensor_data = humidity};
        can_client.send_can_message(can_ids::NodeId::host, message);
        humidity = 0;
    }

  private:
    CanClient &can_client;
    int32_t humidity = 0;
};

template <message_writer_task::TaskClient CanClient>
struct TemperatureReadingCallback {
  public:
    TemperatureReadingCallback(CanClient &can_client)
        : can_client{can_client} {}

    void handle_data(const sensor_callbacks::MaxMessageBuffer &buffer_lsb, const sensor_callbacks::MaxMessageBuffer &buffer_msb) {
        uint8_t lsb = 0x0;
        uint8_t msb = 0x0;
        const auto *lsbiter = buffer_lsb.cbegin();
        lsbiter = bit_utils::bytes_to_int(lsbiter, buffer_lsb.cend(), lsb);
        const auto *msbiter = buffer_msb.cbegin();
        msbiter = bit_utils::bytes_to_int(msbiter, buffer_msb.cend(), msb);
        LOG("temperature Data received from buffer %d %d", lsb, msb);
        uint16_t data = static_cast<uint16_t>(msb) << 8 | lsb;
        temperature = convert(data, SensorType::temperature);
        LOG("Handling temperature data received %d", temperature);
    }

    void send_to_can() {
        auto message = can_messages::ReadFromSensorResponse{
            .sensor = SensorType::temperature, .sensor_data = temperature};
        can_client.send_can_message(can_ids::NodeId::host, message);
        temperature = 0;
    }

  private:
    CanClient &can_client;
    int32_t temperature = 0;
};

// TODO (lc: 02-24-2022 pull into its own shared file)
// This struct should be used when the message handler
// class receives information it should handle (i.e. the device info)
struct InternalCallback {
    std::array<uint16_t, 1> storage{};

    void handle_data(const sensor_callbacks::MaxMessageBuffer &buffer) {
        uint32_t data = 0x0;
        const auto *iter = buffer.cbegin();
        // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
        storage[0] = data;
    }

    void send_to_can() {}
};

}  // namespace environment_sensor_callbacks