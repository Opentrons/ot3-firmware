/**
 * @file state_manager_connection.hpp
 * @brief Provides an interface for setting up socket connections to
 * the simulator's state manager.
 * 
 * @details 
 * The state manager is responsible for tracking the position of the motors on
 * a simulated OT3 system. In order for this to work, the firmware simulators
 * need to be able to update the state manager with the status of their
 * motors. This is performed by opening a socket connection from each 
 * simulator to the central state manager, and sending brief messages when
 * any of the pins related to motor control are modified.
 * 
 */

#pragma once

#include <string>

#include "common/core/synchronization.hpp"

namespace state_manager {

template <synchronization::LockableProtocol CriticalSection>
class StateManagerConnection {
  public:
    explicit StateManagerConnection(std::string host, uint32_t port) :
        _host(host), _port(port), _socket(_context) {}
    ~StateManagerConnection() {}
    StateManagerConnection(const StateManagerConnection &) = delete;
    StateManagerConnection(const StateManagerConnection &&) = delete;
    StateManagerConnection &operator=(const StateManagerConnection &) = delete;
    StateManagerConnection &&operator=(const StateManagerConnection &&) = delete;

    auto open() -> bool {
        LOG("Creating connection to %s:%d", _host.c_str(), _port);

        tcp::resolver resolver(context);
        try {
            boost::asio::connect(socket,
                                resolver.resolve(_host, std::to_string(_port)));
        } catch (boost::system::system_error &) {
            LOG("Error opening connection to %s:%d", _host.c_str(), _port);
            return false;
        }

        LOG("Connected to %s:%d", _host.c_str(), _port);
        return true;
    }

    auto close() -> void {
        if(_socket.is_open()) {
            LOG("Closing socket to %s:%d", _host.c_str(), _port);
            _socket.close();
        }
    }

    // TODO:
    //   - Add message class
    //   - Write Message, accepting a message instance

  private:

    std::string _host;
    uint32_t _port;
    boost::asio::io_context _context{};
    boost::asio::ip::tcp::socket _socket;
    CriticalSection critical_section{};
};

}