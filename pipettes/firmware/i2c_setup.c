#include "platform_specific_hal_conf.h"
#include "pipettes/core/pipette_type.h"
#include "pipettes/firmware/i2c_setup.h"
#include "common/firmware/errors.h"

static I2C_HandleTypeDef hi2c2;
static I2C_HandleTypeDef hi2c3;


/**
 * @brief Eeprom Write Pin GPIO Initialization Function
 * @param None
 * @retval None
 */
void eeprom_write_gpio_init() {
    PipetteType pipette_type = get_pipette_type();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    if (pipette_type == NINETY_SIX_CHANNEL) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /*Configure GPIO pin : A15 */
        GPIO_InitStruct.Pin = GPIO_PIN_15;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    } else {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /*Configure GPIO pin : B8 */
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}


void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();

    if(hi2c->Instance==I2C2) {
        // PIN PB6 is SCL
        // PIN PB7 is SDA
        // Secondary sensors and eeprom
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        HAL_GPIO_Init(
            GPIOA,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
            &GPIO_InitStruct);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

        GPIO_InitStruct.Pin = GPIO_PIN_4;
        HAL_GPIO_Init(
            GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
            &GPIO_InitStruct);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    } else if(hi2c->Instance==I2C3) {
        // PIN PC0 is SCL
        // PIN PC1 is SDA
        // Primary sensors
        __HAL_RCC_I2C3_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
        HAL_GPIO_Init(
            GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
            &GPIO_InitStruct);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    }

    /*Configure data ready pin : PC3 */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

HAL_I2C_HANDLE MX_I2C2_Init()
{
    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x10C0ECFF;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c2) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure Analogue filter
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure Digital filter
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
    {
        Error_Handler();
    }

    return &hi2c2;

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

int data_ready() {
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_SET;
}

/**
 * @brief enable writing to the eeprom.
 */
void enable_eeprom_write() {
    if (get_pipette_type() == NINETY_SIX_CHANNEL) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
    }
}

/**
 * @brief disable writing to the eeprom.
 */
void disable_eeprom_write() {
    if (get_pipette_type() == NINETY_SIX_CHANNEL) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
    }
}

void i2c_setup(I2CHandlerStruct* temp_struct) {
    HAL_I2C_HANDLE i2c3 = MX_I2C3_Init();
    temp_struct->i2c3 = i2c3;
    eeprom_write_gpio_init();
    HAL_I2C_HANDLE i2c2 = MX_I2C2_Init();
    temp_struct->i2c2 = i2c2;

    // Write protect the eeprom
    disable_eeprom_write();
}
