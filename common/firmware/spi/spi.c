#include "common/firmware/spi.h"
#include "common/firmware/errors.h"

#include "platform_specific_hal_conf.h"

////
struct pin_group {
    GPIO_TypeDef  * port;
    size_t no_of_pins;
    uint32_t* pins;
    GPIO_InitTypeDef GPIO_InitStruct;
    uint32_t Mode; /*!< Specifies the operating mode for the selected pins.
                      This parameter can be a value of @ref GPIO_mode */

    uint32_t Pull; /*!< Specifies the Pull-up or Pull-Down activation for the
                      selected pins. This parameter can be a value of @ref
                      GPIO_pull */

    uint32_t Speed; /*!< Specifies the speed for the selected pins.
                        This parameter can be a value of @ref GPIO_speed */

    uint32_t Alternate; /*!< Peripheral to be connected to the selected pins
                             This parameter can be a value of @ref
                           GPIOEx_Alternate_function_selection */
};

struct SPI_config {
    SPI_interface SPI_int;
    component comp;
    sub_component sub_comp;
    struct pin_group* pg;
    size_t no_of_groups;
};

struct SPI_config_group {
    struct SPI_config* sc;
    size_t no_of_groups;
};

/**SPI3 GPIO Configuration
        PA4     ------> SPI3_NSS
        PC10     ------> SPI3_SCK
        PC11     ------> SPI3_MISO
        PC12     ------> SPI3_MOSI
         Step/Dir
         PC1  ---> Dir Pin Motor on SPI3 (Z-axis)
         PC0  ---> Step Pin Motor on SPI3 (Z-axis)
        ctrl pin
        PC4
----z stage pins for ff board---
pins with similiar configurations, pull up etc,
are grouped together. Some groups did
end up with just one pin, oh well.
*/
struct SPI_config* commonConfig(void) {
    uint32_t common_pins_sck_miso_mosi[3] = {GPIO_PIN_13, GPIO_PIN_14,
                                              GPIO_PIN_15};
    uint32_t common_pins_cs[1] = {GPIO_PIN_12};
    uint32_t common_pins_ctrl[1] = {GPIO_PIN_10};
    uint32_t common_pins_dir[1] = {GPIO_PIN_1};
    uint32_t common_pins_step[1] = {GPIO_PIN_8};


    struct pin_group common_pins_sck_miso_mosi_group = {

        .no_of_pins = sizeof(common_pins_sck_miso_mosi) /
                      sizeof(common_pins_sck_miso_mosi[0]),
        .port = GPIOB,
        .pins = common_pins_sck_miso_mosi,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = GPIO_AF5_SPI2
        };

    struct pin_group common_pins_cs_group = {

        .no_of_pins = sizeof(common_pins_cs) / sizeof(common_pins_cs[0]),
        .port = GPIOB,
        .pins = common_pins_cs,
        .Mode = GPIO_MODE_OUTPUT_PP};

    struct pin_group common_pins_ctrl_group = {

        .no_of_pins = sizeof(common_pins_ctrl) / sizeof(common_pins_ctrl[0]),
        .port = GPIOA,
        .pins = common_pins_ctrl,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW};

    struct pin_group common_pins_dir_group = {

        .no_of_pins =
            sizeof(common_pins_dir) / sizeof(common_pins_dir[0]),
        .port = GPIOB,
        .pins = common_pins_dir,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW};

    struct pin_group common_pins_step_group = {

        .no_of_pins =
            sizeof(common_pins_dir) / sizeof(common_pins_step[0]),
        .port = GPIOA,
        .pins = common_pins_dir,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW};

    struct pin_group _pg[4] = {common_pins_sck_miso_mosi_group,
                               common_pins_cs_group, common_pins_ctrl_group,
                               common_pins_dir_group, 
                               common_pins_step_group};
    struct SPI_config common_config = {
        .comp = common,
        .sub_comp = na,
        .SPI_int = _SPI2,
        .no_of_groups = 5,
        // todo check mcu heap situation and maybe get back to dynamic
        .pg = _pg};
    return &common_config;
}

void* SPIConfigInit(size_t no_of_groups) {
    struct SPI_config_group scg;
    scg.no_of_groups = no_of_groups;
    struct SPI_config arr[no_of_groups];
    arr[0] = *(commonConfig());
    scg.sc = arr;
    return &scg;
}
////

SPI_HandleTypeDef handle,handle_SPI2, handle_SPI3;;

/**
 * @brief SPI MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
    // PB14 -> MISO
    // PB12 -> CS
    // PB13 -> CLK
    // PB15 -> MOSI
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hspi->Instance == SPI2) {
        /* Peripheral clock enable */
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**SPI2 GPIO Configuration
        PB12     ------> SPI2_NSS
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_MISO
        PB15     ------> SPI2_MOSI

         Step/Dir
         PB1  ---> Dir Pin Motor 1
         PA8  ---> Step Pin Motor 1
        */
        GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Chip select
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Dir/Step pin
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOA,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                      &GPIO_InitStruct);
    }
}

/**
 * @brief SPI MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
    if (hspi->Instance == SPI2) {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();

        /**SPI2 GPIO Configuration
        PB12     ------> SPI2_NSS
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_MISO
        PB15     ------> SPI2_MOSI
        */
        HAL_GPIO_DeInit(GPIOB,
                        GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8 | GPIO_PIN_9);
    }
}

/**
 * @brief SPI2 Initialization Function
 * @param None
 * @retval None
 */
SPI_HandleTypeDef MX_SPI2_Init() {
    /* SPI2 parameter configuration*/
    __HAL_RCC_SPI2_CLK_ENABLE();
    SPI_HandleTypeDef hspi2 = {
        .Instance = SPI2,
        .Init = {.Mode = SPI_MODE_MASTER,
                 .Direction = SPI_DIRECTION_2LINES,
                 .DataSize = SPI_DATASIZE_8BIT,
                 .CLKPolarity = SPI_POLARITY_HIGH,
                 .CLKPhase = SPI_PHASE_2EDGE,
                 .NSS = SPI_NSS_SOFT,
                 .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32,
                 .FirstBit = SPI_FIRSTBIT_MSB,
                 .TIMode = SPI_TIMODE_DISABLE,
                 .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
                 .CRCPolynomial = 7,
                 .CRCLength = SPI_CRC_LENGTH_DATASIZE,
                 .NSSPMode = SPI_NSS_PULSE_DISABLE}

    };

    if (HAL_SPI_Init(&hspi2) != HAL_OK) {
        Error_Handler();
    }
    return hspi2;
}

/**
 * @brief SPI3 Initialization Function
 * @param None
 * @retval None
 */
SPI_HandleTypeDef MX_SPI3_Init() {
    /* SPI2 parameter configuration*/
    __HAL_RCC_SPI3_CLK_ENABLE();
    SPI_HandleTypeDef hspi3 = {
        .Instance = SPI3,
        .Init = {.Mode = SPI_MODE_MASTER,
                 .Direction = SPI_DIRECTION_2LINES,
                 .DataSize = SPI_DATASIZE_8BIT,
                 .CLKPolarity = SPI_POLARITY_HIGH,
                 .CLKPhase = SPI_PHASE_2EDGE,
                 .NSS = SPI_NSS_SOFT,
                 .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32,
                 .FirstBit = SPI_FIRSTBIT_MSB,
                 .TIMode = SPI_TIMODE_DISABLE,
                 .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
                 .CRCPolynomial = 7,
                 .CRCLength = SPI_CRC_LENGTH_DATASIZE,
                 .NSSPMode = SPI_NSS_PULSE_DISABLE}

    };

    if (HAL_SPI_Init(&hspi3) != HAL_OK) {
        Error_Handler();
    }
    return hspi3;
}

void SPI_init() {
    struct SPI_config_group * scg_ptr= (struct SPI_config_group *)SPIConfigInit(1);
    for (int i = 0; i < scg_ptr->no_of_groups; i++) {
        switch(scg_ptr->sc[i].SPI_int) {
            case _SPI0:
                break;
            case _SPI1:
                break;
            case _SPI2:
                handle_SPI2 = MX_SPI2_Init();
                break;
            case _SPI3:
                handle_SPI3 = MX_SPI3_Init(); 
                break;
            default:
                printf("Invalid SPI interface\n");
        }
    }
}

void SPI2_init() {
    handle = MX_SPI2_Init();
}

void Set_CS_Pin() { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); }

void Reset_CS_Pin() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

void hal_transmit_receive(uint8_t* transmit, uint8_t* receive, uint16_t buff_size, uint32_t timeout) {
    HAL_SPI_TransmitReceive(&handle, transmit, receive, buff_size, timeout);
}