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


using  namespace std;
using  namespace asio;

namespace  asionet_simple {

    enum Mode {
        SERVER,
        CLIENT,
        NONE
    };

    enum Events{
        CONNECT,
        DISCONNECT,
        ERROR,
        SEND_DATA,
        RECIVED_DATA};

    typedef uint8_t* buffer;

    class CAsioSocketSimple {
    public:
       

        using Ptr = shared_ptr<CAsioSocketSimple>;

        static Ptr Create(asio::io_service &io, string host, string port);
        CAsioSocketSimple(asio::io_service &io, string host, string port);
        ~CAsioSocketSimple();

        void InitServer();
        void InitClient();
        void CloseSocket();
        bool IsConnected();
        // Buffer will be deleted after send in async mode. In sync mode not deleted
        bool sendBuffer(bool async, buffer _buffer, size_t _size);
        void addHandler(Events _event, std::function<void(string host)> _func);
        void addHandler(Events _event, std::function<void(error_code error)> _func);
        void addHandler(Events _event, std::function<void(error_code error,size_t)> _func);
        void addHandler(Events _event, std::function<void(error_code error,uint8_t*,size_t)> _func);

    private:

        CAsioSocketSimple(const CAsioSocketSimple &) = delete;
        CAsioSocketSimple(CAsioSocketSimple &&) = delete;

        void HandlerReceive(const asio::error_code &error,size_t bytes_transferred);
        void HandlerAccept(const asio::error_code &_error);
        void HandlerConnect(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator);
        void HandlerSend(const asio::error_code &_error, size_t _bytesTransferred);
        void HandlerSend2(const asio::error_code &_error, size_t _bytesTransferred, uint8_t *buffer);
        void HandlerReceiveFromServer(const asio::error_code &ErrorCode, size_t bytes_transferred);

        Mode m_mode;
        string m_host;
        string m_port;
        io_service &m_io_service;

        shared_ptr<asio::ip::tcp::socket> m_tcp_socket;
        shared_ptr<asio::ip::tcp::acceptor> m_tcp_acceptor;
        asio::ip::tcp::endpoint m_tcp_endpoint;

        uint8_t *m_SocketReadBuffer;
        bool m_is_tcp_connected;
        
        EventList<std::string> m_callback_Str;
        EventList<std::error_code> m_callback_Error;
        EventList<std::error_code,size_t> m_callback_ErrorInt;
        EventList<std::error_code,uint8_t*,size_t > m_callbackErrorUInt8Int;
    };
}



