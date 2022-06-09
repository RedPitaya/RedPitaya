#ifndef BROADCAST_LIB_ASIO_BROADCAST_SOCKET_H
#define BROADCAST_LIB_ASIO_BROADCAST_SOCKET_H

#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <functional>
#include <system_error>
#include <cstdint>
#include <memory>
#include <deque>

#include "data_lib/neon_asm.h"
//#include "net_lib/event_handlers.h"
#include "data_lib/signal.hpp"

namespace  broadcast_lib {

    enum EModel{
        CLIENT        = 0,
        RP_125_14     = 1,
        RP_125_14_Z20 = 2,
        RP_122_16     = 3,
        RP_250_12     = 4,
        RP_125_4CH    = 5
    };

    enum EMode {
        AB_SERVER_MASTER = 0,
        AB_SERVER_SLAVE  = 1,
        AB_CLIENT        = 2,
        AB_NONE          = 3
    };

    enum  EEvents{
        AB_ERROR          = 0,
        AB_SEND_DATA      = 1,
        AB_RECIVED_DATA   = 2
    };

class CAsioBroadcastSocket {
public:

    typedef uint8_t* ab_buffer;

    using Ptr = std::shared_ptr<CAsioBroadcastSocket>;

    static auto create(EModel model,std::string host, std::string port) -> Ptr;
    CAsioBroadcastSocket(EModel model,std::string host, std::string port);
    ~CAsioBroadcastSocket();

    auto initServer(broadcast_lib::EMode mode,int sleep_time_ms = 1000) -> void;
    auto initClient() -> void;
    auto closeSocket() -> void;

    sigslot::signal<std::error_code> errorNotify;
    sigslot::signal<std::error_code,size_t> sendNotify;
    sigslot::signal<std::error_code,uint8_t*,size_t> receivedNotify;

private:

    CAsioBroadcastSocket(const CAsioBroadcastSocket &) = delete;
    CAsioBroadcastSocket(CAsioBroadcastSocket &&) = delete;

    auto handlerReceive(const std::error_code &error,size_t bytes_transferred) -> void;
    auto handlerAccept(const std::error_code &_error) -> void;
    auto handlerSend(const std::error_code &_error, size_t _bytesTransferred) -> void;
    auto handlerSend2(const std::error_code &_error, size_t _bytesTransferred, uint8_t *buffer) -> void;
    auto handlerReceiveFromServer(const std::error_code &ErrorCode, size_t bytes_transferred) -> void;

    broadcast_lib::EMode m_mode;
    int  m_sleep_time_ms;
    std::string m_host;
    std::string m_port;

    uint8_t *m_SocketReadBuffer;

    EModel m_model;

    // Internal implementation class
    class Impl;
    // Pointer to the internal implementation
    std::unique_ptr<Impl> m_pimpl;
};

}

#endif

