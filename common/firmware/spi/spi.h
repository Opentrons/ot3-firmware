#pragma once
#include <stdint.h>

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


void Set_CS_Pin(GPIO_TypeDef * GPIO_instance, uint32_t pin);
void Reset_CS_Pin(GPIO_TypeDef *  GPIO_instance, uint32_t pin);
void hal_transmit_receive(uint8_t* transmit, uint8_t* receive,
                          uint16_t buff_size, uint32_t timeout, SPI_HandleTypeDef* handle) ;

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus