#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef void * HAL_I2C_HANDLE;

HAL_I2C_HANDLE MX_I2C_Init();

/**
 * Wrapper around HAL_I2C_Master_Transmit
 */
bool hal_i2c_master_transmit(HAL_I2C_HANDLE handle, uint16_t dev_address, uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * Wrapper around HAL_I2C_Master_Receive
 */
bool hal_i2c_master_receive(HAL_I2C_HANDLE handle, uint16_t dev_address, uint8_t *data, uint16_t size, uint32_t timeout);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
