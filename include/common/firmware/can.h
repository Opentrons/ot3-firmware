#ifndef __CAN_H__
#define __CAN_H__

#include "stm32g4xx_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


/**
 * The global can 1 handle.
 */
extern FDCAN_HandleTypeDef fdcan1;

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
