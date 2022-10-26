#include "platform_specific_hal_conf.h"
#include "pipettes/core/pipette_type.h"
#include "pipettes/firmware/i2c_setup.h"
#include "common/firmware/errors.h"

static I2C_HandleTypeDef hi2c3;


/**
 * @brief Eeprom Write Pin GPIO Initialization Function
 * @param None
 * @retval None
 */
void eeprom_write_gpio_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /*Configure GPIO pin : B10 */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();

    if(hi2c->Instance==I2C3) {
        // PIN PC8 is SCL
        // PIN PC9 is SDA
        // Primary sensors
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF8_I2C3;
        HAL_GPIO_Init(
            GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
            &GPIO_InitStruct);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        __HAL_RCC_I2C3_CLK_ENABLE();

    }

}


HAL_I2C_HANDLE MX_I2C3_Init()
{
    hi2c3.Instance = I2C3;
    hi2c3.Init.Timing = 0x10C0ECFF;
    hi2c3.Init.OwnAddress1 = 0;
    hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c3.Init.OwnAddress2 = 0;
    hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c3) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure Analogue filter
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure Digital filter
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
    {
        Error_Handler();
    }

    return &hi2c3;

}

/**
 * @brief enable writing to the eeprom.
 */
void enable_eeprom_write() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
}

/**
 * @brief disable writing to the eeprom.
 */
void disable_eeprom_write() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
}

void i2c_setup(I2CHandlerStruct* temp_struct) {
    HAL_I2C_HANDLE i2c3 = MX_I2C3_Init();
    temp_struct->i2c3 = i2c3;
    eeprom_write_gpio_init();
    // Write protect the eeprom
    disable_eeprom_write();
}
