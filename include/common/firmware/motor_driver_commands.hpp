#define WRITE (uint8_t)(0x80)


// STEP (DIO6) AND DIR (DIO7) SHOULD BE CONNECTED TO D7 (PA8) & D8 (PA9) respectively
#define CHOPCONF (uint8_t)(0x6C)
#define COOLCONF (uint8_t)(0x6D)
#define DRV_STATUS (uint8_t)(0x6F)
#define IOIN (uint8_t)(0x04)

//#define WRITE_TO_TMC2130(regAddr, bit_number) (uint32_t *) (WRITE + ((uint)))
//#define READ_FROM_TMC2130(regAddr, bit_number)