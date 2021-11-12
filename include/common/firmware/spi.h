#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void SPI2_init();
void Set_CS_Pin();
void Reset_CS_Pin();
void* get_SPI_handle(void*);
void hal_transmit_receive(uint8_t* transmit, uint8_t* receive,
                          uint16_t buff_size, uint32_t timeout, void* handle) ;

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus