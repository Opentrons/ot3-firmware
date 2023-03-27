#include "gantry/firmware/i2c_setup.h"

#include "common/firmware/errors.h"
#include "platform_specific_hal_conf.h"

#define EEPROM_GPIO_BANK GPIOB
#define EEPROM_GPIO_PIN GPIO_PIN_10

static I2C_HandleTypeDef hi2c2;

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c) {
    (void)hi2c;
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // PIN PC4 is SCL
    // PIN PA8 is SDA
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(
        GPIOA,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        &GPIO_InitStruct);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    HAL_GPIO_Init(
        GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        &GPIO_InitStruct);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    __HAL_RCC_I2C2_CLK_ENABLE();
}

HAL_I2C_HANDLE MX_I2C2_Init() {
    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x10C0ECFF;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Analogue filter
     */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) !=
        HAL_OK) {
        Error_Handler();
    }
    /** Configure Digital filter
     */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK) {
        Error_Handler();
    }
    return &hi2c2;
}
/**
 * @brief enable the eeprom write protect pin.
 */
void eeprom_write_protect_init(void) {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin : B10 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = EEPROM_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(EEPROM_GPIO_BANK, &GPIO_InitStruct);
}

/**
 * @brief enable writing to the eeprom.
 */
void enable_eeprom_write() {
    HAL_GPIO_WritePin(GPIOC, EEPROM_GPIO_PIN, GPIO_PIN_RESET);
}

/**
 * @brief disable writing to the eeprom.
 */
void disable_eeprom_write() {
    HAL_GPIO_WritePin(GPIOC, EEPROM_GPIO_PIN, GPIO_PIN_SET);
}

void i2c_setup(I2CHandlerStruct* i2c_handles) {
    HAL_I2C_HANDLE i2c2 = MX_I2C2_Init();
    i2c_handles->i2c2 = i2c2;
    eeprom_write_protect_init();

    // write protect the eeprom.
    disable_eeprom_write();
}
