#include <chrono>
#include <functional>
#include <thread>

#include "asio_socket_dma.h"
#include "logger_lib/file_logger.h"

#define MIN_BUFFER_SIZE (10 * 1024 * 1024)

using namespace net_lib;

auto CAsioSocketDMA::create(std::string host, uint16_t port, CBuffersCached::Ptr buffers) -> CAsioSocketDMA::Ptr {
    return std::make_shared<CAsioSocketDMA>(host, port, buffers);
}

CAsioSocketDMA::CAsioSocketDMA(std::string host, uint16_t port, CBuffersCached::Ptr buffers)
    : m_mode(net_lib::EMode::M_SERVER),
      m_host(host),
      m_port(port),
      m_tcp_socket(nullptr),
      m_tcp_acceptor(nullptr),
      m_mtx(),
      m_asio(new CAsioService()),
      m_bufferSize(MIN_BUFFER_SIZE),
      m_buffers(buffers),
      m_currentBuffer(nullptr),
      m_isStopReceive(false) {
    auto calculateSocketBuffer = []() -> uint64_t {
        auto getTotalSystemMemory = []() -> uint64_t {
#ifndef _WIN32
            uint64_t pages = sysconf(_SC_PHYS_PAGES);
            uint64_t page_size = sysconf(_SC_PAGE_SIZE);
            return pages * page_size;
#else
            MEMORYSTATUSEX status;
            status.dwLength = sizeof(status);
            GlobalMemoryStatusEx(&status);
            return status.ullTotalPhys;
#endif
        };

        auto totalSize = getTotalSystemMemory();
        auto calc = totalSize * 0.05;
        return MIN_BUFFER_SIZE > calc ? MIN_BUFFER_SIZE : calc;
    };

    m_bufferSize = calculateSocketBuffer();
}

CAsioSocketDMA::~CAsioSocketDMA() {
    stopReceive();
    closeSocket();
    // No handler runs after this; the sockets held by queued handlers go with the io_context.
    m_asio->stop();
    delete m_asio;
    m_asio = nullptr;
}

auto CAsioSocketDMA::initServer() -> void {
    closeSocket();
    auto acceptor = std::make_shared<asio::ip::tcp::acceptor>(m_asio->getIO());
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), m_port);
    acceptor->open(endpoint.protocol());
    acceptor->set_option(asio::ip::tcp::no_delay(true));
    acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
#ifndef _WIN32
    using quickack = asio::detail::socket_option::boolean<IPPROTO_TCP, TCP_QUICKACK>;
    acceptor->set_option(quickack(true));
#endif
    asio::socket_base::send_buffer_size option(m_bufferSize * 3);
    acceptor->set_option(option);
    acceptor->bind(endpoint);
    acceptor->listen();
    {
        std::lock_guard lock(m_mtx);
        m_tcp_acceptor = acceptor;
        m_sessionTaken = false;
        m_mode = net_lib::EMode::M_SERVER;
    }
    asio::post(m_asio->getIO(), [this]() { startAccept(); });
}

auto CAsioSocketDMA::startAccept() -> void {
    std::shared_ptr<asio::ip::tcp::acceptor> acceptor;
    {
        std::lock_guard lock(m_mtx);
        acceptor = m_tcp_acceptor;
    }
    if (!acceptor)
        return;
    auto socket = std::make_shared<asio::ip::tcp::socket>(m_asio->getIO());
    acceptor->async_accept(*socket, [this, socket, acceptor](const asio::error_code& error) { handlerAccept(error, socket); });
}

// The first client is the session's; a later one is closed at once instead of being left
// connected without data.
auto CAsioSocketDMA::handlerAccept(const asio::error_code& _error, socket_ptr socket) -> void {
    if (_error) {
        if (_error == asio::error::operation_aborted)
            return;
        errorServerNotify(_error);
        startAccept();
        return;
    }
    asio::error_code ignored;
    auto endpoint = socket->remote_endpoint(ignored);
    std::string peer = ignored ? "" : endpoint.address().to_string();
    bool take = false;
    {
        std::lock_guard lock(m_mtx);
        if (!m_tcp_acceptor) {
            socket->close(ignored);
            return;
        }
        take = !m_sessionTaken;
        if (take) {
            m_sessionTaken = true;
            m_tcp_socket = socket;
            m_connected = true;
            m_peer = peer;
        }
    }
    if (!take) {
        socket->close(ignored);
        startAccept();
        return;
    }
    socket->set_option(asio::ip::tcp::no_delay(true), ignored);
    connectServerNotify(peer);
    if (m_buffers) {
        getBuffer();
        if (m_currentBuffer)
            startReceive(socket);
    } else {
        startWatch(socket);
    }
    startAccept();
}

auto CAsioSocketDMA::initClient() -> void {
    asio::error_code error;
    asio::ip::tcp::resolver resolver(m_asio->getIO());
    auto endpoints = resolver.resolve(m_host, std::to_string(m_port), error);
    if (!error && endpoints.empty())
        error = asio::error::host_not_found;
    if (error) {
        errorClientNotify(error);
        return;
    }
    auto endpoint = endpoints.begin()->endpoint();
    auto socket = std::make_shared<asio::ip::tcp::socket>(m_asio->getIO());
    {
        std::lock_guard lock(m_mtx);
        m_tcp_socket = socket;
        m_connected = false;
        m_peer = endpoint.address().to_string();
        m_mode = net_lib::EMode::M_CLIENT;
    }
    asio::post(m_asio->getIO(), [this, socket, endpoint]() {
        {
            std::lock_guard lock(m_mtx);
            if (socket != m_tcp_socket)
                return;
        }
        socket->async_connect(endpoint, [this, socket](const asio::error_code& e) { handlerConnect(e, socket); });
    });
}

auto CAsioSocketDMA::handlerConnect(const asio::error_code& _error, socket_ptr socket) -> void {
    std::string peer;
    {
        std::lock_guard lock(m_mtx);
        if (socket != m_tcp_socket)
            return;
        if (_error) {
            m_tcp_socket = nullptr;
        } else {
            m_connected = true;
        }
        peer = m_peer;
    }
    asio::error_code ignored;
    if (_error) {
        socket->close(ignored);
        if (_error != asio::error::operation_aborted)
            errorClientNotify(_error);
        return;
    }
    socket->set_option(asio::ip::tcp::no_delay(true), ignored);
#ifndef _WIN32
    using quickack = asio::detail::socket_option::boolean<IPPROTO_TCP, TCP_QUICKACK>;
    socket->set_option(quickack(true), ignored);
#endif
    socket->set_option(asio::socket_base::receive_buffer_size(m_bufferSize * 3), ignored);
    // Before the first buffer is taken: a client may create its ring in this handler.
    connectClientNotify(peer);
    if (m_buffers) {
        getBuffer();
        if (m_currentBuffer)
            startReceive(socket);
    } else {
        startWatch(socket);
    }
}

auto CAsioSocketDMA::stopReceive() -> void {
    m_isStopReceive = true;
}

auto CAsioSocketDMA::getBuffer() -> void {
    if (m_buffers == nullptr)
        return;
    // Waiting for a free buffer is the flow control of the receiver: the peer waits in send.
    while (!m_isStopReceive && m_currentBuffer == nullptr) {
        const auto begin = std::chrono::steady_clock::now();
        m_currentBuffer = m_buffers->writeBuffer(true);
        if (m_currentBuffer) {
            m_currentBuffer->resetWriteSizeAll();
        } else if (std::chrono::steady_clock::now() - begin < std::chrono::milliseconds(100)) {
            // No ring yet (writeBuffer returns at once): do not spin.
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

auto CAsioSocketDMA::unlockBuffer() -> void {
    if (m_buffers == nullptr)
        return;
    if (m_currentBuffer != nullptr) {
        m_buffers->unlockBufferWrite();
        m_currentBuffer = nullptr;
    }
}

auto CAsioSocketDMA::isConnected() -> bool {
    std::lock_guard lock(m_mtx);
    return m_tcp_socket && m_connected;
}

auto CAsioSocketDMA::cancelSocket() -> void {
    socket_ptr socket;
    std::shared_ptr<asio::ip::tcp::acceptor> acceptor;
    {
        std::lock_guard lock(m_mtx);
        socket = m_tcp_socket;
        acceptor = m_tcp_acceptor;
    }
    asio::post(m_asio->getIO(), [socket, acceptor]() {
        asio::error_code ignored;
        if (acceptor)
            acceptor->cancel(ignored);
        if (socket)
            socket->cancel(ignored);
    });
}

auto CAsioSocketDMA::emitError(const asio::error_code& _error) -> void {
    if (m_mode == net_lib::EMode::M_SERVER) {
        errorServerNotify(_error);
    } else {
        errorClientNotify(_error);
    }
}

auto CAsioSocketDMA::emitDisconnect(std::string peer) -> void {
    if (m_mode == net_lib::EMode::M_SERVER) {
        disconnectServerNotify(peer);
    } else {
        disconnectClientNotify(peer);
    }
}

auto CAsioSocketDMA::closeSocket() -> void {
    socket_ptr socket;
    std::shared_ptr<asio::ip::tcp::acceptor> acceptor;
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
    }
    asio::error_code ignored;
    // shutdown wakes a sender blocked on a peer that does not read; the descriptor itself is
    // closed when the last holder (a sender, a pending handler) lets the socket go.
    if (socket)
        socket->shutdown(asio::socket_base::shutdown_both, ignored);
    if (socket || acceptor) {
        asio::post(m_asio->getIO(), [socket, acceptor]() {
            asio::error_code ignored;
            if (socket)
                socket->cancel(ignored);
            if (acceptor)
                acceptor->close(ignored);
        });
    }
    if (wasConnected)
        emitDisconnect(peer);
}

auto CAsioSocketDMA::dropConnection(socket_ptr socket) -> void {
    bool wasConnected = false;
    std::string peer;
    {
        std::lock_guard lock(m_mtx);
        if (socket != m_tcp_socket)
            return;
        wasConnected = m_connected;
        peer = m_peer;
        m_tcp_socket = nullptr;
        m_connected = false;
    }
    asio::error_code ignored;
    socket->shutdown(asio::socket_base::shutdown_both, ignored);
    asio::post(m_asio->getIO(), [socket]() {
        asio::error_code ignored;
        socket->cancel(ignored);
    });
    if (wasConnected)
        emitDisconnect(peer);
}

auto CAsioSocketDMA::startReceive(socket_ptr socket) -> void {
    auto buff = m_currentBuffer->getNextDMABufferForWrite();
    if (buff == nullptr) {
        FATAL("Buffer is null")
    }
    auto sizeLeft = buff->getWriteSizeLeft();
    auto offset = buff->getBufferFullLenght() - sizeLeft;
    socket->async_receive(asio::buffer((char*)buff->getMappedMemory() + offset, sizeLeft),
                          [this, socket](const asio::error_code& e, size_t n) { handlerReceive(e, n, socket); });
}

auto CAsioSocketDMA::handlerReceive(const asio::error_code& _error, size_t bytes_transferred, socket_ptr socket) -> void {
    if (_error == asio::error::operation_aborted)
        return;
    {
        std::lock_guard lock(m_mtx);
        if (socket != m_tcp_socket)
            return;
    }
    if (_error) {
        emitError(_error);
        dropConnection(socket);
        return;
    }
    if (!m_currentBuffer) {
        FATAL("There is no buffer for writing.")
    }
    auto b = m_currentBuffer->getNextDMABufferForWrite();
    if (b) {
        b->addWriteSize(bytes_transferred);
    }
    if (m_currentBuffer->isAllDataWrite()) {
        m_currentBuffer->verifyPack();
        recivedNotify(_error, m_currentBuffer);
        unlockBuffer();
        getBuffer();
    }
    {
        std::lock_guard lock(m_mtx);
        if (socket != m_tcp_socket)
            return;
    }
    if (m_currentBuffer)
        startReceive(socket);
}

// The ADC server and the DAC client get no data from the peer; a read is kept pending only to
// see it close.
auto CAsioSocketDMA::startWatch(socket_ptr socket) -> void {
    socket->async_read_some(asio::buffer(m_watchBuffer), [this, socket](const asio::error_code& e, size_t) {
        if (e == asio::error::operation_aborted)
            return;
        {
            std::lock_guard lock(m_mtx);
            if (socket != m_tcp_socket)
                return;
        }
        if (e) {
            emitError(e);
            dropConnection(socket);
            return;
        }
        startWatch(socket);
    });
}

auto CAsioSocketDMA::sendSyncBuffer(DataLib::CDataBuffersPackDMA::Ptr _buffer) -> bool {
    socket_ptr socket;
    {
        std::lock_guard lock(m_mtx);
        if (m_connected)
            socket = m_tcp_socket;
    }
    if (!socket) {
        handlerSend(asio::error::not_connected, 0);
        return false;
    }
    std::lock_guard send(m_sendMtx);
    asio::error_code error;
    size_t sent = 0;
    for (auto i = (int)DataLib::EDataBuffersPackChannel::CH1; i <= (int)DataLib::EDataBuffersPackChannel::CH4 && !error; i++) {
        auto buff = _buffer->getBuffer((DataLib::EDataBuffersPackChannel)i);
        if (buff == NULL)
            continue;
        size_t needSendSize = buff->getBufferFullLenght();
        size_t offset = 0;
        // Blocks while the peer does not read: that is the flow control of the stream.
        while (needSendSize && !error) {
            size_t sendedSize = socket->send(asio::buffer((char*)buff->getMappedMemory() + offset, needSendSize), 0, error);
            offset += sendedSize;
            needSendSize -= sendedSize;
            sent += sendedSize;
        }
    }
    handlerSend(error, error ? sent : _buffer->getLenghtBuffers());
    if (error) {
        dropConnection(socket);
        return false;
    }
    return true;
}

void CAsioSocketDMA::handlerSend(const asio::error_code& _error, size_t _bytesTransferred) {
    sendNotify(_error, _bytesTransferred);
}
