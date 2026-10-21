#ifndef NET_LIB_ASIO_SOCKET_SIMPLE_H
#define NET_LIB_ASIO_SOCKET_SIMPLE_H

#include <deque>
#include <future>
#include <mutex>

#include "asio.hpp"
#include "asio_common.h"
#include "asio_service.h"
#include "data_lib/signal.hpp"

namespace net_lib {

// One TCP connection: a server keeps the newest client (a new one replaces the old), a client
// connects once. Every socket operation runs on the asio thread; the signals are emitted
// without the internal lock held, so a handler may call back into the object.
class CAsioSocketSimple {
   public:
    using Ptr = std::shared_ptr<CAsioSocketSimple>;
    typedef std::shared_ptr<asio::ip::tcp::socket> socket_ptr;
    typedef std::shared_ptr<asio::ip::tcp::acceptor> acceptor_ptr;

    static Ptr create(std::string host, uint16_t port);

    CAsioSocketSimple(std::string host, uint16_t port);
    ~CAsioSocketSimple();

    auto initServer() -> void;
    auto initClient() -> void;
    auto closeSocket() -> void;
    // True only while a connection is established.
    auto isConnected() -> bool;
    auto runningInIoThread() -> bool;

    // Buffers are written whole and in order. With async == false the call waits until the
    // buffer is written, except on the asio thread, where it only queues it.
    auto sendBuffer(bool async, net_lib::net_buffer _buffer, size_t _size) -> bool;

    sigslot::signal<std::string&> connectNotify;
    sigslot::signal<std::string&> disconnectNotify;

    sigslot::signal<std::error_code> errorNotify;
    sigslot::signal<std::error_code> connectTimeoutNotify;

    sigslot::signal<std::error_code, size_t> sendNotify;
    sigslot::signal<std::error_code, uint8_t*, size_t> recivedNotify;

   private:
    CAsioSocketSimple(const CAsioSocketSimple&) = delete;
    CAsioSocketSimple(CAsioSocketSimple&&) = delete;
    CAsioSocketSimple& operator=(const CAsioSocketSimple&) = delete;
    CAsioSocketSimple& operator=(const CAsioSocketSimple&&) = delete;

    using read_buffer = std::shared_ptr<uint8_t[]>;

    struct PendingWrite {
        net_buffer buffer;
        size_t size;
        std::shared_ptr<std::promise<bool>> done;
    };

    auto startAccept() -> void;
    auto startReceive(socket_ptr socket, read_buffer buffer) -> void;
    auto startWrite(socket_ptr socket) -> void;
    auto handlerAccept(const asio::error_code& _error, socket_ptr socket) -> void;
    auto handlerConnect(const asio::error_code& _error, socket_ptr socket) -> void;
    auto handlerReceive(const asio::error_code& _error, size_t bytes_transferred, socket_ptr socket, read_buffer buffer) -> void;
    auto handlerWrite(const asio::error_code& _error, size_t _bytesTransferred, socket_ptr socket) -> void;
    // Closes the current connection if it is still this socket; emits disconnectNotify if it was established.
    auto dropConnection(socket_ptr socket) -> void;
    auto closeOnIoThread(socket_ptr socket, acceptor_ptr acceptor) -> void;
    // Under m_mtx: the queued buffers are reported as not sent.
    auto failQueueLocked() -> void;
    auto isCurrentLocked(const socket_ptr& socket) -> bool;

    net_lib::EMode m_mode;
    std::string m_host;
    uint16_t m_port;

    socket_ptr m_tcp_socket = nullptr;
    acceptor_ptr m_tcp_acceptor = nullptr;
    std::string m_peer;
    bool m_connected = false;
    CAsioService* m_asio;
    // Destroyed before m_asio: a timer must not outlive its io_context.
    std::unique_ptr<asio::steady_timer> m_timoutTimer;
    std::mutex m_mtx;

    std::deque<PendingWrite> m_writeQueue;
    bool m_writing = false;
};

}  // namespace net_lib

#endif
