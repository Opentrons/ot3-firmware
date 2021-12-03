#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>

#include "common/core/synchronization.hpp"
#include "transport.hpp"
#include "can/core/message_core.hpp"

namespace stdio_can {

template <synchronization::LockableProtocol CriticalSection>
class SocketTransport : public can_transport::BusTransportBase {
  public:
    SocketTransport() = default;
    ~SocketTransport() = default;
    SocketTransport(const SocketTransport &) = delete;
    SocketTransport(const SocketTransport &&) = delete;
    SocketTransport &operator=(const SocketTransport &) = delete;
    SocketTransport &&operator=(const SocketTransport &&) = delete;

    auto open(const char *ip, uint32_t port) -> bool;
    void close();

    auto write(uint32_t arb_id, const uint8_t *buff, uint32_t buff_len)
        -> bool;
    auto read(uint32_t &arb_id, uint8_t *buff, uint32_t &buff_len) -> bool;

  private:
    int handle{0};
    CriticalSection critical_section{};
};

template <synchronization::LockableProtocol CriticalSection>
auto SocketTransport<CriticalSection>::open(const char* ip, uint32_t port) -> bool {
    struct sockaddr_in addr;
    int s = 0;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    ::inet_aton(ip, &addr.sin_addr);

    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        return false;
    }
    std::cout << "Connected to " << ip << ":" << port << std::endl;
    handle = s;
    return true;
}

template <synchronization::LockableProtocol CriticalSection>
auto SocketTransport<CriticalSection>::write(uint32_t arb_id,
                                            const uint8_t *buff,
                                            uint32_t buff_len) -> bool {
    ::write(handle, &arb_id, sizeof(arb_id));
    ::write(handle, &buff_len, sizeof(buff_len));
    if (buff_len>0) {
        ::write(handle, buff, buff_len);
    }

    return true;
}

template <synchronization::LockableProtocol CriticalSection>
auto SocketTransport<CriticalSection>::read(uint32_t &arb_id, uint8_t *buff,
                                           uint32_t &buff_len) -> bool {
    ::read(handle, &arb_id, sizeof(arb_id));
    ::read(handle, &buff_len, sizeof(buff_len));
    if (buff_len > 0) {
        ::read(handle, buff, buff_len);
    }
    return true;
}

}  // namespace stdio_can