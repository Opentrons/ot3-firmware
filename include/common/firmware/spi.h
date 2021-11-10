#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void SPI2_init();
void SPI3_init();
void Set_CS_Pin();
void Reset_CS_Pin();
//todo refactor. have single trans/recv func for all SPIs
void hal_transmit_receive(uint8_t* transmit, uint8_t *receive, uint16_t buff_size, uint32_t timeout);
void hal_transmit_receive_SPI2(uint8_t* transmit, uint8_t *receive, uint16_t buff_size, uint32_t timeout);
void hal_transmit_receive_SPI3(uint8_t* transmit, uint8_t *receive, uint16_t buff_size, uint32_t timeout);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus