#pragma once

#include "stm32g4xx_hal_conf.h"


#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
struct I2CConfig {
    I2C_TypeDef* instance;
    uint8_t address;
    uint16_t address_mode;
};
I2C_HandleTypeDef MX_I2C_Init(struct I2CConfig* conf);

#define REGISTER1 I2C1
#define REGISTER2 I2C2

#define ADDRESSMODE_7BIT I2C_ADDRESSINGMODE_7BIT
#define ADDRESSMODE_10BIT I2C_ADDRESSINGMODE_10BIT

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif // __I2C_H__