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

HAL_I2C_HANDLE MX_I2C1_Init();
HAL_I2C_HANDLE MX_I2C3_Init();
int data_ready();
void enable_eeprom();
void disable_eeprom();
int tip_present();
typedef enum Type { MULTI_CHANNEL, SINGLE_CHANNEL } PipetteType;
typedef struct HandlerStruct {
    HAL_I2C_HANDLE i2c1;
    HAL_I2C_HANDLE i2c3;
} I2CHandlerStruct;
void i2c_setup(I2CHandlerStruct* temp_struct, PipetteType pipette_type);


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
