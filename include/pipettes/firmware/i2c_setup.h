#pragma once
#include "i2c/firmware/i2c.h"
#include "pipettes/core/pipette_type.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

int tip_present();
typedef struct HandlerStruct {
    HAL_I2C_HANDLE i2c1;
    HAL_I2C_HANDLE i2c3;
} I2CHandlerStruct;
void i2c_setup(I2CHandlerStruct* temp_struct, PipetteType pipette_type);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
