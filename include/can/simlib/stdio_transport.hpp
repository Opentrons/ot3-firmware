#pragma once

#include "common/core/synchronization.hpp"
#include "transport.hpp"

namespace stdio_can {

template <synchronization::LockableProtocol CriticalSection>
class StdioTransport : public can_transport::BusTransportBase {
  public:
    StdioTransport() =default;
    ~StdioTransport() =default;
    StdioTransport(const StdioTransport &) = delete;
    StdioTransport(const StdioTransport &&) = delete;
    StdioTransport &operator=(const StdioTransport &) = delete;
    StdioTransport &&operator=(const StdioTransport &&) = delete;

    auto write(uint32_t arb_id, const uint8_t *cbuff, uint32_t buff_len)
        -> bool;
    auto read(uint32_t &arb_id, uint8_t *buff, uint32_t &buff_len) -> bool;

  private:
    CriticalSection critical_section{};
};


template <synchronization::LockableProtocol CriticalSection>
auto StdioTransport<CriticalSection>::write(uint32_t arb_id,
                                                const uint8_t *cbuff,
                                                uint32_t buff_len) -> bool {
    return true;
}

template <synchronization::LockableProtocol CriticalSection>
auto StdioTransport<CriticalSection>::read(uint32_t &arb_id, uint8_t *buff,
                                               uint32_t &buff_len) -> bool {
    return true;
}

}