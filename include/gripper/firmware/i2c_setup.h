#pragma once
#include "i2c/firmware/i2c.h"
#include "pipettes/core/pipette_type.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


typedef struct HandlerStruct {
    HAL_I2C_HANDLE i2c3;
} I2CHandlerStruct;

void i2c_setup(I2CHandlerStruct* i2c_handles);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
