#include "websocket_client.h"
#include "common/rp_log.h"

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

websocket_client::websocket_client() : m_connected(false), m_running(false) {
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
    m_endpoint.init_asio();
    m_endpoint.set_open_handler(bind(&websocket_client::on_open, this, ::_1));
    m_endpoint.set_close_handler(bind(&websocket_client::on_close, this, ::_1));
    m_endpoint.set_fail_handler(bind(&websocket_client::on_fail, this, ::_1));
    m_endpoint.set_message_handler(bind(&websocket_client::on_message, this, ::_1, ::_2));
}

websocket_client::~websocket_client() {
    receiveHandle.disconnect_all();
    opened.disconnect_all();
    closed.disconnect_all();
    stop();
}

auto websocket_client::start(const std::string &host, uint16_t port) -> bool {
    stop();
    std::lock_guard lock(m_mutex);

    m_uri = "ws://" + host + ":" + std::to_string(port);

    websocketpp::lib::error_code ec;
    auto connection = m_endpoint.get_connection(m_uri, ec);
    if (ec) {
        ERROR_LOG("%s", ec.message().c_str())
        return false;
    }

    m_endpoint.connect(connection);
    m_running = true;
    m_thread = std::thread(&websocket_client::run, this);
    return true;
}

auto websocket_client::run() -> void {
    try {
        m_endpoint.run();
    } catch (websocketpp::exception const &e) {
        ERROR_LOG("%s", e.what())
    }
    m_running = false;
    m_connected = false;
}

auto websocket_client::stop() -> void {
    std::lock_guard lock(m_mutex);
    if (!m_running && !m_thread.joinable()) {
        return;
    }

    if (m_connected) {
        websocketpp::lib::error_code ec;
        m_endpoint.close(m_hdl, websocketpp::close::status::normal, "shutdown", ec);
    }

    m_endpoint.stop();
    if (m_thread.joinable()) {
        m_thread.join();
    }
    // The endpoint keeps its io_service state after run() returned, so a later
    // start() has to begin from a clean one.
    m_endpoint.reset();
    m_connected = false;
    m_running = false;
}

auto websocket_client::isConnected() -> bool {
    return m_connected;
}

auto websocket_client::send(const char *buffer, size_t size) -> bool {
    if (!m_connected || size == 0) {
        return false;
    }

    websocketpp::lib::error_code ec;
    m_endpoint.send(m_hdl, buffer, size, websocketpp::frame::opcode::text, ec);
    if (ec) {
        ERROR_LOG("%s", ec.message().c_str())
        return false;
    }
    return true;
}

void websocket_client::on_open(connection_hdl hdl) {
    TRACE_SHORT("Connected to the websocket server")
    m_hdl = hdl;
    m_connected = true;
    opened();
}

void websocket_client::on_close(connection_hdl hdl) {
    (void)hdl;
    TRACE_SHORT("Disconnected from the websocket server")
    m_connected = false;
    closed();
}

void websocket_client::on_fail(connection_hdl hdl) {
    (void)hdl;
    m_connected = false;
    closed();
}

void websocket_client::on_message(connection_hdl hdl, client::message_ptr msg) {
    (void)hdl;
    receiveHandle(msg->get_payload());
}
