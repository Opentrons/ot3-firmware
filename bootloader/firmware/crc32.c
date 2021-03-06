#include "bootloader/firmware/crc32.h"
#include "platform_specific_hal_conf.h"
#include "common/firmware/errors.h"

/**
 * Used this to match results from python's zlib.crc32.
 * https://m0agx.eu/2021/04/09/matching-stm32-hardware-crc-with-standard-crc-32/
 */

/**
 * Handle to CRC module.
 */
static CRC_HandleTypeDef hcrc;

/**
 * Initialize CRC handle.
 */
static void MX_CRC_Init(void);


/**
 * Initialize crc module.
 */
void crc32_init() {
    MX_CRC_Init();
}

/**
 * Compute the CRC
 * @param data Data
 * @param length Length of data
 * @return Computed CRC
 */
uint32_t crc32_compute(const uint8_t* data, uint8_t length) {
    return ~HAL_CRC_Calculate(&hcrc, (uint32_t*)data, length);
}

/**
 * Continue accumulating CRC using provided data.
 * @param data Data
 * @param length Length of data
 * @return Accumulated CRC
 */
uint32_t crc32_accumulate(const uint8_t* data, uint8_t length) {
    return ~HAL_CRC_Accumulate(&hcrc, (uint32_t*)data, length);
}

/**
 * Reset the accumulated CRC value.
 */
void crc32_reset_accumulator() {
    __HAL_CRC_DR_RESET(&hcrc);
}

/**
 * Initialize the CRC unit.
 */
void MX_CRC_Init(void) {
    hcrc.Instance = CRC;
    hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
    hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
    hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
    if (HAL_CRC_Init(&hcrc) != HAL_OK)
    {
        Error_Handler();
    }
}
