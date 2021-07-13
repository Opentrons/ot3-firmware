#include "stm32g4xx_hal_conf.h"

#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

SPI_HandleTypeDef MX_SPI2_Init();
void MX_GPIOA_Init();
void MX_GPIOB_Init();
void MX_DMA_Init();
void Spi_Error_Handler();
void Set_CS_Pin();
void Reset_CS_Pin();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif // __SPI_H__