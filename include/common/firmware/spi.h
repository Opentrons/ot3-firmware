#pragma once
#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef enum { common, gantry, head, pipette } component;
typedef enum { gantry_x, gantry_y, motor_a, motor_z, na } sub_component;
typedef enum { _SPI0, _SPI1, _SPI2, _SPI3 } SPI_interface;

void SPI2_init();
void* SPIConfigInit(size_t no_of_groups);
void SPI3_configs(void);
void SPI_init();
void Set_CS_Pin();
void Reset_CS_Pin();
void hal_transmit_receive(uint8_t* transmit, uint8_t* receive,
                          uint16_t buff_size, uint32_t timeout);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus