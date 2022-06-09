#ifndef NET_LIB_ASIO_SOCKET_SIMPLE_H
#define NET_LIB_ASIO_SOCKET_SIMPLE_H

#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <functional>
#include <system_error>
#include <cstdint>
#include <memory>
#include <deque>

#include "asio_common.h"
#include "asio.hpp"
#include "data_lib/signal.hpp"
#include "data_lib/neon_asm.h"
#include "asio_service.h"

namespace  net_lib {

using namespace std;

class CAsioSocketSimple {
public:

    using Ptr = std::shared_ptr<CAsioSocketSimple>;

    static Ptr create(std::string host, std::string port);

    CAsioSocketSimple(std::string host, std::string port);
    ~CAsioSocketSimple();

    auto initServer() -> void;
    auto initClient() -> void;
    auto closeSocket() -> void;
    auto isConnected() -> bool;

    auto sendBuffer(bool async, net_lib::net_buffer _buffer, size_t _size) -> bool;

    sigslot::signal<string&>    connectNotify;
    sigslot::signal<string&>    disconnectNotify;

    sigslot::signal<error_code> errorNotify;
    sigslot::signal<error_code> connectTimeoutNotify;

    sigslot::signal<error_code,size_t> sendNotify;
    sigslot::signal<error_code,uint8_t*,size_t> recivedNotify;

private:

    CAsioSocketSimple(const CAsioSocketSimple &) = delete;
    CAsioSocketSimple(CAsioSocketSimple &&) = delete;
    CAsioSocketSimple& operator=(const CAsioSocketSimple&) =delete;
    CAsioSocketSimple& operator=(const CAsioSocketSimple&&) =delete;

    auto handlerReceive(const asio::error_code &error,size_t bytes_transferred) -> void;
    auto handlerAccept(const asio::error_code &_error) -> void;
    auto handlerConnect(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator) -> void;
    auto handlerSend(const asio::error_code &_error, size_t _bytesTransferred) -> void;
    auto handlerSend2(const asio::error_code &_error, size_t _bytesTransferred, uint64_t bufferId) -> void;
    auto handlerReceiveFromServer(const asio::error_code &ErrorCode, size_t bytes_transferred) -> void;

    net_lib::EMode m_mode;
    std::string m_host;
    std::string m_port;

    std::shared_ptr<asio::ip::tcp::socket> m_tcp_socket;
    std::shared_ptr<asio::ip::tcp::acceptor> m_tcp_acceptor;
    asio::ip::tcp::endpoint m_tcp_endpoint;
    CAsioService *m_asio;
    asio::steady_timer  m_timoutTimer;
    uint8_t *m_SocketReadBuffer;
    bool m_disableRestartServer;
    std::mutex m_mtx;
};

}

#endif


