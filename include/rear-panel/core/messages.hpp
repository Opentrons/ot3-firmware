#pragma once
#include <cstdint>
#include <variant>

namespace messages {

/*
** Message structs initiate actions. These may be changes in physical state, or
** a request to send back some data. Each carries an ID, which should be copied
** into the response.
*/

struct IncomingMessageFromHost {
    const char* buffer;
    const char* limit;
};

};  // namespace messages
