#include "platform_specific_hal_conf.h"
#include "platform_specific_hal.h"
#include "i2c/firmware/i2c.h"


/**
 * Wrapper around HAL_I2C_Master_Transmit
 */
bool hal_i2c_master_transmit(HAL_I2C_HANDLE handle, uint16_t DevAddress, uint8_t *data, uint16_t size, uint32_t timeout)
{
    I2C_HandleTypeDef* i2c_handle = (I2C_HandleTypeDef*)handle;
    uint32_t tickstart = HAL_GetTick();
    HAL_StatusTypeDef tx_result = HAL_OK;
    do {
        tx_result = HAL_I2C_Master_Transmit(i2c_handle,
                            DevAddress, data, size, timeout);
        if (__HAL_I2C_GET_FLAG(i2c_handle, I2C_FLAG_AF)) {
            __HAL_I2C_CLEAR_FLAG(i2c_handle, I2C_FLAG_AF);
            tx_result = HAL_BUSY;
        }
        if (tx_result == HAL_OK) {
            break;
        }
    } while ((HAL_GetTick() - tickstart) < timeout);
    return tx_result == HAL_OK;
}

/**
 * Wrapper around HAL_I2C_Master_Receive
 */
bool hal_i2c_master_receive(HAL_I2C_HANDLE handle, uint16_t DevAddress, uint8_t *data, uint16_t size, uint32_t timeout){
    I2C_HandleTypeDef* i2c_handle = (I2C_HandleTypeDef*)handle;
    uint32_t tickstart = HAL_GetTick();
    HAL_StatusTypeDef rx_result = HAL_OK;
    do {
        rx_result = HAL_I2C_Master_Receive(i2c_handle, DevAddress, data, size,
                                           timeout);
        if (__HAL_I2C_GET_FLAG(i2c_handle, I2C_FLAG_AF)){
            __HAL_I2C_CLEAR_FLAG(i2c_handle, I2C_FLAG_AF);
            rx_result = HAL_BUSY;
        }
        if (rx_result == HAL_OK) {
            break;
        }
    } while ((HAL_GetTick() - tickstart) < timeout);
    return rx_result == HAL_OK;
}
