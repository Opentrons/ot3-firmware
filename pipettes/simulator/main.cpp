#include <messages/messages_generated.h>

#include <iostream>

int main() {
    struct Opentrons::SetSpeed command {
        123
    };

    flatbuffers::FlatBufferBuilder builder(100);

    builder.CreateStruct(command);

    flatbuffers::Offset<int> o{builder.GetSize()};
    builder.Finish(o);

    uint8_t *buf = builder.GetBufferPointer();
    int size = builder.GetSize();

    std::cout << "Size:" << size << std::endl;

    for (int i = 0; i < size; i++) {
        std::cout << std::hex << (int)buf[i] << std::endl;
    }
    return 0;
}