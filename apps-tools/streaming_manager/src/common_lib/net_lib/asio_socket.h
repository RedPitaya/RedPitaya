#ifndef NET_LIB_ASIO_SOCKET_H
#define NET_LIB_ASIO_SOCKET_H

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
#include "data_lib/neon_asm.h"
#include "data_lib/signal.hpp"
#include "asio.hpp"

#define  SOCKET_BUFFER_SIZE 65536
#define  FIFO_BUFFER_SIZE  SOCKET_BUFFER_SIZE * 3

using  namespace std;

namespace  net_lib {

class CAsioSocket {
public:

    using Ptr = shared_ptr<CAsioSocket>;

    static Ptr create(net_lib::EProtocol _protocol, string host, string port);

    CAsioSocket(net_lib::EProtocol _protocol, string host, string port);
    ~CAsioSocket();

    auto initServer() -> void;
    auto initClient() -> void;
    auto closeSocket() -> void;
    auto isConnected() -> bool;
    auto sendBuffer(net_buffer _buffer, size_t _size) -> void;
    auto sendBuffer(bool async,net_buffer _buffer, size_t _size) -> bool;
    auto sendSyncBuffer(AsioBufferNolder &_buffer) -> bool;

    sigslot::signal<string&>    connectServerNotify;
    sigslot::signal<string&>    disconnectServerNotify;

    sigslot::signal<string&>    connectClientNotify;
    sigslot::signal<string&>    disconnectClientNotify;

    sigslot::signal<error_code> errorServerNotify;
    sigslot::signal<error_code> errorClientNotify;

    sigslot::signal<error_code,size_t> sendNotify;
    sigslot::signal<error_code,uint8_t*,size_t> recivedNotify;

private:

    CAsioSocket(const CAsioSocket &) = delete;
    CAsioSocket(CAsioSocket &&) = delete;
    CAsioSocket& operator=(const CAsioSocket&) =delete;
    CAsioSocket& operator=(const CAsioSocket&&) =delete;

    auto waitClient() -> void;
    auto handlerReceiveFromClient(const asio::error_code &error) -> void;
    auto handlerAcceptFromClient(const asio::error_code &_error) -> void;
    auto handlerConnectToServer(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator) -> void;
    auto handlerSend(const asio::error_code &_error, size_t _bytesTransferred) -> void;
    auto handlerSend2(const asio::error_code &_error, size_t _bytesTransferred,uint64_t bufferId) -> void;
    auto handlerReceiveFromServer(const asio::error_code &ErrorCode, size_t bytes_transferred) -> void;

    net_lib::EMode m_mode;
    net_lib::EProtocol m_protocol;
    string m_host;
    string m_port;

    shared_ptr<asio::ip::udp::udp::socket> m_udp_socket;
    shared_ptr<asio::ip::tcp::socket> m_tcp_socket;
    shared_ptr<asio::ip::tcp::acceptor> m_tcp_acceptor;
    asio::ip::udp::udp::endpoint m_udp_endpoint;
    asio::ip::tcp::endpoint m_tcp_endpoint;

    uint8_t  *m_SocketReadBuffer;
    char m_udp_recv_server_buffer[1];
    uint8_t  *m_tcp_fifo_buffer;
    uint32_t  m_pos_last_in_fifo;
    uint64_t  m_last_pack_id;
    std::mutex m_mtx;
};

}

#endif


