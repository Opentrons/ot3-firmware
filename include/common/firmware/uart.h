#pragma once

#include "stm32g4xx_hal_conf.h"


#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
UART_HandleTypeDef MX_LPUART1_UART_Init();
#ifdef __cplusplus
}  // extern "C"
#endif // __cplusplus

#endif // __UART_H__