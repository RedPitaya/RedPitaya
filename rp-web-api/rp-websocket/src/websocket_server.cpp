#include "websocket_server.h"
#include "common/rp_log.h"

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

websocket_server::websocket_server() {
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    // m_endpoint.set_access_channels(websocketpp::log::alevel::connect);
    // m_endpoint.set_access_channels(websocketpp::log::alevel::disconnect);
    // m_endpoint.set_access_channels(websocketpp::log::alevel::app);
    m_endpoint.init_asio();
    m_endpoint.set_open_handler(bind(&websocket_server::on_open, this, ::_1));
    m_endpoint.set_close_handler(bind(&websocket_server::on_close, this, ::_1));
    m_endpoint.set_message_handler(bind(&websocket_server::on_message, this, ::_1, ::_2));
    m_isRun = false;
    sem_init(&m_runSem, 0, 0);
}

websocket_server::~websocket_server() {
    receiveHandle.disconnect_all();
    stop();
}

void websocket_server::run(uint16_t port) {
    TRACE_SHORT("run websocket server")
    /* Listening is part of the thread: an exception leaving it (a busy port,
       for one) would call std::terminate instead of reaching the caller. */
    try {
        m_endpoint.set_reuse_addr(true);
        m_endpoint.listen(boost::asio::ip::tcp::v4(), port);
        m_endpoint.start_accept();
        m_isRun = true;
        sem_post(&m_runSem);
        m_endpoint.run();
    } catch (std::exception const& e) {
        ERROR_LOG("%s", e.what())
    }
    m_isRun = false;
    sem_post(&m_runSem);
}

void websocket_server::on_open(connection_hdl hdl) {
    TRACE_SHORT("Client connected")
    std::lock_guard lock(m_connectionsMutex);
    m_connections.insert(hdl);
}

void websocket_server::on_close(connection_hdl hdl) {
    TRACE_SHORT("Client disconnected")
    std::lock_guard lock(m_connectionsMutex);
    m_connections.erase(hdl);
}

void websocket_server::on_message(connection_hdl hdl, server::message_ptr msg) {
    receiveHandle(msg->get_payload());
}

auto websocket_server::send(const char* buffer, size_t size) -> bool {
    if (!m_isRun || size == 0)
        return false;
    /* The list is copied: the server thread adds and removes connections
       while the data is on its way out. */
    con_list connections;
    {
        std::lock_guard lock(m_connectionsMutex);
        connections = m_connections;
    }
    websocketpp::lib::error_code ec;
    for (auto it = connections.begin(); it != connections.end(); ++it) {
        m_endpoint.send(*it, buffer, size, websocketpp::frame::opcode::binary, ec);
        if (ec) {
            ERROR_LOG("%s", ec.message().c_str())
        }
    }
    return true;
}

auto websocket_server::start(uint16_t port) -> void {
    stop();
    std::lock_guard lock(m_mutex);
    /* Counted from zero, so the caller waits for the thread to be listening. */
    sem_destroy(&m_runSem);
    sem_init(&m_runSem, 0, 0);
    m_thread = std::thread(&websocket_server::run, this, port);
    if (sem_wait(&m_runSem)) {
        FATAL("Can't lock semaphore")
    }
}

auto websocket_server::stop() -> void {
    std::lock_guard lock(m_mutex);
    /* Called from the destructor as well, so nothing here may throw. */
    if (m_isRun) {
        websocketpp::lib::error_code ec;
        m_endpoint.stop_listening(ec);
        if (ec) {
            ERROR_LOG("%s", ec.message().c_str())
        }
        /* The handles are taken out first: closing them runs the close handler
           on the server thread, which asks for the same lock. */
        con_list connections;
        {
            std::lock_guard connectionsLock(m_connectionsMutex);
            connections.swap(m_connections);
        }
        for (auto it = connections.begin(); it != connections.end(); ++it) {
            m_endpoint.close(*it, websocketpp::close::status::normal, "shutdown", ec);
            if (ec) {
                ERROR_LOG("%s", ec.message().c_str())
            }
        }
        try {
            m_endpoint.stop();
        } catch (std::exception const& e) {
            ERROR_LOG("%s", e.what())
        }
    }
    /* Joined even when the thread never reached the listening state, otherwise
       the next start would assign over a running thread. */
    if (m_thread.joinable())
        m_thread.join();
}

auto websocket_server::isRun() -> bool {
    return m_isRun;
}
