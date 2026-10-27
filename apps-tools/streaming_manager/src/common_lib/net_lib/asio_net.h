#ifndef NET_LIB_ASIO_NET_H
#define NET_LIB_ASIO_NET_H

#include <atomic>
#include <string>

#include "asio_common.h"
#include "data_lib/buffers_cached.h"
#include "data_lib/signal.hpp"

namespace net_lib {

class CAsioSocketDMA;

class CAsioNet {
   public:
    using Ptr = std::shared_ptr<CAsioNet>;

    static auto create(net_lib::EMode _mode, std::string _host, uint16_t _port, DataLib::CBuffersCached::Ptr buffers) -> CAsioNet::Ptr;

    CAsioNet(net_lib::EMode _mode, std::string _host, uint16_t _port, DataLib::CBuffersCached::Ptr buffers);
    ~CAsioNet();

    auto start() -> void;
    auto stop() -> void;
    auto cancel() -> void;
    auto disconnect() -> void;

    auto sendSyncData(DataLib::CDataBuffersPackDMA::Ptr _buffer) -> bool;
    auto isConnected() -> bool;

    sigslot::signal<std::string&> serverConnectNotify;
    sigslot::signal<std::string&> serverDisconnectNotify;
    sigslot::signal<std::error_code> serverErrorNotify;

    sigslot::signal<std::string&> clientConnectNotify;
    sigslot::signal<std::string&> clientDisconnectNotify;
    sigslot::signal<std::error_code> clientErrorNotify;

    sigslot::signal<std::error_code, size_t> sendNotify;
    sigslot::signal<std::error_code, DataLib::CDataBuffersPackDMA::Ptr> reciveNotify;

   private:
    CAsioNet(const CAsioNet&) = delete;
    CAsioNet(CAsioNet&&) = delete;
    CAsioNet& operator=(const CAsioNet&) = delete;
    CAsioNet& operator=(const CAsioNet&&) = delete;

    net_lib::EMode m_mode;
    std::string m_host;
    uint16_t m_port;
    std::atomic<bool> m_IsRun;
    std::shared_ptr<CAsioSocketDMA> m_server;
};

}  // namespace net_lib

#endif
