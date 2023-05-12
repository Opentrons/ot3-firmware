#pragma once
#include "i2c/firmware/i2c.h"

#include "platform_specific_hal_conf.h"

#define I2C3_SCL_PIN  GPIO_PIN_8
#define I2C3_SDA_PIN  GPIO_PIN_9
#define I2C3_PORT  GPIOC

#define EEPROM_WRITE_ENABLE_PIN GPIO_PIN_9
#define EEPROM_WRITE_ENABLE_PORT GPIOB


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus



typedef struct HandlerStruct {
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
