#include "eeprom/firmware/crc16.h"

#include <stdint.h>
#include <string.h>

#include "common/firmware/errors.h"
#include "platform_specific_hal_conf.h"

/**
 * Handle to CRC module.
 */
static CRC_HandleTypeDef hcrc;

/**
 * Initialize CRC handle.
 */
// static void MX_CRC_Init(void);

/**
 * Initialize the CRC unit.
 */
void MX_CRC_Init(void) {
    __HAL_RCC_CRC_CLK_ENABLE();
    hcrc.Instance = CRC;
    hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    hcrc.Init.GeneratingPolynomial = 0x1021;
    hcrc.Init.CRCLength = CRC_POLYLENGTH_16B;
    hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
    hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
    if (HAL_CRC_Init(&hcrc) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * Initialize crc module.
 */
void crc16_init() { MX_CRC_Init(); }

/**
 * Compute the CRC
 * @param data Data
 * @param length Length of data
 * @return Computed CRC
 */
uint16_t crc16_compute(const uint8_t* data, uint8_t length) {
    return (uint16_t)~HAL_CRC_Calculate(&hcrc, (uint32_t*)data, length);
}
// uint16_t crc16_compute(const uint8_t* data, uint8_t length) {
//     // make sure the data is word_aligned
//     uint32_t aligned_buff[64];
//     memset(aligned_buff, 0,
//            sizeof(aligned_buff));  // initialize to 0 to avoid reading
//                                    // uninitialized memory
//
//     memcpy(aligned_buff, data, length);  // copy data to aligned buffer
//
//     uint8_t aligned_length =
//         (length + 3U) / 4U;  // calculate length in words, round up
//
//     return ~HAL_CRC_Calculate(&hcrc, aligned_buff, aligned_length);
// }

/**
 * Continue accumulating CRC using provided data.
 * @param data Data
 * @param length Length of data
 * @return Accumulated CRC
 */
uint16_t crc16_accumulate(const uint8_t* data, uint8_t length) {
    return ~HAL_CRC_Accumulate(&hcrc, (uint32_t*)data, length);
}

/**
 * Reset the accumulated CRC value.
 */
void crc16_reset_accumulator() { /*__HAL_CRC_DR_RESET(&hcrc);*/ }
