#include "asio_socket_simple.h"
#include <functional>

#define SOCKET_BUFFER_SIZE 4096
#define CONNECT_TIMEOUT 5

using namespace net_lib;

auto CAsioSocketSimple::create(std::string host, uint16_t port) -> CAsioSocketSimple::Ptr {
    return std::make_shared<CAsioSocketSimple>(host, port);
}

CAsioSocketSimple::CAsioSocketSimple(std::string host, uint16_t port) : m_mode(EMode::M_SERVER), m_host(host), m_port(port), m_asio(new CAsioService()) {
    m_timoutTimer = std::make_unique<asio::steady_timer>(m_asio->getIO());
}

CAsioSocketSimple::~CAsioSocketSimple() {
    closeSocket();
    // No handler runs after this; the sockets held by queued handlers go with the io_context.
    m_asio->stop();
    {
        std::lock_guard lock(m_mtx);
        failQueueLocked();
    }
    m_timoutTimer.reset();
    delete m_asio;
    m_asio = nullptr;
}

auto CAsioSocketSimple::runningInIoThread() -> bool {
    return m_asio->runningInThisThread();
}

auto CAsioSocketSimple::isCurrentLocked(const socket_ptr& socket) -> bool {
    return socket && m_tcp_socket == socket;
}

auto CAsioSocketSimple::failQueueLocked() -> void {
    for (auto& w : m_writeQueue) {
        if (w.done)
            w.done->set_value(false);
    }
    m_writeQueue.clear();
    m_writing = false;
}

auto CAsioSocketSimple::closeOnIoThread(socket_ptr socket, acceptor_ptr acceptor) -> void {
    auto close = [socket, acceptor]() {
        asio::error_code ignored;
        if (socket) {
            socket->cancel(ignored);
            socket->close(ignored);
        }
        if (acceptor) {
            acceptor->cancel(ignored);
            acceptor->close(ignored);
        }
    };
    if (runningInIoThread()) {
        close();
    } else {
        asio::post(m_asio->getIO(), close);
    }
}

auto CAsioSocketSimple::initServer() -> void {
    closeSocket();
    auto acceptor = std::make_shared<asio::ip::tcp::acceptor>(m_asio->getIO());
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), (int)m_port);
    acceptor->open(endpoint.protocol());
    acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor->bind(endpoint);
    acceptor->listen();
    {
        std::lock_guard lock(m_mtx);
        m_tcp_acceptor = acceptor;
        m_mode = EMode::M_SERVER;
    }
    asio::post(m_asio->getIO(), [this]() { startAccept(); });
}

auto CAsioSocketSimple::startAccept() -> void {
    acceptor_ptr acceptor;
    {
        std::lock_guard lock(m_mtx);
        acceptor = m_tcp_acceptor;
    }
    if (!acceptor)
        return;
    auto socket = std::make_shared<asio::ip::tcp::socket>(m_asio->getIO());
    acceptor->async_accept(*socket, [this, socket, acceptor](const asio::error_code& error) { handlerAccept(error, socket); });
}

// One client at a time: a new connection replaces the current one.
auto CAsioSocketSimple::handlerAccept(const asio::error_code& _error, socket_ptr socket) -> void {
    if (_error) {
        if (_error != asio::error::operation_aborted)
            errorNotify(_error);
        return;
    }
    asio::error_code epError;
    auto endpoint = socket->remote_endpoint(epError);
    std::string peer = epError ? "" : endpoint.address().to_string();
    socket_ptr old;
    bool oldConnected = false;
    std::string oldPeer;
    {
        std::lock_guard lock(m_mtx);
        if (!m_tcp_acceptor) {
            asio::error_code ignored;
            socket->close(ignored);
            return;
        }
        old = m_tcp_socket;
        oldConnected = m_connected;
        oldPeer = m_peer;
        failQueueLocked();
        m_tcp_socket = socket;
        m_connected = true;
        m_peer = peer;
    }
    if (old) {
        closeOnIoThread(old, nullptr);
        if (oldConnected)
            disconnectNotify(oldPeer);
    }
    connectNotify(peer);
    startReceive(socket, read_buffer(new uint8_t[SOCKET_BUFFER_SIZE]));
    startAccept();
}

auto CAsioSocketSimple::initClient() -> void {
    asio::error_code error;
    asio::ip::tcp::resolver resolver(m_asio->getIO());
    auto endpoints = resolver.resolve(m_host, std::to_string(m_port), error);
    if (!error && endpoints.empty())
        error = asio::error::host_not_found;
    if (error) {
        errorNotify(error);
        return;
    }
    auto endpoint = endpoints.begin()->endpoint();
    auto socket = std::make_shared<asio::ip::tcp::socket>(m_asio->getIO());
    {
        std::lock_guard lock(m_mtx);
        failQueueLocked();
        m_tcp_acceptor = nullptr;
        m_tcp_socket = socket;
        m_connected = false;
        m_peer = endpoint.address().to_string();
        m_mode = EMode::M_CLIENT;
    }
    asio::post(m_asio->getIO(), [this, socket, endpoint]() {
        {
            std::lock_guard lock(m_mtx);
            if (!isCurrentLocked(socket))
                return;
        }
        socket->async_connect(endpoint, [this, socket](const asio::error_code& e) { handlerConnect(e, socket); });
        m_timoutTimer->expires_after(std::chrono::seconds(CONNECT_TIMEOUT));
        m_timoutTimer->async_wait([this, socket](std::error_code er) {
            if (er == asio::error::operation_aborted)
                return;
            bool timedOut = false;
            {
                std::lock_guard lock(m_mtx);
                if (isCurrentLocked(socket) && !m_connected) {
                    m_tcp_socket = nullptr;
                    failQueueLocked();
                    timedOut = true;
                }
            }
            if (timedOut) {
                closeOnIoThread(socket, nullptr);
                connectTimeoutNotify(er);
            }
        });
    });
}

auto CAsioSocketSimple::handlerConnect(const asio::error_code& _error, socket_ptr socket) -> void {
    std::string peer;
    {
        std::lock_guard lock(m_mtx);
        if (!isCurrentLocked(socket))
            return;
        m_timoutTimer->cancel();
        if (_error) {
            m_tcp_socket = nullptr;
            failQueueLocked();
        } else {
            m_connected = true;
        }
        peer = m_peer;
    }
    if (_error) {
        closeOnIoThread(socket, nullptr);
        if (_error != asio::error::operation_aborted)
            errorNotify(_error);
        return;
    }
    connectNotify(peer);
    startReceive(socket, read_buffer(new uint8_t[SOCKET_BUFFER_SIZE]));
}

auto CAsioSocketSimple::closeSocket() -> void {
    socket_ptr socket;
    acceptor_ptr acceptor;
    bool wasConnected = false;
    std::string peer;
    {
        std::lock_guard lock(m_mtx);
        socket = m_tcp_socket;
        acceptor = m_tcp_acceptor;
        wasConnected = m_connected;
        peer = m_peer;
        m_tcp_socket = nullptr;
        m_tcp_acceptor = nullptr;
        m_connected = false;
        failQueueLocked();
    }
    if (socket || acceptor)
        closeOnIoThread(socket, acceptor);
    if (wasConnected)
        disconnectNotify(peer);
}

auto CAsioSocketSimple::dropConnection(socket_ptr socket) -> void {
    bool wasConnected = false;
    std::string peer;
    {
        std::lock_guard lock(m_mtx);
        if (!isCurrentLocked(socket))
            return;
        wasConnected = m_connected;
        peer = m_peer;
        m_tcp_socket = nullptr;
        m_connected = false;
        failQueueLocked();
    }
    closeOnIoThread(socket, nullptr);
    if (wasConnected)
        disconnectNotify(peer);
}

auto CAsioSocketSimple::startReceive(socket_ptr socket, read_buffer buffer) -> void {
    socket->async_receive(asio::buffer(buffer.get(), SOCKET_BUFFER_SIZE),
                          [this, socket, buffer](const asio::error_code& e, size_t n) { handlerReceive(e, n, socket, buffer); });
}

auto CAsioSocketSimple::handlerReceive(const asio::error_code& _error, size_t bytes_transferred, socket_ptr socket, read_buffer buffer) -> void {
    {
        std::lock_guard lock(m_mtx);
        if (!isCurrentLocked(socket))
            return;
    }
    if (!_error) {
        recivedNotify(_error, buffer.get(), bytes_transferred);
        {
            std::lock_guard lock(m_mtx);
            if (!isCurrentLocked(socket))
                return;
        }
        startReceive(socket, buffer);
        return;
    }
    if (_error == asio::error::operation_aborted)
        return;
    // The peer closed or the connection broke: report it and close our side.
    errorNotify(_error);
    dropConnection(socket);
}

auto CAsioSocketSimple::isConnected() -> bool {
    std::lock_guard lock(m_mtx);
    return m_tcp_socket && m_connected;
}

auto CAsioSocketSimple::sendBuffer(bool async, net_buffer _buffer, size_t _size) -> bool {
    const bool wait = !async && !runningInIoThread();
    std::shared_ptr<std::promise<bool>> done;
    std::future<bool> result;
    if (wait) {
        done = std::make_shared<std::promise<bool>>();
        result = done->get_future();
    }
    {
        std::lock_guard lock(m_mtx);
        if (!m_tcp_socket || !m_connected)
            return false;
        m_writeQueue.push_back({_buffer, _size, done});
        if (!m_writing) {
            m_writing = true;
            auto socket = m_tcp_socket;
            asio::post(m_asio->getIO(), [this, socket]() { startWrite(socket); });
        }
    }
    if (!wait)
        return true;
    return result.get();
}

auto CAsioSocketSimple::startWrite(socket_ptr socket) -> void {
    net_buffer buffer;
    size_t size = 0;
    {
        std::lock_guard lock(m_mtx);
        if (!isCurrentLocked(socket))
            return;
        if (m_writeQueue.empty()) {
            m_writing = false;
            return;
        }
        buffer = m_writeQueue.front().buffer;
        size = m_writeQueue.front().size;
    }
    // async_write, not async_send: the buffer goes out whole or the connection is dropped.
    asio::async_write(*socket, asio::buffer(buffer.get(), size),
                      [this, socket, buffer](const asio::error_code& e, size_t n) { handlerWrite(e, n, socket); });
}

auto CAsioSocketSimple::handlerWrite(const asio::error_code& _error, size_t _bytesTransferred, socket_ptr socket) -> void {
    std::shared_ptr<std::promise<bool>> done;
    bool more = false;
    {
        std::lock_guard lock(m_mtx);
        if (!isCurrentLocked(socket))
            return;
        if (!m_writeQueue.empty()) {
            done = m_writeQueue.front().done;
            m_writeQueue.pop_front();
        }
        if (!_error) {
            more = !m_writeQueue.empty();
            if (!more)
                m_writing = false;
        }
    }
    if (done)
        done->set_value(!_error);
    if (_error == asio::error::operation_aborted)
        return;
    sendNotify(_error, _bytesTransferred);
    if (_error) {
        errorNotify(_error);
        dropConnection(socket);
        return;
    }
    if (more)
        startWrite(socket);
}
