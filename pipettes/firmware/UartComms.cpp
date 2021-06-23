
#include "common/UartComms.hpp"

void Uart::send(int l, uint8_t* buff) {
    HAL_UART_Transmit(handle, buff, l, 0xFFFF);
}

void Uart::recv(int l, uint8_t* buff) {
    HAL_UART_Receive(handle, buff, l, 0xFFFF);
}
