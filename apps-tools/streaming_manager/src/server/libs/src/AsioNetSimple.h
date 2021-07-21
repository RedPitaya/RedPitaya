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
#include "AsioSocketSimple.h"

using  namespace std;
using  namespace asio;

namespace  asionet_simple {

    class CAsioNetSimple {
    public:

        using Ptr = shared_ptr<CAsioNetSimple>;

        static Ptr Create(Mode _mode,string _host , string _port);
        CAsioNetSimple(Mode _mode,string _host , string _port);
        ~CAsioNetSimple();

        void start();
        void disconnect();
        void addCall_Connect(function<void(string host)> _func);
        void addCall_Disconnect(function<void(string host)> _func);
        void addCall_Error(function<void(error_code error)> _func);

        void addCall_Send(function<void(error_code error,size_t)> _func);
        void addCall_Received(function<void(error_code error,asionet_simple::buffer,size_t)> _func);

        bool sendData(bool async,asionet_simple::buffer _buffer,size_t _size);
        bool isConnected();

    private:

        CAsioNetSimple(const CAsioNetSimple &) = delete;
        CAsioNetSimple(CAsioNetSimple &&) = delete;

        Mode m_mode;
        string m_host;
        string m_port;
        asio::io_service m_Ios;
        asio::io_service::work m_Work;
        asio::thread *m_asio_th;
        bool m_IsRun;
        CAsioSocketSimple *m_server;
    };


}



