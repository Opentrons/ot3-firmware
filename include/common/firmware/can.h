#ifndef __CAN_H__
#define __CAN_H__

#if __has_include("stm32l5xx_hal_conf.h")
#include "stm32l5xx_hal_conf.h"
#else
#include "stm32g4xx_hal_conf.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


/**
 * Initializa FDCAN1
 *
 * @param handle Pointer to a FDCAN_HandleTypeDef
 * @return HAL_OK on success
 */
HAL_StatusTypeDef MX_FDCAN1_Init(FDCAN_HandleTypeDef * handle);


#ifdef __cplusplus
}  // extern "C"
#endif // __cplusplus

#endif  // __CAN_H__
