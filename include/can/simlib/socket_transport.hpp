#pragma once

#include <time.h>

#include <algorithm>
#include <boost/asio.hpp>

#include "can/core/message_core.hpp"
#include "common/core/logging.h"
#include "common/core/synchronization.hpp"
#include "transport.hpp"

namespace socket_transport {

using boost::asio::ip::tcp;

template <synchronization::LockableProtocol CriticalSection>
class SocketTransport : public can_transport::BusTransportBase {
  public:
    explicit SocketTransport(const char *host, uint32_t port)
        : host{host}, port{port}, socket{context} {}
    ~SocketTransport() = default;
    SocketTransport(const SocketTransport &) = delete;
    SocketTransport(const SocketTransport &&) = delete;
    SocketTransport &operator=(const SocketTransport &) = delete;
    SocketTransport &&operator=(const SocketTransport &&) = delete;

    auto open() -> bool;
    void close();

    auto write(uint32_t arb_id, const uint8_t *buff, uint32_t buff_len) -> bool;
    auto read(uint32_t &arb_id, uint8_t *buff, uint32_t &buff_len) -> bool;

  private:
    std::string host;
    uint32_t port;
    boost::asio::io_context context{};
    tcp::socket socket;
    CriticalSection critical_section{};
};

template <synchronization::LockableProtocol CriticalSection>
auto SocketTransport<CriticalSection>::open() -> bool {
    LOG("Creating connection to %s:%d\n", host.c_str(), port);

    tcp::resolver resolver(context);
    try {
        boost::asio::connect(socket,
                             resolver.resolve(host, std::to_string(port)));
    } catch (boost::system::system_error) {
        return false;
    }

    LOG("Connected to %s:%d\n", host.c_str(), port);
    return true;
}

template <synchronization::LockableProtocol CriticalSection>
void SocketTransport<CriticalSection>::close() {
    socket.close();
}

template <synchronization::LockableProtocol CriticalSection>
auto SocketTransport<CriticalSection>::write(uint32_t arb_id,
                                             const uint8_t *buff,
                                             uint32_t buff_len) -> bool {
    LOG("Sending: arbitration %X dlc %d\n", arb_id, buff_len);

    // Critical section block
    auto lock = synchronization::Lock(critical_section);

    auto out_arb_id = htonl(arb_id);
    auto out_buff_len = htonl(buff_len);
    socket.send(boost::asio::const_buffer(&out_arb_id, sizeof(arb_id)));
    socket.send(boost::asio::const_buffer(&out_buff_len, sizeof(buff_len)));
    if (buff_len > 0) {
        socket.send(boost::asio::const_buffer(buff, buff_len));
    }
    return true;
}

template <synchronization::LockableProtocol CriticalSection>
auto SocketTransport<CriticalSection>::read(uint32_t &arb_id, uint8_t *buff,
                                            uint32_t &buff_len) -> bool {
    struct timespec tspec;
    tspec.tv_sec = 0;
    tspec.tv_nsec = 1 * 1000000;  // milliseconds

    // Wait for data to be available for blocking read.
    while (!socket.available()) {
        // FreeRTOS recommends use of nanosleep in their posix port docs
        nanosleep(&tspec, nullptr);
    }

    // Critical section block
    auto lock = synchronization::Lock(critical_section);

    if (boost::asio::read(socket,
                          boost::asio::buffer(&arb_id, sizeof(arb_id))) <
        sizeof(arb_id))
        return false;

    if (boost::asio::read(socket,
                          boost::asio::buffer(&buff_len, sizeof(buff_len))) <
        sizeof(buff_len))
        return false;

    arb_id = ntohl(arb_id);
    buff_len = std::min(static_cast<uint32_t>(message_core::MaxMessageSize),
                        ntohl(buff_len));

    LOG("Read: arbitration %X dlc %d\n", arb_id, buff_len);
    if (buff_len > 0) {
        if (boost::asio::read(socket, boost::asio::buffer(buff, buff_len)) <
            buff_len)
            return false;
    }
    return true;
}

}  // namespace socket_transport