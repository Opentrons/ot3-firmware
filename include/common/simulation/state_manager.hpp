/**
 * @file state_manager.hpp
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

#include <array>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <memory>
#include <string>

#include "common/core/logging.h"
#include "common/core/synchronization.hpp"

namespace state_manager {

auto add_options(boost::program_options::options_description &cmdline_desc,
                 boost::program_options::options_description &env_desc)
    -> std::function<std::string(std::string)>;

template <synchronization::LockableProtocol CriticalSection>
class StateManagerConnection {
  public:
    explicit StateManagerConnection(std::string host, uint32_t port)
        : _host(host), _port(port), _socket(_context) {}
    ~StateManagerConnection() {}
    StateManagerConnection(const StateManagerConnection &) = delete;
    StateManagerConnection(const StateManagerConnection &&) = delete;
    StateManagerConnection &operator=(const StateManagerConnection &) = delete;
    StateManagerConnection &&operator=(const StateManagerConnection &&) =
        delete;

    auto open() -> bool {
        LOG("Creating state manager connection to %s:%d", _host.c_str(), _port);

        boost::asio::ip::udp::resolver resolver(_context);
        try {
            _endpoint = *resolver
                             .resolve(boost::asio::ip::udp::v4(), _host,
                                      std::to_string(_port))
                             .begin();
            _socket.open(boost::asio::ip::udp::v4());
        } catch (boost::system::system_error &) {
            LOG("Error opening state manager connection to %s:%d",
                _host.c_str(), _port);
            return false;
        }

        LOG("Connected to state manager at %s:%d", _host.c_str(), _port);
        return true;
    }

    auto close() -> void {
        if (_socket.is_open()) {
            LOG("Closing state manager socket to %s:%d", _host.c_str(), _port);
            _socket.close();
        }
    }

    template <size_t N>
    auto send(std::array<uint8_t, N> data) -> bool {
        if (_socket.is_open()) {
            LOG("Sending %d bytes to state manager", N);
            _socket.send_to(boost::asio::const_buffer(data.data(), N),
                            _endpoint);
        }
        return true;
    }

    // TODO:
    //   - Add message class
    //   - Write Message, accepting a message instance

  private:
    std::string _host;
    uint32_t _port;
    boost::asio::io_context _context{};
    boost::asio::ip::udp::endpoint _endpoint{};
    boost::asio::ip::udp::socket _socket;
    CriticalSection critical_section{};
};

template <synchronization::LockableProtocol CriticalSection>
auto create(const boost::program_options::variables_map &options)
    -> std::shared_ptr<StateManagerConnection<CriticalSection>> {
    auto host = options["state-mgr-host"].as<std::string>();
    auto port = options["state-mgr-port"].as<std::uint16_t>();
    auto ret =
        std::make_shared<StateManagerConnection<CriticalSection>>(host, port);
    ret->open();
    return ret;
}

}  // namespace state_manager