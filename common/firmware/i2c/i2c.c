#include "platform_specific_hal_conf.h"
#include "common/firmware/i2c.h"


/**
 * Wrapper around HAL_I2C_Master_Transmit
 */
bool hal_i2c_master_transmit(HAL_I2C_HANDLE handle, uint16_t DevAddress, uint8_t *data, uint16_t size, uint32_t timeout)
{
    return HAL_I2C_Master_Transmit(handle,
                            DevAddress, data, size, timeout) == HAL_OK;
}

/**
 * Wrapper around HAL_I2C_Master_Receive
 */
bool hal_i2c_master_receive(HAL_I2C_HANDLE handle, uint16_t DevAddress, uint8_t *data, uint16_t size, uint32_t timeout){
    if (data_ready()) {
        return HAL_I2C_Master_Receive(handle,
                                      DevAddress, data, size,
                                      timeout) == HAL_OK;
    } else {
        return 0;
    }

}