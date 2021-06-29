#pragma once

#include <concepts>
#include <span>

namespace io {

template<class C>
concept ReaderProtocol = requires(C c, std::span<uint8_t> buff) {
    { c.read(buff) };
};


template<class C>
concept WriterProtocol = requires(C c, std::span<uint8_t> buff) {
    { c.write(buff) };
};

}