#pragma once

#include <stdint.h>
#include "stm32g4xx_hal_conf.h"


class Uart {
public:
    explicit Uart(UART_HandleTypeDef * handle): handle{handle} {};
    void send(int l, uint8_t * buff);
    void recv(int l, uint8_t * buff);

private:
    static constexpr auto timeout = 0xFFFF;
    UART_HandleTypeDef * handle;
};
