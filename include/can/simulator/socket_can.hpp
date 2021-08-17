#pragma once

#include <stdint.h>

namespace socket_can {

class SocketCanTransport {
  public:
    SocketCanTransport();
    ~SocketCanTransport();
    SocketCanTransport(const SocketCanTransport &) = delete;
    SocketCanTransport(const SocketCanTransport &&) = delete;
    SocketCanTransport &operator=(const SocketCanTransport &) = delete;
    SocketCanTransport &&operator=(const SocketCanTransport &&) = delete;

    auto open(const char *address) -> bool;
    void close();

    auto write(uint32_t arb_id, const uint8_t *cbuff, uint32_t buff_len)
        -> bool;
    auto read(uint32_t &arb_id, uint8_t *buff, uint32_t &buff_len) -> bool;

  private:
    int handle;
};

}  // namespace socket_can
