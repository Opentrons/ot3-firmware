#pragma once

#include <span>
#include "stm32g4xx_hal_conf.h"

namespace uart_comms {

class Uart {
public:
    explicit Uart(UART_HandleTypeDef *handle) : handle{handle} {}

    void read(std::span <uint8_t> &buff);

    void write(const std::span <uint8_t> &buff);

private:
    static constexpr auto timeout = 0xFFFF;
    UART_HandleTypeDef *handle;
};


}