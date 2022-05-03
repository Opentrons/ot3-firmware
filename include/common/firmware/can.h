#ifndef __CAN_H__
#define __CAN_H__

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


/**
 * Initializa FDCAN1
 *
 * @param handle Pointer to a FDCAN_HandleTypeDef
 * @param clock_divider Clock divider for canbus peripheral
 * @param segment_1_tqs Time quanta for the phase 1 segment
 * @param segment_2_tqs Time quanta for the phase 2 segment
 * @param max_sync_jump_width Max jump width for auto resync
 * @return HAL_OK on success
 */
HAL_StatusTypeDef MX_FDCAN1_Init(FDCAN_HandleTypeDef * handle, uint8_t clock_divider, uint8_t segment_1_tqs, uint8_t segment_2_tqs, uint8_t max_sync_jump_width);


#ifdef __cplusplus
}  // extern "C"
#endif // __cplusplus

#endif  // __CAN_H__
