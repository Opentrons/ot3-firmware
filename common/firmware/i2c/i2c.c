#include "common/firmware/i2c.h"
#include "common/firmware/errors.h"

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c) {

    // PIN PB7 is SCL
    // PIN PB8-BOOT0 is SDA
    if(hi2c->Instance==I2C1) {
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(
            GPIOB,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
            &GPIO_InitStruct);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    } else if(hi2c->Instance==I2C2) {
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(
            GPIOB,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
            &GPIO_InitStruct);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    }
}

I2C_HandleTypeDef MX_I2C_Init(struct I2CConfig* conf)
{
    I2C_HandleTypeDef hi2c;
    hi2c.Instance = conf->instance;
    hi2c.Init.Timing = 0x30A0A7FB;
    hi2c.Init.OwnAddress1 = conf->address;
    hi2c.Init.AddressingMode = conf->address_mode;
    hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c.Init.OwnAddress2 = 0;
    hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure Analogue filter
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure Digital filter
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c, 0) != HAL_OK)
    {
        Error_Handler();
    }

    return hi2c;

}
