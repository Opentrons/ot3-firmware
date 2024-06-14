#pragma once

#include <array>

#ifdef ENABLE_CROSS_ONLY_HEADERS
// TODO(fps 7/12/2023): This is super hacky and I hate throwing #ifdefs
// in our nicely host-independent code but for now we really just need
// the vTaskDelay function and hopefully sometime in the near future I
// can refactor this file with a nice templated sleep function.
#include "FreeRTOS.h"
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define HACKY_TASK_SLEEP(___timeout___) vTaskDelay(___timeout___)

#else

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define HACKY_TASK_SLEEP(___timeout___) (void)(___timeout___)
#endif

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "common/core/sensor_buffer.hpp"
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
            sensors::hardware::SensorHardwareBase &hardware, bool shared_sensor,
            std::array<float, SENSOR_BUFFER_SIZE> *sensor_buffer)
        : writer(writer),
          poller(poller),
          can_client(can_client),
          own_queue(own_queue),
          hardware(hardware),
          shared_sensor(shared_sensor),
          sensor_buffer(sensor_buffer) {}

    [[nodiscard]] auto initialized() const -> bool { return _initialized; }

    auto initialize() -> void {
        if (!_initialized) {
            // FIXME we should grab device id and compare it to the static
            // device id in code.

            // We should send a message that the sensor is in a ready state,
            // not sure if we should have a separate can message to do that
            // holding off for this PR.

            // Initial delay to avoid I2C bus traffic.
            HACKY_TASK_SLEEP(100);
            update_capacitance_configuration();
            // Second delay to ensure IC is ready to start
            // readings (and also to avoid I2C bus traffic).
            HACKY_TASK_SLEEP(100);
            set_sample_rate();
            _initialized = true;
        }
    }

    auto register_map() -> fdc1004::FDC1004RegisterMap & { return _registers; }

    auto get_host_id() -> NodeId { return NodeId::host; }

    auto get_can_client() -> CanClient & { return can_client; }

    auto get_sensor_id() -> can::ids::SensorId { return sensor_id; }

    auto set_sensor_id(can::ids::SensorId _id) -> void {
        if (shared_sensor && sensor_id != _id) {
            if (_id == can::ids::SensorId::S1) {
                measure_mode = fdc1004::MeasureConfigMode::TWO;
            } else {
                measure_mode = fdc1004::MeasureConfigMode::ONE;
            }
            sensor_id = _id;
            update_capacitance_configuration();
        }
    }

    [[nodiscard]] auto get_offset() const -> float { return current_offset_pf; }

    void set_echoing(bool should_echo) {
        echoing = should_echo;
        if (should_echo) {
            sensor_buffer_index = 0;  // reset buffer index
        }
    }

    void set_bind_sync(bool should_bind) {
        bind_sync = should_bind;
        hardware.reset_sync();
    }

    void set_max_bind_sync(bool should_bind) {
        max_capacitance_sync = should_bind;
        hardware.reset_sync();
    }

    auto set_bind_flags(uint8_t binding) -> void { sensor_binding = binding; }

    auto set_measurement_rate(fdc1004::MeasurementRate rate) -> void {
        measurement_rate = rate;
    }

    auto update_capacitance_configuration() {
        // You should call sensor_id before calling this
        // function to ensure you're reading from the correct
        // mode.
        current_offset_pf = -1;
        set_offset(0);
    }

    void reset_limited() {
        number_of_reads = 1;
        measurement = 0;
    }

    void set_number_of_reads(uint16_t number_of_reads) {
        this->number_of_reads = number_of_reads;
    }

    void set_sample_rate() {
        _registers.fdc_conf.measurement_rate =
            static_cast<uint8_t>(measurement_rate);
        _registers.fdc_conf.repeating_measurements = 1;
        // Enable both measurements no matter what. We check the Ready
        // bits anyways and the data doesn't overwrite, so there's no danger.
        _registers.fdc_conf.measure_mode_1 = 1;
        _registers.fdc_conf.measure_mode_2 = 1;
        _registers.fdc_conf.padding_0 = 0;
        _registers.fdc_conf.padding_1 = 0;
        set_register(_registers.fdc_conf);
    }

    void set_offset(float new_offset) {
        new_offset = std::max(new_offset, 0.0F);
        if (new_offset != current_offset_pf) {
            auto capdac_raw = fdc1004_utils::get_capdac_raw(new_offset);
            if (measure_mode == fdc1004::MeasureConfigMode::TWO) {
                auto mode_2_register = build_mode_2_configuration_register();
                mode_2_register.CAPDAC = capdac_raw;
                set_register(mode_2_register);
            } else {
                auto mode_1_register = build_mode_1_configuration_register();
                mode_1_register.CAPDAC = capdac_raw;
                set_register(mode_1_register);
            }
            current_offset_pf = new_offset;
        }
    }

    auto write(fdc1004::Registers reg, uint16_t command_data) -> bool {
        return writer.write(fdc1004::ADDRESS, static_cast<uint8_t>(reg),
                            command_data);
    }

    auto write(fdc1004::Registers reg) -> bool {
        return writer.write(fdc1004::ADDRESS, static_cast<uint8_t>(reg));
    }

    auto transact(fdc1004::Registers reg) -> bool {
        std::array reg_buf{static_cast<uint8_t>(reg)};
        return writer.transact(
            fdc1004::ADDRESS, reg_buf, 4, own_queue,
            utils::build_id(fdc1004::ADDRESS, static_cast<uint8_t>(reg)));
    }

    auto poll_limited_capacitance(uint16_t number_reads, can::ids::SensorId _id,
                                  uint8_t tags) -> void {
        set_sensor_id(_id);
        poller.single_register_poll(
            fdc1004::ADDRESS,
            static_cast<uint8_t>(fdc1004::Registers::FDC_CONF), 2, number_reads,
            DELAY, own_queue,
            utils::build_id(fdc1004::ADDRESS,
                            static_cast<uint8_t>(fdc1004::Registers::FDC_CONF),
                            tags));
    }

    auto poll_continuous_capacitance(can::ids::SensorId _id, uint8_t tags,
                                     uint8_t binding) -> void {
        set_sensor_id(_id);
        auto delay = delay_or_disable(binding);
        poller.continuous_single_register_poll(
            fdc1004::ADDRESS,
            static_cast<uint8_t>(fdc1004::Registers::FDC_CONF), 2, delay,
            own_queue,
            utils::build_id(fdc1004::ADDRESS,
                            static_cast<uint8_t>(fdc1004::Registers::FDC_CONF),
                            tags));
    }

    auto stop_continuous_polling(uint32_t transaction_id) -> void {
        poller.continuous_single_register_poll(
            fdc1004::ADDRESS,
            static_cast<uint8_t>(fdc1004::Registers::FDC_CONF), 2, STOP_DELAY,
            own_queue, transaction_id);
    }

    void send_accumulated_sensor_data(uint32_t message_index) {
#ifdef USE_SENSOR_MOVE
        for (int i = 0; i < sensor_buffer_index; i++) {
            // send over buffer adn then clear buffer values
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ReadFromSensorResponse{
                    .message_index = message_index,
                    .sensor = can::ids::SensorType::capacitive,
                    .sensor_id = sensor_id,
                    .sensor_data = convert_to_fixed_point(
                        (*sensor_buffer).at(i), S15Q16_RADIX)});
            if (i % 10 == 0) {
                // slow it down so the can buffer doesn't choke
                vTaskDelay(50);
            }
            (*sensor_buffer).at(i) = 0;
        }
#else
        std::ignore = message_index;
#endif
    }

    void handle_fdc_response(i2c::messages::TransactionResponse &m) {
        uint16_t reg_int = 0;
        static_cast<void>(bit_utils::bytes_to_int(
            m.read_buffer.cbegin(), m.read_buffer.cend(), reg_int));
        auto fdc = read_register<fdc1004::FDCConf>(reg_int).value();
        auto measurement_done =
            fdc1004_utils::measurement_ready(fdc, measure_mode);
        if (measurement_done) {
            // Start a single-shot, two-register transaction for the data.
            send_followup(m);
        } else {
            // Double check that the configuration reg seems okay. If not, we
            // should reconfigure it before re-requesting a reading.
            if (!check_fdc_readback_ok(fdc)) {
                update_capacitance_configuration();
                set_sample_rate();
            }
            i2c::messages::MaxMessageBuffer buffer = {
                static_cast<uint8_t>(fdc1004::Registers::FDC_CONF)};
            // Retrigger the same exact reading
            writer.transact(fdc1004::ADDRESS, 1, buffer, 2, m.id, own_queue);
        }
    }

    void handle_ongoing_response(i2c::messages::TransactionResponse &m) {
        static_cast<void>(bit_utils::bytes_to_int(
            m.read_buffer.cbegin(), m.read_buffer.cend(),

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            polling_results[m.id.transaction_index]));
        if (m.id.transaction_index == 0) {
            send_followup(m);
            return;
        }

        if (!bind_sync && !echoing && !max_capacitance_sync) {
            stop_continuous_polling(m.id.token);
            return;
        }

        auto raw_capacitance = fdc1004_utils::convert_reads(polling_results[0],
                                                            polling_results[1]);

        auto capacitance = fdc1004_utils::convert_capacitance(
            raw_capacitance, 1, current_offset_pf);

        auto new_offset =
            fdc1004_utils::update_offset(capacitance, current_offset_pf);
        set_offset(new_offset);

        if (max_capacitance_sync) {
            if (capacitance > fdc1004::MAX_CAPACITANCE_READING) {
                hardware.set_sync();
            } else {
                hardware.reset_sync();
            }
        }
        if (bind_sync) {
            if (capacitance > zero_threshold_pf) {
                hardware.set_sync();
            } else {
                hardware.reset_sync();
            }
        }

        if (echoing) {
#ifdef USE_SENSOR_MOVE
            // send a response with 9999 to make an overload of the buffer
            // visible
            if (sensor_buffer_index < SENSOR_BUFFER_SIZE) {
                (*sensor_buffer).at(sensor_buffer_index) = capacitance;
                sensor_buffer_index++;
            }
#else
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ReadFromSensorResponse{
                    .message_index = m.message_index,
                    .sensor = can::ids::SensorType::capacitive,
                    .sensor_id = sensor_id,
                    .sensor_data =
                        convert_to_fixed_point(capacitance, S15Q16_RADIX)});
#endif
        }
    }

    void handle_baseline_response(i2c::messages::TransactionResponse &m) {
        static_cast<void>(bit_utils::bytes_to_int(
            m.read_buffer.cbegin(), m.read_buffer.cend(),
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            baseline_results[m.id.transaction_index]));
        if (m.id.transaction_index == 0) {
            send_followup(m);
            return;
        }
        measurement += fdc1004_utils::convert_reads(baseline_results[0],
                                                    baseline_results[1]);
        if (!m.id.is_completed_poll) {
            return;
        }
        auto capacitance = fdc1004_utils::convert_capacitance(
            measurement, number_of_reads, current_offset_pf);
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
        auto new_offset =
            fdc1004_utils::update_offset(capacitance, current_offset_pf);
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
    static constexpr uint16_t STOP_DELAY = 0;
    can::ids::SensorId sensor_id = can::ids::SensorId::S0;
    fdc1004::MeasureConfigMode measure_mode = fdc1004::MeasureConfigMode::ONE;
    fdc1004::MeasurementRate measurement_rate =
        fdc1004::MeasurementRate::ONE_HUNDRED_SAMPLES_PER_SECOND;
    bool shared_sensor = false;

    float current_offset_pf = 0;
    float zero_threshold_pf = 30;
    float next_autothreshold_pf = 0;
    int32_t measurement = 0;
    uint16_t number_of_reads = 1;
    bool echoing = false;
    bool bind_sync = false;
    bool max_capacitance_sync = false;
    std::array<uint16_t, 2> baseline_results{};
    std::array<uint16_t, 2> polling_results{};

    auto send_followup(i2c::messages::TransactionResponse &m) -> void {
        auto address = utils::reg_from_id<uint8_t>(m.id.token);
        uint8_t followup_address = 0;
        auto tags = utils::tags_from_token(m.id.token);
        i2c::messages::TransactionIdentifier id = m.id;
        switch (address) {
            case static_cast<uint8_t>(fdc1004::Registers::FDC_CONF):
                // After CONF, read MSB for transaction index 0
                followup_address =
                    (measure_mode == fdc1004::MeasureConfigMode::ONE)
                        ? static_cast<uint8_t>(fdc1004::Registers::MEAS1_MSB)
                        : static_cast<uint8_t>(fdc1004::Registers::MEAS2_MSB);
                id.transaction_index = 0;
                break;
            case static_cast<uint8_t>(fdc1004::Registers::MEAS1_MSB):
                // After MSB, read LSB for index 1
                followup_address =
                    static_cast<uint8_t>(fdc1004::Registers::MEAS1_LSB);
                id.transaction_index = 1;
                break;
            case static_cast<uint8_t>(fdc1004::Registers::MEAS2_MSB):
                // After MSB, read LSB for index 1
                followup_address =
                    static_cast<uint8_t>(fdc1004::Registers::MEAS2_LSB);
                id.transaction_index = 1;
                break;
            default:
                // If this isn't one of the hardcoded regs, do nothing
                return;
        }

        i2c::messages::MaxMessageBuffer buffer;
        buffer[0] = followup_address;

        // Overwrite the token to get the updated address
        id.token = utils::build_id(fdc1004::ADDRESS, followup_address, tags);
        // This maintains the entire transaction ID of the original request.
        writer.transact(fdc1004::ADDRESS, 1, buffer, 2, id, own_queue);
    }

    auto build_mode_1_configuration_register() -> fdc1004::ConfMeasure1 {
        _registers.config_measure_1.CHA =
            static_cast<uint8_t>(fdc1004::CHA::CIN1);
        _registers.config_measure_1.CHB =
            static_cast<uint8_t>(fdc1004::CHB::CAPDAC);
        return _registers.config_measure_1;
    }

    auto build_mode_2_configuration_register() -> fdc1004::ConfMeasure2 {
        _registers.config_measure_2.CHA =
            static_cast<uint8_t>(fdc1004::CHA::CIN2);
        _registers.config_measure_2.CHB =
            static_cast<uint8_t>(fdc1004::CHB::CAPDAC);
        return _registers.config_measure_2;
    }

    auto build_mode_3_configuration_register() -> fdc1004::ConfMeasure3 {
        _registers.config_measure_3.CHA =
            static_cast<uint8_t>(fdc1004::CHA::CIN3);
        _registers.config_measure_3.CHB =
            static_cast<uint8_t>(fdc1004::CHB::CAPDAC);
        return _registers.config_measure_3;
    }

    auto build_mode_4_configuration_register() -> fdc1004::ConfMeasure4 {
        _registers.config_measure_4.CHA =
            static_cast<uint8_t>(fdc1004::CHA::CIN4);
        _registers.config_measure_4.CHB =
            static_cast<uint8_t>(fdc1004::CHB::CAPDAC);
        return _registers.config_measure_4;
    }

    auto delay_or_disable(uint8_t binding) -> uint8_t {
        if (binding ==
            static_cast<uint8_t>(can::ids::SensorOutputBinding::none)) {
            return 0;
        }
        return DELAY;
    }

    auto check_fdc_readback_ok(fdc1004::FDCConf fdc) -> bool {
        auto &comp = _registers.fdc_conf;
        // Check everything except for the readback status
        return (comp.measure_mode_4 == fdc.measure_mode_4 &&
                comp.measure_mode_3 == fdc.measure_mode_3 &&
                comp.measure_mode_2 == fdc.measure_mode_2 &&
                comp.measure_mode_1 == fdc.measure_mode_1 &&
                comp.repeating_measurements == fdc.repeating_measurements &&
                comp.measurement_rate == fdc.measurement_rate);
    }

    template <fdc1004::FDC1004Register Reg>
    requires fdc1004::WritableRegister<Reg>
    auto set_register(Reg &reg) -> bool {
        auto value =
            // Ignore the typical linter warning because we're only using
            // this on __packed structures that mimic hardware registers
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            *reinterpret_cast<fdc1004::RegisterSerializedTypeA *>(&reg);
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
    template <fdc1004::FDC1004Register Reg>
    requires fdc1004::ReadableRegister<Reg>
    auto read_register(uint16_t data) -> std::optional<Reg> {
        using RT = std::optional<fdc1004::RegisterSerializedType>;
        using RG = std::optional<Reg>;

        auto ret = RT(data);
        if (!ret.has_value()) {
            return RG();
        }
        // Ignore the typical linter warning because we're only using
        // this on __packed structures that mimic hardware registers
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return RG(*reinterpret_cast<Reg *>(&ret.value()));
    }
    std::array<float, SENSOR_BUFFER_SIZE> *sensor_buffer;
    uint16_t sensor_buffer_index = 0;

};  // end of FDC1004 class

}  // namespace tasks
}  // namespace sensors
