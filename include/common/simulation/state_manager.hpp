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
#include <atomic>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>

#include "common/core/logging.h"
#include "common/core/synchronization.hpp"
#include "common/simulation/state_manager_parser.hpp"
#include "ot_utils/core/bit_utils.hpp"

namespace state_manager {

using namespace state_manager_parser;

auto add_options(boost::program_options::options_description &cmdline_desc,
                 boost::program_options::options_description &env_desc)
    -> std::function<std::string(std::string)>;

/**
 * @brief Manages the datagram connection to the central state manager.
 *
 * @details
 * This class asynchronously sends to and receives from the state manager
 * server. The run() function must be invoked from its own thread context,
 * and it will automatically serve all new messages.
 *
 * @tparam CriticalSection Provides thread safety on internal queues
 */
template <synchronization::LockableProtocol CriticalSection>
class StateManagerConnection {
  public:
    using T = StateManagerConnection<CriticalSection>;
    // This only applies to OUTGOING messages. Responses from the server
    // may have variable length.
    static constexpr size_t StateMessageLen = 4;
    using StateMessage = std::array<uint8_t, StateMessageLen>;
    using MQueue = std::deque<StateMessage>;
    static constexpr size_t MaxReceive = 128;

    explicit StateManagerConnection(std::string host, uint32_t port)
        : _host(host), _port(port), _socket(_service) {}
    ~StateManagerConnection() { close(); }
    StateManagerConnection(const StateManagerConnection &) = delete;
    StateManagerConnection(const StateManagerConnection &&) = delete;
    StateManagerConnection &operator=(const StateManagerConnection &) = delete;
    StateManagerConnection &&operator=(const StateManagerConnection &&) =
        delete;

    /**
     * @brief This function must be called from its own thread context in
     * order to provide servicing of the state manager UDP connection in an
     * asynchronous manner. This function will block until the thread ends.
     */
    [[noreturn]] void run() {
        LOG("Starting state manager io_service");
        if (!open()) {
            // This actually doesn't rely on a server being open since
            // we're using datagrams, so it makes sense to throw on
            // errors here.
            throw std::invalid_argument(
                "Could not initialize state manager connection.");
        }
        // This ensures that run() doesn't return prematurely
        boost::asio::io_service::work work(_service);
        _socket.async_receive(
            boost::asio::buffer(_rx_buf),
            boost::bind(&T::handle_receive, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        get_sync_state();
        // Should run until the program is killed
        while (1) {
            _service.run();
        }
    }
    /**
     * @brief Send a Move message to the state manager, indicating an axis on
     * the robot moved a single microstep.
     *
     * @param id The id of the axis
     * @param direction The direction the axis moved
     * @return true if the message sent succesfully, false otherwise
     */
    auto send_move_msg(MoveMessageHardware id, Direction direction) -> void {
        std::array<uint8_t, 4> message;
        auto itr = ot_utils::bit_utils::int_to_bytes(
            static_cast<uint8_t>(MessageID::move), message.begin(),
            message.end());
        itr = ot_utils::bit_utils::int_to_bytes(static_cast<uint16_t>(id), itr,
                                                message.end());
        itr = ot_utils::bit_utils::int_to_bytes(static_cast<uint8_t>(direction),
                                                itr, message.end());
        send_message(message);
    }

    /**
     * @brief Send a Sync pin message to the state manager, indicating the
     * sync pin on the OT-3 changed state.
     *
     * @param state The updated state of the sync pin
     * @return true if the message sent succesfully, false otherwise
     */
    auto send_sync_msg(SyncPinState state) -> void {
        std::array<uint8_t, 4> message;
        auto itr = ot_utils::bit_utils::int_to_bytes(
            static_cast<uint8_t>(MessageID::sync_pin), message.begin(),
            message.end());
        itr = ot_utils::bit_utils::int_to_bytes(static_cast<uint16_t>(0), itr,
                                                message.end());
        itr = ot_utils::bit_utils::int_to_bytes(static_cast<uint8_t>(state),
                                                itr, message.end());
        send_message(message);
    }

    auto get_sync_state() -> void {
        std::array<uint8_t, 4> message = {
            static_cast<uint8_t>(MessageID::get_sync_pin_state), 0x00, 0x00,
            0x00};
        send_message(message);
    }

    auto current_sync_state() -> SyncPinState { return SyncPinState::LOW; }

  private:
    /**
     * Enqueues a new message to send. This should only be called under
     * a synchronization lock.
     * @return True if there was already a message in the queue, false
     * if the queue was empty before adding this message.
     */
    auto queue_message(StateMessage &msg) -> bool {
        _messages.push_back(msg);
        return _messages.size() > 1;
    }

    /**
     * @brief Send a message. This might not send the message immediately,
     * but it will be enqueued to send eventually.
     *
     * @param msg
     */
    auto send_message(StateMessage &msg) -> void {
        auto lock = synchronization::Lock(critical_section);
        // This will add the message no matter what. We only send if there was
        // nothing enqueued before adding this message.
        if (!queue_message(msg)) {
            _socket.async_send_to(
                boost::asio::const_buffer(_messages.front().data(),
                                          StateMessageLen),
                _endpoint,
                boost::bind(&T::handle_send, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
        }
    }

    auto handle_send(const boost::system::error_code &error, std::size_t bytes)
        -> void {
        auto lock = synchronization::Lock(critical_section);
        _messages.pop_front();
        std::ignore = bytes;
        std::ignore = error;
        if (_messages.size() > 0) {
            // Start another send
            _socket.async_send_to(
                boost::asio::const_buffer(_messages.front().data(),
                                          StateMessageLen),
                _endpoint,
                boost::bind(&T::handle_send, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
        }
    }

    auto handle_receive(const boost::system::error_code &error,
                        std::size_t bytes) -> void {
        // Don't need to lock on reception because no other thread context
        // touches the rx buffer
        std::ignore = bytes;
        std::ignore = error;
        if (!error || error == boost::asio::error::message_size) {
            LOG("Received %d bytes", bytes);

            auto end = _rx_buf.begin();
            std::advance(end, bytes);
            auto response = parse_state_manager_response(_rx_buf.begin(), end);
            if (std::holds_alternative<SyncPinState>(response)) {
                _sync_pin_state = std::get<SyncPinState>(response);
                LOG("Updated sync pin value: %d",
                    static_cast<int>(_sync_pin_state.load()));
            }

            _socket.async_receive(
                boost::asio::buffer(_rx_buf),
                boost::bind(&T::handle_receive, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
        }
    }

    auto open() -> bool {
        auto lock = synchronization::Lock(critical_section);

        LOG("Creating state manager connection to %s:%d", _host.c_str(), _port);

        boost::asio::ip::udp::resolver resolver(_service);
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

    std::string _host;
    uint32_t _port;
    boost::asio::io_service _service{};
    boost::asio::ip::udp::endpoint _endpoint{};
    boost::asio::ip::udp::socket _socket;
    CriticalSection critical_section{};
    MQueue _messages{};
    std::array<uint8_t, MaxReceive> _rx_buf{};
    std::atomic<SyncPinState> _sync_pin_state = SyncPinState::LOW;
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

template <synchronization::LockableProtocol CriticalSection>
struct StateManagerTask {
    using Connection = std::shared_ptr<StateManagerConnection<CriticalSection>>;
    [[noreturn]] void operator()(Connection *connection_ptr) {
        if (connection_ptr) {
            auto connection = *connection_ptr;
            if (connection) {
                connection->run();
            }
        }
        LOG("State manager spuriously exited");
        while (1) {
        }
    }
};

}  // namespace state_manager