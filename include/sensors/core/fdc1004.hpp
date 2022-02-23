#pragma once

/*
 * FDC1004 Capacitive Sensor
 *
 * Datasheet: https://www.ti.com/lit/ds/symlink/fdc1004.pdf?ts=1645556558559&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FFDC1004
 *
 * The configuration of the sensor is always in single-ended measurement mode. We are currently only using
 * 1 out of the 4 measurement channels.
 *
 * By default, CAPDAC is enabled. The max resolution of the capdac is 5 bits.
 *
 * Capacitance measurement is a total of 24 bits long. You must do consecutive reads of
 * the MSB and LSB registers to get the data.
 */

namespace fdc1004_utils {
    static auto convert(uint32_t measurement, uint16_t accumulation, float current_offset) -> sq14_15;
    static auto calculate_capdac(uint16_t measurement, uint16_t accumulation, float current_offset)-> uint16_t;

    // Capacitance Sensor Address and Registers
    static constexpr uint16_t ADDRESS = 0x50 << 1;
    static constexpr uint8_t MSB_MEASUREMENT_1 = 0x00;
    static constexpr uint8_t LSB_MEASUREMENT_1 = 0x01;
    static constexpr uint8_t CONFIGURATION_MEASUREMENT = 0x08;
    static constexpr uint8_t FDC_CONFIGURATION = 0x0C;
    static constexpr uint8_t DEVICE_ID_REGISTER = 0xFF;

    //configurations
    static constexpr uint16_t DEVICE_CONFIGURATION = 0x0 << 13 |  // CHA = CIN1 (U.FL Connector)
                                                     0x4 << 10;   // CHB = CAPDAC
    static constexpr uint16_t SAMPLE_RATE = 1 << 10 |  // 100S/s
                                            1 << 8 |   // Repeat enabled
                                            1 << 7;    // Measurement 1 enabled

    static constexpr uint16_t DEVICE_ID = 0x1004;

    // Constants
    static constexpr int MAX_CAPDAC_RESOLUTION = 5;
    static constexpr int MSB_SHIFT = 16;
    static constexpr uint16_t MAX_CAPDAC_OFFSET = 0x1F << MAX_CAPDAC_RESOLUTION;
    static constexpr float MAX_MEASUREMENT = 524288.0;
    // single ended capdac offset measurement in pF
    static constexpr float CAPDAC_OFFSET = 3.125;


    [[maybe_unused]] static auto convert(uint32_t measurement, uint16_t accumulation, float current_offset) -> sq14_15 {
        // shift the 32 bit value number down to 24 bits
        auto average = static_cast<float>(measurement >> 8)/static_cast<float>(accumulation);
        float capacitance = average/MAX_MEASUREMENT + current_offset;
        return convert_to_fixed_point(capacitance, 15);
    }

    [[maybe_unused]] static auto calculate_capdac(uint16_t measurement, uint16_t accumulation, float current_offset) -> uint16_t {
        float average = static_cast<float>(measurement)/static_cast<float>(accumulation);
        float offset = (average + current_offset) / CAPDAC_OFFSET;
        uint16_t new_offset = static_cast<uint8_t>(offset) << MAX_CAPDAC_RESOLUTION;
        if (new_offset > MAX_CAPDAC_OFFSET) {
            new_offset = MAX_CAPDAC_OFFSET;
        }
        return new_offset;
    }

} // namespace fdc1004_utils


/*


float fdc1004_set_capdac(I2C_HandleTypeDef *i2c, float pf_delta)
{
    static const float CAPDAC_PF_PER_COUNT = 3.125;
    static const float CAPDAC_COUNTS_PER_PF = 0.32; // 1.0 counts / 3.125 pF
    uint16_t temp;
    i2c_read_reg_uint16(i2c, FDC1004_ADDR, FDC1004_CONF_MEAS1, &temp, I2C_MSB_FIRST);

    // Calculate CAPDAC value
    // Commands are relative to measurement, so include present offset
    uint8_t offset = (uint8_t)((capacitance_offset + pf_delta) * CAPDAC_COUNTS_PER_PF);
    if (offset > 0x1F) offset = 0x1F;
    temp &= ~(0x1F << 5);
    temp |= offset << 5;

    // Set CHB to CAPDAC
    temp &= ~(0x7 << 10);
    temp |= 0x4 << 10;

    i2c_write_reg_uint16(i2c, FDC1004_ADDR, FDC1004_CONF_MEAS1, temp, I2C_MSB_FIRST);
    capacitance_offset = (float)offset * CAPDAC_PF_PER_COUNT;
    return capacitance_offset;
}

 */
