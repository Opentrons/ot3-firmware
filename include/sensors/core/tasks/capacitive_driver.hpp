#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "i2c/core/messages.hpp"
#include "sensors/core/fdc1004.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {

namespace tasks {

using namespace can::ids;

template <class I2CQueueWriter, class I2CQueuePoller,
          can::message_writer_task::TaskClient CanClient, class OwnQueue>
class FDC1004 {
  public:
    FDC1004(I2CQueueWriter &writer, I2CQueuePoller &poller,
            CanClient &can_client, OwnQueue &own_queue,
            sensors::hardware::SensorHardwareBase &hardware,
            bool shared_sensor)
        : writer(writer),
          poller(poller),
          can_client(can_client),
          own_queue(own_queue),
          hardware(hardware),
		  shared_sensor(shared_sensor) {}

	[[nodiscard]] auto initialized() const -> bool { return _initialized; }

    auto initialize(can::ids::SensorId _id) -> void {
		// FIXME we should grab device id and compare it to the static
        // device id in code.

        // We should send a message that the sensor is in a ready state,
        // not sure if we should have a separate can message to do that
        // holding off for this PR.
        if (shared_sensor) {
            set_sensor_id(can::ids::SensorId::S1);
            update_capacitance_configuration();
        }
        set_sensor_id(_id);
		update_capacitance_configuration();
        is_initialized = true;
    }

	auto register_map() -> fdc1004::FDC1004RegisterMap & { return _registers; }

    auto get_host_id() -> NodeId { return NodeId::host; }

    auto get_can_client() -> CanClient & { return can_client; }

    auto get_sensor_id() -> can::ids::SensorId { return sensor_id; }

	auto set_sensor_id(can::ids::SensorId _id) -> void { 
		if (shared_sensor && sensor_id != _id) {
			_initialized = false;
            if _id == can::ids::SensorId::S1:
                measure_mode = MeasureConfigMode::TWO;
            else:
                measure_mode = MeasureConfigMode::ONE;
			update_capacitance_configuration();
		}
		sensor_id = _id; 
	}

    [[nodiscard]] auto get_offset() const -> float { return current_offset_pf; }

    void set_echoing(bool should_echo) { echoing = should_echo; }

    void set_bind_sync(bool should_bind) {
        bind_sync = should_bind;
        hardware.reset_sync();
    }

    auto set_bind_flags(uint8_t binding) -> void { sensor_binding = binding; }

    auto set_measurement_rate(MeasurementRate rate) -> void { measurement_rate = rate; }

	auto update_capacitance_configuration() {
        // You should call sensor_id before calling this
        // function to ensure you're reading from the correct
        // mode.
		current_offset_pf = -1;
        set_offset(0);
        set_sample_rate();
	}

    void reset_limited() {
        number_of_reads = 1;
        measurement = 0;
    }

    void set_number_of_reads(uint16_t number_of_reads) {
        this->number_of_reads = number_of_reads;
    }

    void set_sample_rate() {
        _registers.fdc_conf.measurement_rate = measurement_rate;
        _registers.fdc_conf.repeating_measurements = 0x1;
        if (measure_mode == MeasureConfigMode::TWO) {
            _registers.measure_mode_1 = 0x0;
            _registers.measure_mode_2 = 0x1;
        } else {
            _registers.measure_mode_1 = 0x1;
            _registers.measure_mode_2 = 0x0;
        }
        set_register(_registers.fdc_conf);
    }

    void set_offset(float new_offset) {
        new_offset = std::max(new_offset, 0.0F);
        if (new_offset != current_offset_pf) {
            auto capdac_raw = fdc1004_utils::get_capdac_raw(new_offset);
            if (measure_mode == MeasureConfigMode::TWO) {
                auto mode_2_register = build_mode_2_configuration_register();
                mode_2_register.CAPDAC = capdac_raw;
                set_register(_registers.mode_2_register)
            } else {
                auto mode_1_register = build_mode_1_configuration_register();
                mode_1_register.CAPDAC = capdac_raw;
                set_register(_registers.mode_1_register)
            }
            current_offset_pf = new_offset;
        }

    }

    auto write(fdc1004::Registers reg, uint32_t command_data) -> void {
        writer.write(fdc1004::ADDRESS, static_cast<uint8_t>(reg),
                     command_data);
    }

    auto write(fdc1004::Registers reg) -> void {
        writer.write(fdc1004::ADDRESS, static_cast<uint8_t>(reg));
    }

    auto transact(fdc1004::Registers reg) -> void {
        std::array reg_buf{static_cast<uint8_t>(reg)};
        writer.transact(
            fdc1004::ADDRESS, reg_buf, 4, own_queue,
            utils::build_id(fdc1004::ADDRESS, static_cast<uint8_t>(reg)));
    }

    auto poll_limited_capacitance(uint16_t number_reads, can::ids::SensorId _id, std::array tags) -> void {
        // TODO add tags here
        set_sensor_id(_id);
        if (measure_mode == fdc1004::MeasureConfigMode::TWO) {
            poller.multi_register_poll(
                fdc1004::ADDRESS, fdc1004::Registers::MEAS2_MSB, 2, fdc1004::Registers::MEAS2_LSB, 2, 1, DELAY,
                own_queue, utils::build_id(fdc1004::ADDRESS, fdc1004::Registers::MEAS2_MSB, 1));
        } else {
            poller.multi_register_poll(
                fdc1004::ADDRESS, fdc1004::Registers::MEAS1_MSB, 2, fdc1004::Registers::MEAS1_LSB, 2, 1, DELAY,
                own_queue, utils::build_id(fdc1004::ADDRESS, fdc1004::Registers::MEAS1_MSB, 1));
        }

    }

    auto poll_continuous_capacitance(can::ids::SensorId _id, std::array tags) -> void {
        // TODO add tags here
        set_sensor_id(_id);
        if (measure_mode == fdc1004::MeasureConfigMode::TWO) {
            poller.continuous_multi_register_poll(
                fdc1004::ADDRESS, fdc1004::Registers::MEAS2_MSB, 2, fdc1004::Registers::MEAS2_LSB, 2, 1, DELAY,
                own_queue, utils::build_id(fdc1004::ADDRESS, fdc1004::Registers::MEAS2_MSB, utils::byte_from_tags(tags)));
        } else {
            poller.continuous_multi_register_poll(
                fdc1004::ADDRESS, fdc1004::Registers::MEAS1_MSB, 2, fdc1004::Registers::MEAS1_LSB, 2, 1, DELAY,
                own_queue, utils::build_id(fdc1004::ADDRESS, fdc1004::Registers::MEAS1_MSB, utils::byte_from_tags(tags)));
        }
    }

    void handle_ongoing_response(i2c::messages::TransactionResponse &m) {
        static_cast<void>(bit_utils::bytes_to_int(
            m.read_buffer.cbegin(), m.read_buffer.cend(),

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            polling_results[m.id.transaction_index]));
        if (m.id.transaction_index == 0) {
            return;
        }
        auto capacitance = fdc1004_utils::convert_capacitance(
            fdc1004_utils::convert_reads(polling_results[0], polling_results[1]), 1,
            current_offset_pf);
        // TODO (lc 1-2-2022) we should figure out a better strategy for
        // adjusting the capdac.
        auto new_offset = fdc1004_utils::update_offset(capacitance, current_offset_pf);
        set_offset(new_offset);
        if (bind_sync) {
            if (capacitance > zero_threshold_pf) {
                hardware.set_sync();
            } else {
                hardware.reset_sync();
            }
        }
        if (echoing) {
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ReadFromSensorResponse{
                    .message_index = m.message_index,
                    .sensor = can::ids::SensorType::capacitive,
                    .sensor_id = sensor_id,
                    .sensor_data =
                        convert_to_fixed_point(capacitance, S15Q16_RADIX)});
        }
    }

    void handle_baseline_response(i2c::messages::TransactionResponse &m) {
        static_cast<void>(bit_utils::bytes_to_int(
            m.read_buffer.cbegin(), m.read_buffer.cend(),
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            baseline_results[m.id.transaction_index]));
        if (m.id.transaction_index == 0) {
            return;
        }
        measurement += fdc1004_utils::convert_reads(baseline_results[0], baseline_results[1]);
        if (!m.id.is_completed_poll) {
            return;
        }
        auto capacitance = fdc1004_utils::convert_capacitance(measurement, number_of_reads,
                                               current_offset_pf);
        if (utils::tag_in_token(m.id.token,
                                utils::ResponseTag::IS_THRESHOLD_SENSE)) {
            set_threshold(capacitance + next_autothreshold_pf,
                          can::ids::SensorThresholdMode::auto_baseline,
                          m.message_index);
        } else {
            auto message = can::messages::ReadFromSensorResponse{
                .message_index = m.message_index,
                .sensor = SensorType::capacitive,
                .sensor_id = sensor_id,
                .sensor_data =
                    convert_to_fixed_point(capacitance, S15Q16_RADIX)};
            can_client.send_can_message(can::ids::NodeId::host, message);
        }
        // TODO (lc 1-2-2022) we should figure out a better strategy for
        // adjusting the capdac.
        auto new_offset = fdc1004_utils::update_offset(capacitance, current_offset_pf);
        set_offset(new_offset);
    }

    auto prime_autothreshold(float next_autothresh_pf) -> void {
        next_autothreshold_pf = next_autothresh_pf;
    }

    auto set_threshold(float threshold_pf,
                       can::ids::SensorThresholdMode from_mode,
                       uint32_t message_index) -> void {
        zero_threshold_pf = threshold_pf;
        auto message = can::messages::SensorThresholdResponse{
            .message_index = message_index,
            .sensor = SensorType::capacitive,
            .sensor_id = sensor_id,
            .threshold =
                convert_to_fixed_point(zero_threshold_pf, S15Q16_RADIX),
            .mode = from_mode};
        can_client.send_can_message(can::ids::NodeId::host, message);
    }

    [[nodiscard]] auto get_threshold() const -> float {
        return zero_threshold_pf;
    }

  private:
    I2CQueueWriter &writer;
    I2CQueuePoller &poller;
    CanClient &can_client;
    OwnQueue &own_queue;
    hardware::SensorHardwareBase &hardware;

	uint8_t sensor_binding{2};
	fdc1004::FDC1004RegisterMap _registers{};
    bool _initialized = false;

    static constexpr uint16_t DELAY = 20;
    can::ids::SensorId sensor_id = can::ids::SensorId::S0;
    fdc1004::MeasureConfigMode measure_mode = fdc1004::MeasureConfigMode::ONE;
    fdc1004::MeasurementRate measurement_rate = fdc1004::MeasurementRate::ONE_HUNDRED_SAMPLES_PER_SECOND;
	bool shared_sensor = false;

    auto build_mode_1_configuration_register() -> ConfMeasure1 {
        _registers.config_measure_1.CHA = CHA::CIN1;
        _registers.config_measure_1.CHB = CHB::CAPDAC;
        return _registers.config_measure_1
    }
    auto build_mode_2_configuration_register() -> ConfMeasure2 {
        _registers.config_measure_2.CHA = CHA::CIN2;
        _registers.config_measure_2.CHB = CHB::CAPDAC;
        return _registers.config_measure_2
    }
    auto build_mode_3_configuration_register() -> ConfMeasure3 {
        _registers.config_measure_3.CHA = CHA::CIN3;
        _registers.config_measure_3.CHB = CHB::CAPDAC;
        return _registers.config_measure_3

    }
    auto build_mode_4_configuration_register() -> ConfMeasure4 {
        _registers.config_measure_4.CHA = CHA::CIN4;
        _registers.config_measure_4.CHB = CHB::CAPDAC;
        return _registers.config_measure_4
    }


    template <FDC1004Register Reg>
    requires WritableRegister<Reg>
    auto set_register(Reg& reg) -> bool {
        // Ignore the typical linter warning because we're only using
        // this on __packed structures that mimic hardware registers
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto value = *reinterpret_cast<RegisterSerializedTypeA*>(&reg);
        value &= Reg::value_mask;
        return write(Reg::address, value);
    }
    /**
     * @brief Read a register on the TMC2130
     *
     * @tparam Reg The type of register to read
     * @tparam Policy Abstraction class for actual writing
     * @param[in] policy Instance of the abstraction policy to use
     * @return The contents of the register, or nothing if the register
     * can't be read.
     */
    template <FDC1004Register Reg>
    requires ReadableRegister<Reg>
    auto read_register(uint32_t data) -> std::optional<Reg> {
        using RT = std::optional<RegisterSerializedType>;
        using RG = std::optional<Reg>;

        auto ret = RT(data);
        if (!ret.has_value()) {
            return RG();
        }
        // Ignore the typical linter warning because we're only using
        // this on __packed structures that mimic hardware registers
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return RG(*reinterpret_cast<Reg*>(&ret.value()));
    }


}  // namespace tasks
}  // namespace sensors
