#include "../../include/spi/firmware/spi.h"

#include "common/firmware/errors.h"
#include "platform_specific_hal_conf.h"

void Set_CS_Pin(GPIO_TypeDef* GPIO_instance, uint32_t pin) {
    HAL_GPIO_WritePin(GPIO_instance, pin, GPIO_PIN_SET);
}

void Reset_CS_Pin(GPIO_TypeDef* GPIO_instance, uint32_t pin) {
    HAL_GPIO_WritePin(GPIO_instance, pin, GPIO_PIN_RESET);
}

HAL_StatusTypeDef hal_transmit_receive(uint8_t* transmit, uint8_t* receive,
                                       uint16_t buff_size, uint32_t timeout,
                                       SPI_HandleTypeDef* handle) {
    return HAL_SPI_TransmitReceive(handle, transmit, receive, buff_size,
                                   timeout);
}