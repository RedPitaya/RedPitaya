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

namespace  asionet_broadcast {

    enum Mode {
        SERVER_MASTER,
        SERVER_SLAVE,
        CLIENT,
        NONE
    };

    enum Events{
        ERROR,
        SEND_DATA,
        RECIVED_DATA};

    typedef uint8_t* buffer;

    class CAsioBroadcastSocket {
    public:
       

        using Ptr = shared_ptr<CAsioBroadcastSocket>;

        static Ptr Create(string host, string port);
        CAsioBroadcastSocket(string host, string port);
        ~CAsioBroadcastSocket();

        void InitServer(Mode mode,int sleep_time_ms = 1000);
        void InitClient();
        void CloseSocket();
        void addHandler(Events _event, std::function<void(error_code error)> _func);
        void addHandler(Events _event, std::function<void(error_code error,size_t)> _func);
        void addHandler(Events _event, std::function<void(error_code error,uint8_t*,size_t)> _func);

    private:

        CAsioBroadcastSocket(const CAsioBroadcastSocket &) = delete;
        CAsioBroadcastSocket(CAsioBroadcastSocket &&) = delete;

        void HandlerReceive(const asio::error_code &error,size_t bytes_transferred);
        void HandlerAccept(const asio::error_code &_error);
        void HandlerConnect(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator);
        void HandlerSend(const asio::error_code &_error, size_t _bytesTransferred);
        void HandlerSend2(const asio::error_code &_error, size_t _bytesTransferred, uint8_t *buffer);
        void HandlerReceiveFromServer(const asio::error_code &ErrorCode, size_t bytes_transferred);

        Mode m_mode;
        int  m_sleep_time_ms;
        string m_host;
        string m_port;
        asio::io_service m_Ios;
        asio::io_service::work m_Work;
        
        shared_ptr<asio::ip::udp::socket> m_socket;

        uint8_t *m_SocketReadBuffer;

        EventList<std::error_code> m_callback_Error;
        EventList<std::error_code,size_t> m_callback_ErrorInt;
        EventList<std::error_code,uint8_t*,size_t > m_callbackErrorUInt8Int;
        asio::thread *m_asio_th;
    };
}



