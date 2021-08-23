#pragma once

#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <functional>
#include <system_error>
#include <cstdint>
#include <memory>
#include <deque>

#include "neon_asm.h"
#include "asio.hpp"
#include "EventHandlers.h"


#define  SOCKET_BUFFER_SIZE 65536
#define  FIFO_BUFFER_SIZE  SOCKET_BUFFER_SIZE * 3

using  namespace std;
using  namespace asio;

namespace  asionet {

    extern const char ID_PACK[];

    enum Protocol {
        TCP,
        UDP
    };

    enum Mode {
        SERVER,
        CLIENT,
        NONE
    };

    enum Events{
        CONNECT_SERVER,
        DISCONNECT_SERVER,
        CONNECT_CLIENT,
        DISCONNECT_CLIENT,
        ERROR_SERVER,
        ERROR_CLIENT,
        SEND_DATA,
        RECIVED_DATA_FROM_SERVER};

    class CAsioSocket {
    public:
        typedef uint8_t* send_buffer;
       

        using Ptr = shared_ptr<CAsioSocket>;

        static Ptr Create(asio::io_service &io, Protocol _protocol, string host, string port);
        CAsioSocket(asio::io_service &io, Protocol _protocol, string host, string port);
        ~CAsioSocket();

        void InitServer();
        void InitClient();
        void CloseSocket();
        bool IsConnected();
        void SendBuffer(const void *_buffer, size_t _size);
        bool SendBuffer(bool async,send_buffer _buffer, size_t _size);
        void addHandler(Events _event, std::function<void(string host)> _func);
        void addHandler(Events _event, std::function<void(error_code error)> _func);
        void addHandler(Events _event, std::function<void(error_code error,size_t)> _func);
        void addHandler(Events _event, std::function<void(error_code error,uint8_t*,size_t)> _func);

    private:

        CAsioSocket(const CAsioSocket &) = delete;
        CAsioSocket(CAsioSocket &&) = delete;


        void WaitClient();
        void HandlerReceiveFromClient(const asio::error_code &error);
        void HandlerAcceptFromClient(const asio::error_code &_error);
        void HandlerConnectToServer(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator);
        void HandlerSend(const asio::error_code &_error, size_t _bytesTransferred);
        void HandlerSend2(const asio::error_code &_error, size_t _bytesTransferred, uint8_t *buffer);
        void HandlerReceiveFromServer(const asio::error_code &ErrorCode, size_t bytes_transferred);

        Mode m_mode;
        Protocol m_protocol;
        string m_host;
        string m_port;
        io_service &m_io_service;

        shared_ptr<asio::ip::udp::udp::socket> m_udp_socket;
        shared_ptr<asio::ip::tcp::socket> m_tcp_socket;
        shared_ptr<asio::ip::tcp::acceptor> m_tcp_acceptor;
        asio::ip::udp::udp::endpoint m_udp_endpoint;
        asio::ip::tcp::endpoint m_tcp_endpoint;

        uint8_t *m_SocketReadBuffer;
        char m_udp_recv_server_buffer[1];
        bool m_is_udp_connected;
        bool m_is_tcp_connected;
        uint8_t  *m_tcp_fifo_buffer;
        uint32_t  m_pos_last_in_fifo;
        uint64_t  m_last_pack_id;


        EventList<std::string> m_callback_Str;
        EventList<std::error_code> m_callback_Error;
        EventList<std::error_code,size_t> m_callback_ErrorInt;
        EventList<std::error_code,uint8_t*,size_t > m_callbackErrorUInt8Int;
    };

}



