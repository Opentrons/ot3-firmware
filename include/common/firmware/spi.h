#include "stm32g4xx_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define COUNTOF(__BUFFER__) (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
#define BUFFERSIZE (COUNTOF(aTxBuffer) - 1)

enum { TRANSFER_WAIT, TRANSFER_COMPLETE, TRANSFER_ERROR };

SPI_HandleTypeDef MX_SPI1_Init();
void MX_GPIOA_Init();
void MX_GPIOB_Init();
void MX_DMA_Init();
void Spi_Error_Handler();
void Set_CS_Pin();
void Reset_CS_Pin();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
