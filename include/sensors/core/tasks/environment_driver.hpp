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


namespace sensors {

namespace tasks {

using namespace can::ids;

template <class I2CQueueWriter, class I2CQueuePoller,
          can::message_writer_task::TaskClient CanClient, class OwnQueue>
class HDC3020 {
  public:
    HDC3020(I2CQueueWriter &writer, I2CQueuePoller &poller,
            CanClient &can_client, OwnQueue &own_queue,
            sensors::hardware::SensorHardwareBase &hardware,
            const can::ids::SensorId &id)
        : writer(writer),
          poller(poller),
          can_client(can_client),
          own_queue(own_queue),
          hardware(hardware),
          sensor_id(id) {}

    [[nodiscard]] auto initialized() const -> bool { return _initialized; }

    auto register_map() -> hdc3020::HDC3020RegisterMap & {
        return _registers;
    }

    void trigger_on_demand() {}

    void auto_measure_mode() {
        // use poller
        poller.continuous_multi_register_poll(ADDRESS, REGISTER);
    }

    void handle_response() {
        switch (reg) { case Registers::TRIGGER_ON_DEMAND_MODE: }
    }

  private:
    hdc3020::HDC3020RegisterMap _registers{};
    bool _initialized = false;
    I2CQueueWriter &writer;
    I2CQueuePoller &poller;
    CanClient &can_client;
    OwnQueue &own_queue;
    hardware::SensorHardwareBase &hardware;
    const can::ids::SensorId &sensor_id;
};

} // namespace tasks
} // namespace sensors