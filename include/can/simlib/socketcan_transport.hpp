#pragma once

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

#include "common/core/logging.hpp"
#include "common/core/synchronization.hpp"
#include "transport.hpp"

namespace socketcan_transport {

template <synchronization::LockableProtocol CriticalSection>
class SocketCanTransport : public can_transport::BusTransportBase {
  public:
    explicit SocketCanTransport(const char *address) : address{address} {}
    ~SocketCanTransport() { close(); };
    SocketCanTransport(const SocketCanTransport &) = delete;
    SocketCanTransport(const SocketCanTransport &&) = delete;
    SocketCanTransport &operator=(const SocketCanTransport &) = delete;
    SocketCanTransport &&operator=(const SocketCanTransport &&) = delete;

    auto open() -> bool;
    void close();

    auto write(uint32_t arb_id, const uint8_t *cbuff, uint32_t buff_len)
        -> bool;
    auto read(uint32_t &arb_id, uint8_t *buff, uint32_t &buff_len) -> bool;

  private:
    int handle{0};
    std::string address;
    CriticalSection critical_section{};
};

template <synchronization::LockableProtocol CriticalSection>
auto SocketCanTransport<CriticalSection>::open() -> bool {
    struct sockaddr_can addr;
    struct ifreq ifr;
    int s = 0;
    constexpr int use_canfd = 1;

    LOG("Trying to connect to %s\n", address.c_str());

    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) == -1) {
        return false;
    }

    if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &use_canfd,
                   sizeof(use_canfd))) {
        LOG("Failed to enable can fd.");
        return false;
    }

    strcpy(ifr.ifr_name, address);
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        return false;
    }
    LOG("Connected to %s\n", address.c_str());
    handle = s;
    return true;
}

template <synchronization::LockableProtocol CriticalSection>
void SocketCanTransport<CriticalSection>::close() {
    ::close(handle);
}

template <synchronization::LockableProtocol CriticalSection>
auto SocketCanTransport<CriticalSection>::write(uint32_t arb_id,
                                                const uint8_t *cbuff,
                                                uint32_t buff_len) -> bool {
    struct canfd_frame frame;
    // Set MSB for extended id
    frame.can_id = arb_id | (1 << 31);
    frame.len = buff_len;
    ::memcpy(frame.data, cbuff, buff_len);
    LOG("Writing: arb_id %X dlc %d\n", arb_id, buff_len);
    return ::write(handle, &frame, sizeof(struct can_frame)) > 0;
}

template <synchronization::LockableProtocol CriticalSection>
auto SocketCanTransport<CriticalSection>::read(uint32_t &arb_id, uint8_t *buff,
                                               uint32_t &buff_len) -> bool {
    struct canfd_frame frame;
    auto read_len = 0;

    {
        // Critical section block
        auto lock = synchronization::Lock(critical_section);
        read_len = ::read(handle, &frame, sizeof(struct can_frame));
    }

    if (read_len > 0) {
        arb_id = frame.can_id;
        buff_len = frame.len;
        ::memcpy(buff, frame.data, buff_len);

        LOG("Read: arb_id %X dlc %d\n", arb_id, buff_len);

        return true;
    } else {
        LOG("Read failed: %d\n", errno);
    }
    return false;
}

}  // namespace socketcan_transport
