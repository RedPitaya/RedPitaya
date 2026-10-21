#ifndef NET_LIB_ASIO_NET_SIMPLE_H
#define NET_LIB_ASIO_NET_SIMPLE_H

#include <atomic>
#include "data_lib/signal.hpp"
#include "net_lib/asio_common.h"

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

    sigslot::signal<std::string&> connectNotify;
    sigslot::signal<std::string&> disconnectNotify;

    sigslot::signal<std::error_code> errorNotify;
    sigslot::signal<std::error_code> connectTimeoutNotify;

    sigslot::signal<std::error_code, size_t> sendNotify;
    sigslot::signal<std::error_code, uint8_t*, size_t> recivedNotify;

    auto sendData(bool async, net_buffer _buffer, size_t _size) -> bool;
    auto isConnected() -> bool;
    auto runningInIoThread() -> bool;
    // Nothing is reported to the owner after this, for an object that outlives it.
    auto disconnectSignals() -> void;

   private:
    CAsioNetSimple(const CAsioNetSimple&) = delete;
    CAsioNetSimple(CAsioNetSimple&&) = delete;
    CAsioNetSimple& operator=(const CAsioNetSimple&) = delete;
    CAsioNetSimple& operator=(const CAsioNetSimple&&) = delete;

    net_lib::EMode m_mode;
    std::string m_host;
    uint16_t m_port;
    std::atomic<bool> m_IsRun;
    CAsioSocketSimple* m_server;
};

}  // namespace net_lib

#endif
