namespace sensors {
namespace fdc1004 {

// Capacitance Sensor Address and Registers
constexpr uint16_t ADDRESS = 0x50 << 1;

enum class Registers : uint8_t {
    MEAS1_MSB = 0x0,
    MEAS1_LSB = 0x01,
    MEAS2_MSB = 0x02,
    MEAS2_LSB = 0x03,
    MEAS3_MSB = 0x04,
    MEAS3_LSB = 0x05,
    MEAS4_MSB = 0x06,
    MEAS4_LSB = 0x07,
    CONF_MEAS1 = 0x08,
    CONF_MEAS2 = 0x09,
    CONF_MEAS3 = 0x0A,
    CONF_MEAS4 = 0x0B,
    FDC_CONF = 0x0C,
    OFFSET_CAL_CIN1 = 0x0D,
    OFFSET_CAL_CIN2 = 0x0E,
    OFFSET_CAL_CIN3 = 0x0F,
    OFFSET_CAL_CIN4 = 0x10,
    GAIN_CAL_CIN1 = 0x11,
    GAIN_CAL_CIN2 = 0x12,
    GAIN_CAL_CIN3 = 0x13,
    GAIN_CAL_CIN4 = 0x14,
    MANUFACTURER_ID = 0xFE,
    DEVICE_ID = 0xFF
};

/** Template concept to constrain what structures encapsulate registers.*/
template <typename Reg>
// Struct has a valid register address
// Struct has an integer with the total number of bits in a register.
// This is used to mask the value before writing it to the sensor.
concept FDC1004CommandRegister =
    std::same_as<std::remove_cvref_t<decltype(Reg::address)>,
                 std::remove_cvref_t<Registers&>> &&
    std::integral<decltype(Reg::value_mask)> &&
    std::is_array_v<decltype(Reg::mode_map)>;

}  // namespace fdc1004

struct __attribute__((packed, __may_alias__)) ManufactureId {
    static constexpr Registers address = Registers::MANUFACTURER_ID;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode1_MSB {
    static constexpr Registers address = Registers::MEAS1_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode1_LSB {
    static constexpr Registers address = Registers::MEAS1_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t measurement : 8 = 0;
    uint8_t padding : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode2_MSB {
    static constexpr Registers address = Registers::MEAS2_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode2_LSB {
    static constexpr Registers address = Registers::MEAS2_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t measurement : 8 = 0;
    uint8_t padding : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode3_MSB {
    static constexpr Registers address = Registers::MEAS3_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode3_LSB {
    static constexpr Registers address = Registers::MEAS3_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t measurement : 8 = 0;
    uint8_t padding : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode4_MSB {
    static constexpr Registers address = Registers::MEAS4_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode4_LSB {
    static constexpr Registers address = Registers::MEAS4_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t measurement : 8 = 0;
    uint8_t padding : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure1 {
    static constexpr Registers address = Registers::CONF_MEAS1;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // Notes:

    // (1) It is not permitted to configure a measurement where the CHA field
    // and CHB field hold the same value (for example, if CHA=b010, CHB cannot
    // also be set to b010). (2) It is not permitted to configure a differential
    // measurement between CHA and CHB where CHA > CHB (for example, if CHA=
    // b010, CHB cannot be b001 or b000).

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
    uint8_t padding_0 : 4 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure2 {
    static constexpr Registers address = Registers::CONF_MEAS2;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // Notes:

    // (1) It is not permitted to configure a measurement where the CHA field
    // and CHB field hold the same value (for example, if CHA=b010, CHB cannot
    // also be set to b010). (2) It is not permitted to configure a differential
    // measurement between CHA and CHB where CHA > CHB (for example, if CHA=
    // b010, CHB cannot be b001 or b000).

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
    uint8_t padding_0 : 4 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure3 {
    static constexpr Registers address = Registers::CONF_MEAS3;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // Notes:

    // (1) It is not permitted to configure a measurement where the CHA field
    // and CHB field hold the same value (for example, if CHA=b010, CHB cannot
    // also be set to b010). (2) It is not permitted to configure a differential
    // measurement between CHA and CHB where CHA > CHB (for example, if CHA=
    // b010, CHB cannot be b001 or b000).

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
    uint8_t padding_0 : 4 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure4 {
    static constexpr Registers address = Registers::CONF_MEAS4;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // Notes:

    // (1) It is not permitted to configure a measurement where the CHA field
    // and CHB field hold the same value (for example, if CHA=b010, CHB cannot
    // also be set to b010). (2) It is not permitted to configure a differential
    // measurement between CHA and CHB where CHA > CHB (for example, if CHA=
    // b010, CHB cannot be b001 or b000).

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
    uint8_t padding_0 : 4 = 0;
};

struct __attribute__((packed, __may_alias__)) FDCConf {
    static constexpr Registers address = Registers::FDC_CONF;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN1 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN1;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // Notes:

    //    (1) It is not permitted to configure a measurement where the CHA field
    //    and CHB field hold the same value (for example, if
    // CHA=b010, CHB cannot also be set to b010).
    // (2) It is not permitted to configure a differential measurement between
    // CHA and CHB where CHA > CHB (for example, if CHA= b010, CHB cannot be
    // b001 or b000).

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
    uint8_t padding_0 : 4 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN2 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN2;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN3 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN3;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN4 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN4;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN1 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN1;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN2 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN2;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN3 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN3;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN4 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN4;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 0;
};

struct FDC1004RegisterMap {
    ManufactureId manufacture_id = {};
    AutoMeasureStatus status = {};
    TriggerOnDemandMeasure trigger_measurement = {};
    MeasureMode1_MSB measure_mode_1_msb = {};
    MeasureMode1_LSB measure_mode_1_lsb = {};
    MeasureMode2_MSB measure_mode_2_msb = {};
    MeasureMode2_LSB measure_mode_2_lsb = {};
    MeasureMode3_MSB measure_mode_3_msb = {};
    MeasureMode3_LSB measure_mode_3_lsb = {};
    MeasureMode4_MSB measure_mode_4_msb = {};
    MeasureMode4_LSB measure_mode_4_lsb = {};
    ConfMeasure1 config_measure_1 = {};
    ConfMeasure2 config_measure_2 = {};
    ConfMeasure3 config_measure_3 = {};
    ConfMeasure4 config_measure_4 = {};
    FDCConf fdc_conf = {};
    OffsetCalCIN1 offset_cal_cin1 = {};
    OffsetCalCIN2 offset_cal_cin2 = {};
    OffsetCalCIN3 offset_cal_cin3 = {};
    OffsetCalCIN4 offset_cal_cin4 = {};
    Reset reset = {};
};

}  // namespace sensors
