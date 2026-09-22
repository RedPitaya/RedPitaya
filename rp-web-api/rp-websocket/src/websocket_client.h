/**
 * $Id$
 *
 * @brief Red Pitaya Web module, client side
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#pragma once
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include "signal.hpp"

/**
 * Connects to a websocket server run by another process on the board.
 *
 * The counterpart of websocket_server: an application links this to reach a
 * background service instead of hosting the device access itself.
 */
class websocket_client {
public:
    typedef websocketpp::connection_hdl connection_hdl;
    typedef websocketpp::client<websocketpp::config::asio_client> client;

    websocket_client();
    ~websocket_client();

    /** Starts connecting. Returns false only when the address is malformed:
     *  a server that is not up yet is reported through the opened signal. */
    auto start(const std::string &host, uint16_t port) -> bool;
    auto stop() -> void;
    auto isConnected() -> bool;

    auto send(const char *buffer, size_t size) -> bool;

    sigslot::signal<const std::string&> receiveHandle;
    sigslot::signal<> opened;
    sigslot::signal<> closed;

private:
    auto run() -> void;

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_fail(connection_hdl hdl);
    void on_message(connection_hdl hdl, client::message_ptr msg);

    client m_endpoint;
    connection_hdl m_hdl;
    std::string m_uri;
    std::thread m_thread;
    std::mutex m_mutex;
    std::atomic_bool m_connected;
    std::atomic_bool m_running;
};
