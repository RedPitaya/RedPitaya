#ifndef NET_LIB_ASIO_NET_H
#define NET_LIB_ASIO_NET_H

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
#include "event_handlers.h"
#include "data_lib/neon_asm.h"
#include "data_lib/signal.hpp"

//#define  SOCKET_BUFFER_SIZE 65536
//#define  FIFO_BUFFER_SIZE  SOCKET_BUFFER_SIZE * 3

using  namespace std;

namespace  net_lib {

class CAsioSocket;

class CAsioNet {
public:

    using Ptr = shared_ptr<CAsioNet>;

    static auto create(net_lib::EMode _mode,net_lib::EProtocol _protocol,string _host , string _port) -> CAsioNet::Ptr;

    CAsioNet(net_lib::EMode _mode,net_lib::EProtocol _protocol,string _host , string _port);
    ~CAsioNet();

    auto start() -> void;
    auto stop()  -> void;
    auto disconnect()  -> void;

    auto sendData(bool async,net_buffer _buffer,size_t _size) -> bool;
    auto sendSyncData(AsioBufferNolder &_buffer) -> bool;
    auto getProtocol() -> net_lib::EProtocol;
    auto isConnected() -> bool;

    sigslot::signal<string&>    serverConnectNotify;
    sigslot::signal<string&>    serverDisconnectNotify;
    sigslot::signal<error_code> serverErrorNotify;

    sigslot::signal<string&>    clientConnectNotify;
    sigslot::signal<string&>    clientDisconnectNotify;
    sigslot::signal<error_code> clientErrorNotify;

    sigslot::signal<error_code,size_t>          sendNotify;
    sigslot::signal<error_code,uint8_t*,size_t> reciveNotify;


private:

    CAsioNet(const CAsioNet &) = delete;
    CAsioNet(CAsioNet &&) = delete;
    CAsioNet& operator=(const CAsioNet&) =delete;
    CAsioNet& operator=(const CAsioNet&&) =delete;

    void sendServerStop();

    net_lib::EMode m_mode;
    net_lib::EProtocol m_protocol;
    string m_host;
    string m_port;
    bool m_IsRun;
    shared_ptr<CAsioSocket> m_server;
};

}

#endif


