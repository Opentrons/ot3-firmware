#pragma once
#include "i2c/firmware/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct HandlerStruct {
    HAL_I2C_HANDLE i2c2;
    HAL_I2C_HANDLE i2c3;
} I2CHandlerStruct;

void i2c_setup(I2CHandlerStruct* i2c_handles);

/**
 * enable writing to the eeprom.
 */
void enable_eeprom_write();

/**
 * disable writing to the eeprom.
 */
void disable_eeprom_write();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
