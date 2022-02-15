#include "can/core/types.h"

/**
 * Round a length to a CAN FD length.
 *
 * @param length a message payload length
 * @return a CanFDMessageLength
 */
CanFDMessageLength to_canfd_length(uint32_t length) {
    switch (length) {
        case 0:
            return CanFDMessageLength::l0;
        case 1:
            return CanFDMessageLength::l1;
        case 2:
            return CanFDMessageLength::l2;
        case 3:
            return CanFDMessageLength::l3;
        case 4:
            return CanFDMessageLength::l4;
        case 5:
            return CanFDMessageLength::l5;
        case 6:
            return CanFDMessageLength::l6;
        case 7:
            return CanFDMessageLength::l7;
        case 8:
            return CanFDMessageLength::l8;
    }
    if (length <= 12)
        return CanFDMessageLength::l12;
    else if (length <= 16)
        return CanFDMessageLength::l16;
    else if (length <= 20)
        return CanFDMessageLength::l20;
    else if (length <= 24)
        return CanFDMessageLength::l24;
    else if (length <= 32)
        return CanFDMessageLength::l32;
    else if (length <= 48)
        return CanFDMessageLength::l48;
    else
        return CanFDMessageLength::l64;
}
