#pragma once

#include <cmath>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "i2c/core/messages.hpp"
#include "sensors/core/mmr920C04.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/sensors.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {

namespace tasks {

using namespace can::ids;

/**
 * Pressure sensor driver class. It takes in a i2c writer queue and
 * can client queue so that it can handle sensor readings internally.
 *
 * @tparam I2CQueueWriter
 * @tparam CanClient
 */

template <class I2CQueueWriter, class I2CQueuePoller,
          can::message_writer_task::TaskClient CanClient, class OwnQueue>
class MMR920C04 {
  public:
    MMR920C04(I2CQueueWriter &writer, I2CQueuePoller &poller,
              CanClient &can_client, OwnQueue &own_queue,
              sensors::hardware::SensorHardwareBase &hardware,
              const can::ids::SensorId &id)
        : writer(writer),
          poller(poller),
          can_client(can_client),
          own_queue(own_queue),
          hardware(hardware),
          sensor_id(id) {
        hardware.add_data_ready_callback(
            [this]() -> void { this->sensor_callback(); });
    }

    /**
     * @brief Check if the MMR92C04 has been initialized.
     * @return true if the config registers have been written at least once,
     * false otherwise.
     */
    [[nodiscard]] auto initialized() const -> bool { return _initialized; }

    auto register_map() -> mmr920C04::MMR920C04RegisterMap & {
        return _registers;
    }

    auto get_sensor_type() -> SensorType { return SensorType::pressure; }

    auto get_host_id() -> NodeId { return NodeId::host; }

    auto get_threshold() -> int32_t { return threshold_pascals; }

    auto set_threshold(int32_t new_threshold) -> void {
        threshold_pascals = fixed_point_to_float(new_threshold, S15Q16_RADIX);
    }

    auto write(mmr920C04::Registers reg, uint32_t command_data) -> void {
        writer.write(mmr920C04::ADDRESS, static_cast<uint8_t>(reg),
                     command_data);
    }

    auto write(mmr920C04::Registers reg) -> void {
        writer.write(mmr920C04::ADDRESS, static_cast<uint8_t>(reg));
    }

    auto transact(mmr920C04::Registers reg) -> void {
        std::array reg_buf{static_cast<uint8_t>(reg)};
        writer.transact(
            mmr920C04::ADDRESS, reg_buf, 4, own_queue,
            utils::build_id(mmr920C04::ADDRESS, static_cast<uint8_t>(reg)));
    }

    auto poll_read(mmr920C04::Registers reg, uint16_t number_reads) -> void {
        std::array reg_buf{static_cast<uint8_t>(reg)};
        poller.single_register_poll(
            mmr920C04::ADDRESS, reg_buf, 4, number_reads, DELAY, own_queue,
            utils::build_id(
                mmr920C04::ADDRESS, static_cast<uint8_t>(reg),
                utils::byte_from_tag(utils::ResponseTag::IS_PART_OF_POLL)));
    }

    auto write_config() -> bool {
        if (!reset(_registers.reset)) {
            return false;
        }
        if (!set_measure_mode(mmr920C04::Registers::RESET)) {
            return false;
        }
        _initialized = true;
        return true;
    }

    auto set_measure_mode(mmr920C04::Registers reg) -> bool {
        switch (reg) {
            case mmr920C04::Registers::MEASURE_MODE_1:
                if (set_register(_registers.measure_mode_1)) {
                    return true;
                }
                return false;
            case mmr920C04::Registers::MEASURE_MODE_2:
                if (set_register(_registers.measure_mode_2)) {
                    return true;
                }
                return false;
            case mmr920C04::Registers::MEASURE_MODE_3:
                if (set_register(_registers.measure_mode_3)) {
                    return true;
                }
                return false;
            case mmr920C04::Registers::MEASURE_MODE_4:
                if (set_register(_registers.measure_mode_4)) {
                    return true;
                }
                return false;
            case mmr920C04::Registers::TEMPERATURE_READ:
                if (set_register(_registers.temperature)) {
                    return true;
                }
                return false;
            default:
                return false;
        }
    }

    auto get_pressure() -> bool {
        read_register = mmr920C04::Registers::PRESSURE_READ;
        return set_measure_mode(mmr920C04::Registers::MEASURE_MODE_4);
    }

    auto get_temperature() -> bool {
        read_register = mmr920C04::Registers::TEMPERATURE_READ;
        return set_measure_mode(mmr920C04::Registers::MEASURE_MODE_4);
    }

    auto reset(mmr920C04::Reset reg) -> bool {
        if (set_register(reg)) {
            _registers.reset = reg;
            return true;
        }
        return false;
    }

    auto read_pressure(uint32_t data) -> bool {
        LOG("Updated pressure reading is %u", data);
        _registers.pressure.reading = data;
        return true;
    }

    auto read_pressure_low_pass(uint32_t data) -> bool {
        _registers.low_pass_pressure.reading = data;
        return true;
    }

    auto read_temperature(uint32_t data) -> bool {
        _registers.temperature.reading = data;
        return true;
    }

    auto read_status(uint32_t data) -> bool {
        _registers.status.reading = data & mmr920C04::Status::value_mask;
        return true;
    }

    auto send_pressure(uint32_t message_index) -> void {
        // Pressure is sent via CAN in pascals as a fixed point value
        auto pressure_pascals =
            mmr920C04::Pressure::to_pressure(_registers.pressure.reading);
        auto pressure_fixed_point =
            convert_to_fixed_point(pressure_pascals, S15Q16_RADIX);
        auto message = can::messages::ReadFromSensorResponse{
            .message_index = message_index,
            .sensor = get_sensor_type(),
            .sensor_data = pressure_fixed_point};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_baseline_response(uint32_t message_index) -> void {
        auto offset_fixed_point =
            convert_to_fixed_point(offset_average, S15Q16_RADIX);
        auto message = can::messages::BaselineSensorResponse{
            .message_index = message_index,
            .sensor = get_sensor_type(),
            .offset_average = offset_fixed_point};
        auto host_id = get_host_id();
        can_client.send_can_message(host_id, message);
    }

    auto send_pressure_low_pass(uint32_t message_index) -> void {
        auto pressure = mmr920C04::LowPassPressure::to_pressure(
            _registers.low_pass_pressure.reading);
        auto pressure_fixed_point =
            convert_to_fixed_point(pressure, S15Q16_RADIX);
        auto message = can::messages::ReadFromSensorResponse{
            .message_index = message_index,
            .sensor = get_sensor_type(),
            .sensor_data = pressure_fixed_point};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_temperature(uint32_t message_index) -> void {
        auto temperature = mmr920C04::Temperature::to_temperature(
            _registers.low_pass_pressure.reading);
        auto message = can::messages::ReadFromSensorResponse{
            .message_index = message_index,
            .sensor = get_sensor_type(),
            .sensor_data = temperature};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_status(uint32_t message_index) -> void {
        auto status = mmr920C04::Status::to_status(_registers.status.reading);
        auto message = can::messages::ReadFromSensorResponse{
            .message_index = message_index,
            .sensor = get_sensor_type(),
            .sensor_id = sensor_id,
            .sensor_data = static_cast<int32_t>(status)};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_threshold(uint32_t message_index) -> void {
        auto message = can::messages::SensorThresholdResponse{
            .message_index = message_index,
            .sensor = get_sensor_type(),
            .sensor_id = sensor_id,
            .threshold = get_threshold()};
        can_client.send_can_message(get_host_id(), message);
    }

    auto send_peripheral_response(uint32_t message_index) -> void {
        auto message = can::messages::PeripheralStatusResponse{
            .message_index = message_index,
            .sensor = get_sensor_type(),
            .sensor_id = sensor_id,
            .status = initialized()};
        can_client.send_can_message(get_host_id(), message);
    }

    auto sensor_callback() -> void {
        writer.transact_isr(mmr920C04::ADDRESS,
                            static_cast<uint8_t>(read_register),
                            static_cast<std::size_t>(3), own_queue,
                            static_cast<uint8_t>(read_register));
        if (limited_poll && !stop_polling) {
            number_of_reads++;
            if (number_of_reads >= total_baseline_reads) {
                stop_polling = true;
            }
        }
        if (stop_polling) {
            writer.write_isr(mmr920C04::ADDRESS,
                             static_cast<uint8_t>(mmr920C04::Registers::RESET));
        }
    }

    auto set_sync_bind(SensorOutputBinding binding) -> void {
        hardware.reset_sync();
        set_sync(binding);
        set_report(binding);
        set_stop_polling(binding);
    }

    void set_baseline_values(uint16_t number_of_reads) {
        this->total_baseline_reads = number_of_reads;
        this->number_of_reads = 0;
        this->running_total = 0;
        this->offset_average = 0;
    }

    void set_limited_poll(bool _limited) { limited_poll = _limited; }

    auto handle_response(const i2c::messages::TransactionResponse &tm) {
        uint32_t raw_data = 0x00;
        int32_t data = 0x0;
        const auto *iter = tm.read_buffer.cbegin();
        // Pressure is always a three-byte value
        iter = bit_utils::bytes_to_int(iter, tm.read_buffer.cend(), raw_data);
        data = static_cast<int32_t>(raw_data >> 8);
        auto pressure_pascals = mmr920C04::Pressure::to_pressure(data);
        switch (static_cast<mmr920C04::Registers>(tm.id.token)) {
            case mmr920C04::Registers::PRESSURE_READ:
                read_pressure(data);
                if (sync) {
                    if (std::fabs(pressure_pascals) -
                            std::fabs(offset_average) >
                        threshold_pascals) {
                        hardware.set_sync();
                        stop_polling = true;
                    } else {
                        hardware.reset_sync();
                    }
                }
                if (report && !limited_poll) {
                    send_pressure(tm.message_index);
                }
                if (limited_poll) {
                    running_total += pressure_pascals;
                    offset_average =
                        running_total / static_cast<float>(number_of_reads);
                    if (number_of_reads == total_baseline_reads) {
                        send_baseline_response(tm.message_index);
                        set_baseline_values(0);
                    }
                }
                break;
            case mmr920C04::Registers::LOW_PASS_PRESSURE_READ:
                read_pressure_low_pass(data);
                send_pressure_low_pass(tm.message_index);
                break;
            case mmr920C04::Registers::TEMPERATURE_READ:
                read_temperature(data);
                send_temperature(tm.message_index);
                break;
            case mmr920C04::Registers::STATUS:
                read_status(data);
                send_status(tm.message_index);
                break;
            case mmr920C04::Registers::RESET:
            case mmr920C04::Registers::IDLE:
            case mmr920C04::Registers::MEASURE_MODE_1:
            case mmr920C04::Registers::MEASURE_MODE_2:
            case mmr920C04::Registers::MEASURE_MODE_3:
            case mmr920C04::Registers::MEASURE_MODE_4:
            case mmr920C04::Registers::MACRAM_WRITE:
                break;
        }
    }

    auto get_can_client() -> CanClient & { return can_client; }

  private:
    mmr920C04::MMR920C04RegisterMap _registers{};
    bool _initialized = false;
    bool stop_polling = true;
    bool sync = false;
    bool report = true;
    bool limited_poll = true;
    uint16_t number_of_reads = 0x1;
    float running_total = 0;
    uint16_t total_baseline_reads = 0x8;
    // TODO(fs, 2022-11-11): Need to figure out a realistic threshold. Pretty
    // sure this is an arbitrarily large number to enable continuous reads.
    float threshold_pascals = 100.0F;
    float offset_average = 0;
    const uint16_t DELAY = 20;
    mmr920C04::Registers read_register = mmr920C04::Registers::PRESSURE_READ;
    I2CQueueWriter &writer;
    I2CQueuePoller &poller;
    CanClient &can_client;
    OwnQueue &own_queue;
    hardware::SensorHardwareBase &hardware;
    const can::ids::SensorId &sensor_id;

    template <mmr920C04::MMR920C04Register Reg>
    requires registers::WritableRegister<Reg>
    auto set_register(Reg) -> bool {
        write(Reg::address);
        return true;
    }

    auto set_sync(SensorOutputBinding binding) -> void {
        sync = (static_cast<uint8_t>(binding) &
                static_cast<uint8_t>(SensorOutputBinding::sync)) ==
               static_cast<uint8_t>(SensorOutputBinding::sync);
    }

    auto set_report(SensorOutputBinding binding) -> void {
        report = (static_cast<uint8_t>(binding) &
                  static_cast<uint8_t>(SensorOutputBinding::report)) ==
                 static_cast<uint8_t>(SensorOutputBinding::report);
    }

    auto set_stop_polling(SensorOutputBinding binding) -> void {
        stop_polling = (static_cast<uint8_t>(binding) ==
                        static_cast<uint8_t>(SensorOutputBinding::none));
    }
};
};  // namespace tasks
};  // namespace sensors
