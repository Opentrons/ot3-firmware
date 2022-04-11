#pragma once

#include <optional>

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "i2c/core/messages.hpp"
#include "sensors/core/mmr920C04.hpp"
#include "sensors/core/sensors.hpp"

namespace sensors {

namespace tasks {

using namespace can_ids;
using namespace mmr920C04;
using namespace registers;

/**
 * Pressure sensor driver class. It takes in a i2c writer queue and
 * can client queue so that it can handle sensor readings internally.
 *
 * @tparam I2CQueueWriter
 * @tparam CanClient
 */

template <class I2CQueueWriter, class I2CQueuePoller,
          message_writer_task::TaskClient CanClient, class OwnQueue>
class MMR92C04 {
  public:
    MMR92C04(I2CQueueWriter &writer, I2CQueuePoller &poller,
             CanClient &can_client, OwnQueue &own_queue)
        : writer(writer),
          poller(poller),
          can_client(can_client),
          own_queue(own_queue) {}

    /**
     * @brief Check if the MMR92C04 has been initialized.
     * @return true if the config registers have been written at least once,
     * false otherwise.
     */
    [[nodiscard]] auto initialized() const -> bool { return _initialized; }

    auto register_map() -> MMR920C04RegisterMap & { return _registers; }

    auto get_sensor_id() -> SensorType { return SensorType::pressure; }

    auto get_host_id() -> NodeId { return NodeId::host; }

    auto get_threshold() -> int32_t { return threshold_cmH20; }

    auto set_threshold(int32_t new_threshold) -> void {
        threshold_cmH20 = new_threshold;
    }

    auto write(Registers reg, uint32_t command_data) -> void {
        writer.write(ADDRESS, static_cast<uint8_t>(reg), command_data);
    }

    auto transact(Registers reg) -> void {
        std::array reg_buf{static_cast<uint8_t>(reg)};
        writer.transact(ADDRESS, reg_buf, 4, own_queue,
                        utils::build_id(ADDRESS, static_cast<uint8_t>(reg)));
    }

    auto poll_read(Registers reg, uint16_t number_reads) -> void {
        std::array reg_buf{static_cast<uint8_t>(reg)};
        poller.single_register_poll(
            ADDRESS, reg_buf, 4, number_reads, DELAY, own_queue,
            utils::build_id(
                ADDRESS, static_cast<uint8_t>(reg),
                utils::byte_from_tag(utils::ResponseTag::IS_PART_OF_POLL)));
    }

    auto write_config() -> bool {
        if (!reset(_registers.reset)) {
            return false;
        }
        if (!set_measure_mode(Registers::MEASURE_MODE_4)) {
            return false;
        }
        _initialized = true;
        return true;
    }

    auto set_measure_mode(Registers reg) -> bool {
        switch (reg) {
            case Registers::MEASURE_MODE_1:
                if (set_register(_registers.measure_mode_1)) {
                    return true;
                }
                return false;
            case Registers::MEASURE_MODE_2:
                if (set_register(_registers.measure_mode_2)) {
                    return true;
                }
                return false;
            case Registers::MEASURE_MODE_3:
                if (set_register(_registers.measure_mode_3)) {
                    return true;
                }
                return false;
            case Registers::MEASURE_MODE_4:
                if (set_register(_registers.measure_mode_4)) {
                    return true;
                }
                return false;
            default:
                return false;
        }
    }

    auto get_pressure(Registers reg, bool poll = false,
                      uint16_t sample_rate = 0) -> void {
        if (poll) {
            poll_read(reg, sample_rate);
        } else {
            transact(reg);
        }
    }

    auto get_temperature(bool poll = false, uint16_t sample_rate = 0) -> void {
        if (poll) {
            poll_read(Registers::TEMPERATURE_READ, sample_rate);
        } else {
            transact(Registers::TEMPERATURE_READ);
        }
    }

    auto reset(Reset reg) -> bool {
        if (set_register(reg)) {
            _registers.reset = reg;
            return true;
        }
        return false;
    }

    auto read_pressure(uint32_t data) -> bool {
        LOG("Updated pressure reading is %d", data);
        if (data) {
            _registers.pressure.reading = data;
            return true;
        }
        return false;
    }

    auto read_pressure_low_pass(uint32_t data) -> bool {
        if (data) {
            _registers.low_pass_pressure.reading = data;
            return true;
        }
        return false;
    }

    auto read_temperature(uint32_t data) -> bool {
        if (data) {
            _registers.temperature.reading = data;
            return true;
        }
        return false;
    }

    auto read_status(uint32_t data) -> bool {
        if (data) {
            _registers.status.reading = data & Status::value_mask;
            return true;
        }
        return false;
    }

    auto send_pressure() -> void {
        auto pressure = Pressure::to_pressure(_registers.pressure.reading);
        auto message = can_messages::ReadFromSensorResponse{
            .sensor = get_sensor_id(), .sensor_data = pressure};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_pressure_low_pass() -> void {
        auto pressure =
            LowPassPressure::to_pressure(_registers.low_pass_pressure.reading);
        auto message = can_messages::ReadFromSensorResponse{
            .sensor = get_sensor_id(), .sensor_data = pressure};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_temperature() -> void {
        auto temperature =
            Temperature::to_temperature(_registers.low_pass_pressure.reading);
        auto message = can_messages::ReadFromSensorResponse{
            .sensor = get_sensor_id(), .sensor_data = temperature};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_status() -> void {
        auto status = Status::to_status(_registers.status.reading);
        auto message = can_messages::ReadFromSensorResponse{
            .sensor = get_sensor_id(),
            .sensor_data = static_cast<int32_t>(status)};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_threshold() -> void {
        auto message = can_messages::SensorThresholdResponse{
            .sensor = get_sensor_id(), .threshold = get_threshold()};
        can_client.send_can_message(get_host_id(), message);
    }

    auto handle_response(const i2c::messages::TransactionResponse &tm) {
        uint32_t data = 0x0;
        const auto *iter = tm.read_buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, tm.read_buffer.cend(), data);

        switch (utils::reg_from_id<Registers>(tm.id.token)) {
            case Registers::PRESSURE_READ:
                read_pressure(data);
                send_pressure();
                break;
            case Registers::LOW_PASS_PRESSURE_READ:
                read_pressure_low_pass(data);
                send_pressure_low_pass();
                break;
            case Registers::TEMPERATURE_READ:
                read_temperature(data);
                send_temperature();
                break;
            case Registers::STATUS:
                read_status(data);
                send_status();
                break;
            case Registers::RESET:
            case Registers::IDLE:
            case Registers::MEASURE_MODE_1:
            case Registers::MEASURE_MODE_2:
            case Registers::MEASURE_MODE_3:
            case Registers::MEASURE_MODE_4:
            case Registers::MACRAM_WRITE:
                break;
        }
    }

  private:
    MMR920C04RegisterMap _registers{};
    bool _initialized = false;
    int32_t threshold_cmH20 = 0x8;
    const uint16_t DELAY = 20;
    I2CQueueWriter &writer;
    I2CQueuePoller &poller;
    CanClient &can_client;
    OwnQueue &own_queue;

    template <MMR920C04Register Reg>
    requires WritableRegister<Reg>
    auto set_register(Reg reg) -> bool {
        write(Reg::address, 0x0);
        return true;
    }
};
};  // namespace tasks
};  // namespace sensors
