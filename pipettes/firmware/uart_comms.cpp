
#include "common/uart_comms.hpp"

void Uart::send(int l, uint8_t* buff) {
    HAL_UART_Transmit(handle, buff, l, timeout);
}

void Uart::recv(int l, uint8_t* buff) {
    HAL_UART_Receive(handle, buff, l, timeout);
}
