#pragma once
#include "i2c/firmware/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct HandlerStruct {
    HAL_I2C_HANDLE i2c2;
    HAL_I2C_HANDLE i2c3;
} I2CHandlerStruct;
void i2c_setup(I2CHandlerStruct* temp_struct);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
