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

namespace  asionet_simple {

    class CAsioNetSimple {
    public:

        using Ptr = std::shared_ptr<CAsioNetSimple>;

        static Ptr Create(CAsioSocketSimple::ASMode _mode,std::string _host , std::string _port);
        CAsioNetSimple(CAsioSocketSimple::ASMode _mode,std::string _host , std::string _port);
        ~CAsioNetSimple();

        void start();
        void disconnect();
        void addCall_Connect(std::function<void(std::string host)> _func);
        void addCall_Disconnect(std::function<void(std::string host)> _func);
        void addCall_Error(std::function<void(std::error_code error)> _func);
        void addCall_TimeoutError(std::function<void(std::error_code error)> _func);

        void addCall_Send(std::function<void(std::error_code error,size_t)> _func);
        void addCall_Received(std::function<void(std::error_code error,CAsioSocketSimple::as_buffer,size_t)> _func);

        bool sendData(bool async,CAsioSocketSimple::as_buffer _buffer,size_t _size);
        bool isConnected();

    private:

        CAsioNetSimple(const CAsioNetSimple &) = delete;
        CAsioNetSimple(CAsioNetSimple &&) = delete;

        CAsioSocketSimple::ASMode m_mode;
        std::string m_host;
        std::string m_port;
        asio::io_service m_Ios;
        asio::io_service::work m_Work;
        asio::thread *m_asio_th;
        bool m_IsRun;
        CAsioSocketSimple *m_server;
    };


}



