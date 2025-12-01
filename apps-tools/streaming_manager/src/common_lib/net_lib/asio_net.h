#ifndef NET_LIB_ASIO_NET_H
#define NET_LIB_ASIO_NET_H

#include <string>

#include "asio_common.h"
#include "data_lib/buffers_cached.h"
#include "data_lib/signal.hpp"

using namespace std;

namespace net_lib {

class CAsioSocketDMA;

class CAsioNet {
   public:
    using Ptr = shared_ptr<CAsioNet>;

    static auto create(net_lib::EMode _mode, string _host, uint16_t _port, DataLib::CBuffersCached::Ptr buffers) -> CAsioNet::Ptr;

    CAsioNet(net_lib::EMode _mode, string _host, uint16_t _port, DataLib::CBuffersCached::Ptr buffers);
    ~CAsioNet();

    auto start() -> void;
    auto stop() -> void;
    auto cancel() -> void;
    auto disconnect() -> void;

    auto sendSyncData(DataLib::CDataBuffersPackDMA::Ptr _buffer) -> bool;
    auto isConnected() -> bool;

    sigslot::signal<string&> serverConnectNotify;
    sigslot::signal<string&> serverDisconnectNotify;
    sigslot::signal<error_code> serverErrorNotify;

    sigslot::signal<string&> clientConnectNotify;
    sigslot::signal<string&> clientDisconnectNotify;
    sigslot::signal<error_code> clientErrorNotify;

    sigslot::signal<error_code, size_t> sendNotify;
    sigslot::signal<error_code, DataLib::CDataBuffersPackDMA::Ptr> reciveNotify;

   private:
    CAsioNet(const CAsioNet&) = delete;
    CAsioNet(CAsioNet&&) = delete;
    CAsioNet& operator=(const CAsioNet&) = delete;
    CAsioNet& operator=(const CAsioNet&&) = delete;

    net_lib::EMode m_mode;
    string m_host;
    uint16_t m_port;
    bool m_IsRun;
    shared_ptr<CAsioSocketDMA> m_server;
};

}  // namespace net_lib

#endif
