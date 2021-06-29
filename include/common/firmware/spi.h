#include "stm32g4xx_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
#define BUFFERSIZE                       (COUNTOF(aTxBuffer) - 1)

enum
{
    TRANSFER_WAIT,
    TRANSFER_COMPLETE,
    TRANSFER_ERROR
};

SPI_HandleTypeDef MX_SPI1_Init();
void MX_GPIO_Init();
void MX_DMA_Init();
void Spi_Error_Handler();


#ifdef __cplusplus
}  // extern "C"
#endif // __cplusplus
