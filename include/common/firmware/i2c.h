#pragma once

#include "stm32g4xx_hal_conf.h"


#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

I2C_HandleTypeDef MX_I2C1_Init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif // __I2C_H__