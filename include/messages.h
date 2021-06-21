#ifndef OT3FIRMWARE_MESSAGES_H
#define OT3FIRMWARE_MESSAGES_H

struct Message {
    unsigned long arbitration_id;
    unsigned int length;
    unsigned char data[64];
};

#endif  // OT3FIRMWARE_MESSAGES_H
