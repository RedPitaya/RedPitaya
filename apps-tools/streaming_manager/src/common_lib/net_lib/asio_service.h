#ifndef NET_LIB_ASIO_SERVICE_H
#define NET_LIB_ASIO_SERVICE_H

#include "asio.hpp"

namespace net_lib {

class CAsioService {
   public:
    auto getIO() -> asio::io_context&;
    // Stops the loop and waits for its thread, unless called from that thread (then it is detached).
    auto stop() -> void;
    auto runningInThisThread() -> bool;

    CAsioService();
    ~CAsioService();

   private:
    CAsioService(const CAsioService&) = delete;
    CAsioService(CAsioService&&) = delete;
    CAsioService& operator=(const CAsioService&) = delete;
    CAsioService& operator=(const CAsioService&&) = delete;

    asio::io_context m_Ios;
    // asio::io_service::work m_Work;
    std::thread* m_asio_th;
};

}  // namespace net_lib

#endif
