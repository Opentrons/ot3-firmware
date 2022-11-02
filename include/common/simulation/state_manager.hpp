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
#include <stdexcept>
#include <string>

#include "common/core/logging.h"
#include "common/core/synchronization.hpp"
#include "ot_utils/core/bit_utils.hpp"

// Message types
#include "common/simulation/direction.hpp"
#include "common/simulation/message_ids.hpp"
#include "common/simulation/move_message_hw_ids.hpp"
#include "common/simulation/sync_pin_state.hpp"

namespace state_manager {

auto add_options(boost::program_options::options_description &cmdline_desc,
                 boost::program_options::options_description &env_desc)
    -> std::function<std::string(std::string)>;

template <synchronization::LockableProtocol CriticalSection>
class StateManagerConnection {
  public:
    explicit StateManagerConnection(std::string host, uint32_t port)
        : _host(host), _port(port), _socket(_context) {
        if (!open()) {
            // This actually doesn't rely on a server being open since
            // we're using datagrams, so it makes sense to throw on
            // errors here.
            throw std::invalid_argument(
                "Could not initialize state manager connection.");
        }
    }
    ~StateManagerConnection() { close(); }
    StateManagerConnection(const StateManagerConnection &) = delete;
    StateManagerConnection(const StateManagerConnection &&) = delete;
    StateManagerConnection &operator=(const StateManagerConnection &) = delete;
    StateManagerConnection &&operator=(const StateManagerConnection &&) =
        delete;

    /**
     * @brief Send a Move message to the state manager, indicating an axis on
     * the robot moved a single microstep.
     *
     * @param id The id of the axis
     * @param direction The direction the axis moved
     * @return true if the message sent succesfully, false otherwise
     */
    auto send_move_msg(MoveMessageHardware id, Direction direction) -> bool {
        std::array<uint8_t, 4> message;
        auto itr = ot_utils::bit_utils::int_to_bytes(
            static_cast<uint8_t>(MessageID::move), message.begin(),
            message.end());
        itr = ot_utils::bit_utils::int_to_bytes(static_cast<uint16_t>(id), itr,
                                                message.end());
        itr = ot_utils::bit_utils::int_to_bytes(static_cast<uint8_t>(direction),
                                                itr, message.end());
        std::ignore = send_and_receive(message);
        return true;
    }

    /**
     * @brief Send a Sync pin message to the state manager, indicating the
     * sync pin on the OT-3 changed state.
     *
     * @param state The updated state of the sync pin
     * @return true if the message sent succesfully, false otherwise
     */
    auto send_sync_msg(SyncPinState state) -> bool {
        std::array<uint8_t, 4> message;
        auto itr = ot_utils::bit_utils::int_to_bytes(
            static_cast<uint8_t>(MessageID::sync_pin), message.begin(),
            message.end());
        itr = ot_utils::bit_utils::int_to_bytes(static_cast<uint16_t>(0), itr,
                                                message.end());
        itr = ot_utils::bit_utils::int_to_bytes(static_cast<uint8_t>(state),
                                                itr, message.end());
        std::ignore = send_and_receive(message);
        return true;
    }

    auto get_sync_state() -> SyncPinState {
        std::array<uint8_t, 4> message = {
            static_cast<uint8_t>(MessageID::get_sync_pin_state), 0x00, 0x00,
            0x00};
        auto response = send_and_receive(message);
        // Response is an ascii character for 'low' or 'high'
        return response[0] == '0' ? SyncPinState::LOW : SyncPinState::HIGH;
    }

  private:
    auto open() -> bool {
        auto lock = synchronization::Lock(critical_section);

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
        auto lock = synchronization::Lock(critical_section);
        if (_socket.is_open()) {
            LOG("Closing state manager socket to %s:%d", _host.c_str(), _port);
            _socket.close();
        }
    }

    template <size_t N>
    auto send_and_receive(const std::array<uint8_t, N> &send)
        -> std::array<uint8_t, N> {
        auto lock = synchronization::Lock(critical_section);
        _socket.send_to(boost::asio::const_buffer(send.data(), N), _endpoint);
        std::array<uint8_t, N> ret;
        _socket.receive(boost::asio::buffer(ret));
        return ret;
    }

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
    return ret;
}

}  // namespace state_manager