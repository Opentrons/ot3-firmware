
#include "common/firmware/uart_comms.hpp"

using namespace uart_comms;

void Uart::write(const std::span<uint8_t>& buff) {
    HAL_UART_Transmit(handle, buff.data(), buff.size(), timeout);
}

void Uart::read(std::span<uint8_t>& buff) {
    HAL_UART_Receive(handle, buff.data(), buff.size(), timeout);
}
