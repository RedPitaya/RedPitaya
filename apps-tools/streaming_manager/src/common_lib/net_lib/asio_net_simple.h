#ifndef NET_LIB_ASIO_NET_SIMPLE_H
#define NET_LIB_ASIO_NET_SIMPLE_H

#include "data_lib/signal.hpp"
#include "net_lib/asio_common.h"

using namespace std;

namespace net_lib {

class CAsioSocketSimple;

class CAsioNetSimple {
   public:
    using Ptr = std::shared_ptr<CAsioNetSimple>;

    static Ptr create(net_lib::EMode _mode, std::string _host, uint16_t _port);

    CAsioNetSimple(net_lib::EMode _mode, std::string _host, uint16_t _port);
    ~CAsioNetSimple();

    auto start() -> void;
    auto disconnect() -> void;

    sigslot::signal<string&> connectNotify;
    sigslot::signal<string&> disconnectNotify;

    sigslot::signal<error_code> errorNotify;
    sigslot::signal<error_code> connectTimeoutNotify;

    sigslot::signal<error_code, size_t> sendNotify;
    sigslot::signal<error_code, uint8_t*, size_t> recivedNotify;

    auto sendData(bool async, net_buffer _buffer, size_t _size) -> bool;
    auto isConnected() -> bool;

   private:
    CAsioNetSimple(const CAsioNetSimple&) = delete;
    CAsioNetSimple(CAsioNetSimple&&) = delete;
    CAsioNetSimple& operator=(const CAsioNetSimple&) = delete;
    CAsioNetSimple& operator=(const CAsioNetSimple&&) = delete;

    net_lib::EMode m_mode;
    std::string m_host;
    uint16_t m_port;
    bool m_IsRun;
    CAsioSocketSimple* m_server;
};

}  // namespace net_lib

#endif
