#ifndef STREAMING_LIB_STREAMING_NET_H
#define STREAMING_LIB_STREAMING_NET_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "data_lib/signal.hpp"
#include "data_lib/buffers_pack.h"

#include "net_lib/asio_common.h"
#include "net_lib/asio_net.h"

#define UDP_BUFFER_LIMIT 1024
#define TCP_BUFFER_LIMIT 32 * 1024
//#define ZERO_BUFFER_SIZE 1048576

#define MIN(X,Y) ((X < Y) ? X: Y)
#define MAX(X,Y) ((X > Y) ? X: Y)


namespace streaming_lib {

class CStreamingNet
{

public:

    using Ptr = std::shared_ptr<CStreamingNet>;
    typedef std::function<DataLib::CDataBuffersPack::Ptr()> getBufferFunc;
    typedef std::function<void()> unlockBufferFunc;

    static auto create(std::string &_host, std::string &_port, net_lib::EProtocol _protocol) -> Ptr;

    CStreamingNet(std::string &_host, std::string &_port, net_lib::EProtocol _protocol);
    ~CStreamingNet();


    auto run() -> void;
    auto runNonThread() -> void;
    auto stop() -> void;
    auto getProtocol() -> net_lib::EProtocol;
    auto sendBuffers(DataLib::CDataBuffersPack::Ptr pack) -> void;

    getBufferFunc getBuffer;
    unlockBufferFunc unlockBufferF;

private:

    CStreamingNet(const CStreamingNet &) = delete;
    CStreamingNet(CStreamingNet &&) = delete;
    CStreamingNet& operator=(const CStreamingNet&) =delete;
    CStreamingNet& operator=(const CStreamingNet&&) =delete;

    std::string         m_host;
    std::string         m_port;

    net_lib::EProtocol  m_protocol;
    net_lib::CAsioNet  *m_asionet;

    uint64_t            m_index_of_message;
    std::thread         m_thread;
    std::atomic_bool    m_threadRun;
    std::mutex          m_mtx;

    auto startServer() -> void;
    auto stopServer() -> void;
    auto task() -> void;
};

}

#endif
