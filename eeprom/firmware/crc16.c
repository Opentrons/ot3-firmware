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
    uint32_t aligned_buf[64];  // 255 bytes max -> 64 words (256 bytes)
    memset(aligned_buf, 0, sizeof(aligned_buf));
    memcpy(aligned_buf, data, length);  // safe, alignment-agnostic copy

    // length in words, rounded up
    uint32_t word_len = (length + 3U) / 4U;

    return (uint16_t)~HAL_CRC_Calculate(&hcrc, aligned_buf, word_len);
}
