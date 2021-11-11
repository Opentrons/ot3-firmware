#include "common/firmware/spi.h"

#include "common/firmware/errors.h"
#include "platform_specific_hal_conf.h"

/////

/*
These structs should ideally be under ./inculde/common/firmware/spi.h
Wasn't sure how to deal with types coming in from BSP under ./inlcude/common.. 
*/

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
struct SPI_config* zmotorConfig(void) {
    uint32_t z_stage_pins_sck_miso_mosi[3] = {GPIO_PIN_10, GPIO_PIN_11,
                                              GPIO_PIN_12};
    uint32_t z_stage_pins_cs[1] = {GPIO_PIN_4};
    uint32_t z_stage_pins_ctrl[1] = {GPIO_PIN_4};
    uint32_t z_stage_pins_dir_step[2] = {GPIO_PIN_0, GPIO_PIN_1};

    struct pin_group z_stage_pins_sck_miso_mosi_group = {

        .no_of_pins = sizeof(z_stage_pins_sck_miso_mosi) /
                      sizeof(z_stage_pins_sck_miso_mosi[0]),
        .port = GPIOC,
        .pins = z_stage_pins_sck_miso_mosi,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = GPIO_AF6_SPI3
        };

    struct pin_group z_stage_pins_cs_group = {

        .no_of_pins = sizeof(z_stage_pins_cs) / sizeof(z_stage_pins_cs[0]),
        .port = GPIOA,
        .pins = z_stage_pins_cs,
        .Mode = GPIO_MODE_OUTPUT_PP};

    struct pin_group z_stage_pins_ctrl_group = {

        .no_of_pins = sizeof(z_stage_pins_ctrl) / sizeof(z_stage_pins_ctrl[0]),
        .port = GPIOC,
        .pins = z_stage_pins_ctrl,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW};

    struct pin_group z_stage_pins_dir_step_group = {

        .no_of_pins =
            sizeof(z_stage_pins_dir_step) / sizeof(z_stage_pins_dir_step[0]),
        .port = GPIOC,
        .pins = z_stage_pins_dir_step,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW};

    struct pin_group _pg[4] = {z_stage_pins_sck_miso_mosi_group,
                               z_stage_pins_cs_group, z_stage_pins_ctrl_group,
                               z_stage_pins_dir_step_group};
    struct SPI_config z_config = {
        .comp = head,
        .sub_comp = motor_z,
        .SPI_int = _SPI3,
        .no_of_groups = 4,
        // todo check mcu heap situation and maybe get back to dynamic
        // allocation.. z-config.pg = malloc( z_config.no_of_groups * sizeof
        // pin_group);
        .pg = _pg};
    return &z_config;
}

void* SPIConfigInit(size_t no_of_groups) {
    struct SPI_config_group scg;
    scg.no_of_groups = no_of_groups;
    struct SPI_config arr[no_of_groups];
    arr[0] = *(zmotorConfig());
    scg.sc = arr;
    return &scg;
}

void SPI3_configs(void) {
    struct SPI_config* SPI_config_tmp = zmotorConfig();
        /* Peripheral clock enable */
        __HAL_RCC_SPI3_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**SPI3 GPIO Configuration
        PA4     ------> SPI3_NSS
        PC10     ------> SPI3_SCK
        PC11     ------> SPI3_MISO
        PC12     ------> SPI3_MOSI
         Step/Dir
         PC1  ---> Dir Pin Motor on SPI3 (Z-axis)
         PC0  ---> Step Pin Motor on SPI3 (Z-axis)
        */

        for (int i = 0; i < SPI_config_tmp->no_of_groups; i++) {
            int total_pins_in_group = SPI_config_tmp->pg[i].no_of_pins;
            uint32_t pin_tmp = SPI_config_tmp->pg[i].pins[0];
            for (int j = 0; j < total_pins_in_group; j++) {
                pin_tmp = pin_tmp | SPI_config_tmp->pg[i].pins[j];
            }
            // GPIO_InitStruct
            SPI_config_tmp->pg[i].GPIO_InitStruct.Pin = pin_tmp;
            SPI_config_tmp->pg[i].GPIO_InitStruct.Mode =
                SPI_config_tmp->pg[i].Mode;
            SPI_config_tmp->pg[i].GPIO_InitStruct.Pull =
                SPI_config_tmp->pg[i].Pull;
            SPI_config_tmp->pg[i].GPIO_InitStruct.Speed =
                SPI_config_tmp->pg[i].Speed;
            SPI_config_tmp->pg[i].GPIO_InitStruct.Alternate =
                SPI_config_tmp->pg[i].Alternate;
            HAL_GPIO_Init(SPI_config_tmp->pg[i].port, &SPI_config_tmp->pg[i].GPIO_InitStruct);
        }

}
/////

SPI_HandleTypeDef handle, handle_SPI2, handle_SPI3;

/**
 * @brief SPI MSP Initialization
 * This function configures SPI for the Z/A axis motors
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
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
         PC7  ---> Dir Pin Motor A
         PC6  ---> Step Pin Motor A
        */
        

        // Chip select
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // PB11 A motor control
        // ctrl /Dir/Step pin
        GPIO_InitStruct.Pin = GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                      &GPIO_InitStruct);
    }
    else if (hspi->Instance == SPI3) {
        SPI3_configs();
    }
}

/**
 * @brief SPI MSP De-Initialization
 * This function denits SPI for Z/A motors
 * @param hspi: SPI2 handle pointer
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

        Step/Dir
         PC7  ---> Dir Pin Motor on SPI2 (A-axis)
         PC6  ---> Step Pin Motor on SPI2 (A-axis)
        */
        HAL_GPIO_DeInit(GPIOB,
                        GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6 | GPIO_PIN_7);
    } else if (hspi->Instance == SPI3) {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();

        /**SPI3 GPIO Configuration
        PA04     ------> SPI3_NSS
        PC10     ------> SPI3_SCK
        PC11     ------> SPI3_MISO
        PC12     ------> SPI3_MOSI

        Step/Dir
         PC1  ---> Dir Pin Motor on SPI3 (Z-axis)
         PC0  ---> Step Pin Motor on SPI3 (Z-axis)
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
                                   GPIO_PIN_0 | GPIO_PIN_1);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
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

/*
once the refactor has been done for all components (gantry and pipettes) 
these two functions can be taken out of ./inculde/common/firmware/sph.h
and replaced with SPI_init()
*/
void SPI2_init() {}

void SPI3_init() {}

void Set_CS_Pin() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void Reset_CS_Pin() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
}

void hal_transmit_receive(uint8_t* transmit, uint8_t* receive,
                          uint16_t buff_size, uint32_t timeout) {
    HAL_SPI_TransmitReceive(&handle, transmit, receive, buff_size, timeout);
}
void hal_transmit_receive_SPI2(uint8_t* transmit, uint8_t* receive,
                               uint16_t buff_size, uint32_t timeout) {
    HAL_SPI_TransmitReceive(&handle_SPI2, transmit, receive, buff_size,
                            timeout);
}
void hal_transmit_receive_SPI3(uint8_t* transmit, uint8_t* receive,
                                uint16_t buff_size, uint32_t timeout) {
    HAL_SPI_TransmitReceive(&handle_SPI3, transmit, receive, buff_size,
                            timeout);
}