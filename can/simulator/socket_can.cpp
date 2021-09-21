#include "can/simulator/socket_can.hpp"

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "FreeRTOS.h"
#include "task.h"

using namespace socket_can;

SocketCanTransport::SocketCanTransport() : handle{0} {}

SocketCanTransport::~SocketCanTransport() { close(); }

auto SocketCanTransport::open(const char *address) -> bool {
    struct sockaddr_can addr;
    struct ifreq ifr;
    int s = 0;
    constexpr int use_canfd = 1;

    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) == -1) {
        return false;
    }

    if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES,
                   &use_canfd, sizeof(use_canfd))){
        printf("Unable to enable can fd.");
        return false;
    }

    strcpy(ifr.ifr_name, address);
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        return false;
    }
    handle = s;
    return true;
}

void SocketCanTransport::close() { ::close(handle); }

auto SocketCanTransport::write(uint32_t arb_id, const uint8_t *cbuff,
                               uint32_t buff_len) -> bool {
    struct canfd_frame frame;
    // Set MSB for extended id
    frame.can_id = arb_id | (1 << 31);
    frame.can_dlc = buff_len;
    ::memcpy(frame.data, cbuff, buff_len);
    return ::write(handle, &frame, sizeof(struct can_frame)) > 0;
}

auto SocketCanTransport::read(uint32_t &arb_id, uint8_t *buff,
                              uint32_t &buff_len) -> bool {
    struct canfd_frame frame;

    taskENTER_CRITICAL();
    auto read_len = ::read(handle, &frame, sizeof(struct can_frame));
    taskEXIT_CRITICAL();

    if (read_len > 0) {
        arb_id = frame.can_id;
        buff_len = frame.can_dlc;
        ::memcpy(buff, frame.data, buff_len);

        std::cout << "arb_id: " << std::hex << arb_id << " "
                  << "length: " << buff_len << ":";
        for (int i = 0; i < buff_len; i++) {
            std::cout << " " << std::hex << (int)buff[i];
        }
        std::cout << std::endl;

        return true;
    } else {
        std::cout << "read failed: " << errno << std::endl;
    }
    return false;
}
