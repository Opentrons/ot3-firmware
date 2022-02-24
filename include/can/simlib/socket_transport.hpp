#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>

#include "FreeRTOS.h"
#include "can/core/message_core.hpp"
#include "common/core/logging.h"
#include "common/core/synchronization.hpp"
#include "task.h"
#include "transport.hpp"

namespace socket_transport {

template <synchronization::LockableProtocol CriticalSection>
class SocketTransport : public can_transport::BusTransportBase {
  public:
    explicit SocketTransport(const char *host, uint32_t port)
        : host{host}, port{port} {}
    ~SocketTransport() = default;
    SocketTransport(const SocketTransport &) = delete;
    SocketTransport(const SocketTransport &&) = delete;
    SocketTransport &operator=(const SocketTransport &) = delete;
    SocketTransport &&operator=(const SocketTransport &&) = delete;

    auto open() -> bool;
    void close();

    auto write(uint32_t arb_id, const uint8_t *buff, uint32_t buff_len) -> bool;
    auto read(uint32_t &arb_id, uint8_t *buff, uint32_t &buff_len) -> bool;

    static auto host_to_ip(const std::string &host) -> std::string;

  private:
    int handle{0};
    std::string host;
    uint32_t port;
    CriticalSection critical_section{};
};

template <synchronization::LockableProtocol CriticalSection>
auto SocketTransport<CriticalSection>::open() -> bool {
    struct sockaddr_in addr;
    int s = 0;

    LOG("Creating connection to %s:%d\n", host.c_str(), port);

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return false;
    }

    auto ip = host_to_ip(host);
    LOG("Trying to connect to %s:%d\n", ip.c_str(), port);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    ::inet_aton(ip.c_str(), &addr.sin_addr);

    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        return false;
    }
    LOG("Connected to %s:%d\n", host.c_str(), port);
    handle = s;
    return true;
}

template <synchronization::LockableProtocol CriticalSection>
void SocketTransport<CriticalSection>::close() {
    ::close(handle);
    handle = 0;
}

template <synchronization::LockableProtocol CriticalSection>
auto SocketTransport<CriticalSection>::write(uint32_t arb_id,
                                             const uint8_t *buff,
                                             uint32_t buff_len) -> bool {
    // Critical section block
    auto lock = synchronization::Lock(critical_section);

    LOG("Sending: arbitration %X dlc %d\n", arb_id, buff_len);

    auto out_arb_id = htonl(arb_id);
    auto out_buff_len = htonl(buff_len);
    ::write(handle, &out_arb_id, sizeof(arb_id));
    ::write(handle, &out_buff_len, sizeof(buff_len));
    if (buff_len > 0) {
        ::write(handle, buff, buff_len);
    }

    return true;
}

template <synchronization::LockableProtocol CriticalSection>
auto SocketTransport<CriticalSection>::read(uint32_t &arb_id, uint8_t *buff,
                                            uint32_t &buff_len) -> bool {
    // TODO (2021-12-08, Amit): This is required for FreeRTOS to have time to
    //  process message. Need to find a better solution.
    vTaskDelay(1);

    // Critical section block
    auto lock = synchronization::Lock(critical_section);

    if (::read(handle, &arb_id, sizeof(arb_id)) < sizeof(arb_id)) return false;

    if (::read(handle, &buff_len, sizeof(buff_len)) < sizeof(buff_len))
        return false;

    arb_id = ntohl(arb_id);
    buff_len = std::min(static_cast<uint32_t>(message_core::MaxMessageSize),
                        ntohl(buff_len));

    LOG("Read: arbitration %X dlc %d\n", arb_id, buff_len);
    if (buff_len > 0) {
        if (::read(handle, buff, buff_len) < buff_len) return false;
    }
    return true;
}

template <synchronization::LockableProtocol CriticalSection>
auto SocketTransport<CriticalSection>::host_to_ip(const std::string &host)
    -> std::string {
    auto h = gethostbyname(host.c_str());
    if (!h) {
        throw std::runtime_error("Invalid host: " + std::string(host));
    }
    auto addr_list = reinterpret_cast<struct in_addr **>(h->h_addr_list);
    return inet_ntoa(*addr_list[0]);
}

}  // namespace socket_transport