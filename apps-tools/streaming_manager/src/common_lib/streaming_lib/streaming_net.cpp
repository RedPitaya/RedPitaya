#include <chrono>
#include <iostream>
#include <fstream>
#include <time.h>
#include <functional>
#include <cstdlib>
#include <unistd.h>

#include "streaming_net.h"
#include "data_lib/thread_cout.h"
#include "data_lib/neon_asm.h"

using namespace streaming_lib;

auto CStreamingNet::create(std::string &_host, std::string &_port, net_lib::EProtocol _protocol) -> CStreamingNet::Ptr {
    return std::make_shared<CStreamingNet>(_host,_port,_protocol);
}

CStreamingNet::CStreamingNet(std::string &_host, std::string &_port, net_lib::EProtocol _protocol):
        m_host(_host),
        m_port(_port),
        m_protocol(_protocol),
        m_asionet(nullptr),
        m_index_of_message(0),
        m_thread(),
        m_mtx()
{
    getBuffer = nullptr;
    unlockBufferF = nullptr;
}

CStreamingNet::~CStreamingNet() {
    stop();
}

void CStreamingNet::startServer() {
    if (m_asionet){
        return;
    }

    m_index_of_message = 0;
//    m_SendData = 0;
    m_asionet = new net_lib::CAsioNet(net_lib::EMode::M_SERVER, m_protocol, m_host, m_port);
    m_asionet->serverConnectNotify.connect([](std::string host)
                                     {
                                        aprintf(stdout,"Connected %s\n",host.c_str());
                                     });
    m_asionet->serverDisconnectNotify.connect([](std::string host)
                                     {
                                        aprintf(stdout,"Disconnect %s\n",host.c_str());
                                     });
    m_asionet->start();
}

auto CStreamingNet::stopServer() -> void{
    if (m_asionet) {
        m_asionet->stop();
        delete m_asionet;
        m_asionet = nullptr;
    }
}

auto CStreamingNet::runNonThread() -> void{
    std::lock_guard<std::mutex> lock(m_mtx);
    startServer();
}

auto CStreamingNet::run() -> void {
    std::lock_guard<std::mutex> lock(m_mtx);
    startServer();
    try {
        m_threadRun = true;
        m_thread = std::thread(&CStreamingNet::task, this);
    }
    catch (const std::system_error &e)
    {
        aprintf(stderr,"Error: CStreamingApplication::runNonBlock() %s\n",e.what());
    }
}

auto CStreamingNet::stop() -> void{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_threadRun = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
    stopServer();
}

auto CStreamingNet::getProtocol() -> net_lib::EProtocol{
    return m_protocol;
}

uint64_t packTime = 0;

auto CStreamingNet::task() -> void{
    // uint64_t sendSize = 0;
    // uint64_t packs = 0;
    // uint64_t lockTime = 0;
    // uint64_t sendTIme = 0;
    // packTime = 0;
    // auto timeNowEnd = std::chrono::system_clock::now();
    // auto p1 = std::chrono::time_point_cast<std::chrono::microseconds>(timeNowEnd).time_since_epoch();
    while(m_threadRun){
        if (getBuffer && unlockBufferF){
            // timeNowEnd = std::chrono::system_clock::now();
            // auto a1 = std::chrono::time_point_cast<std::chrono::microseconds>(timeNowEnd).time_since_epoch();
            auto pack = getBuffer();
            // timeNowEnd = std::chrono::system_clock::now();
            // auto a2 = std::chrono::time_point_cast<std::chrono::microseconds>(timeNowEnd).time_since_epoch();
            sendBuffers(pack);
            // timeNowEnd = std::chrono::system_clock::now();
            // auto a3 = std::chrono::time_point_cast<std::chrono::microseconds>(timeNowEnd).time_since_epoch();
            // lockTime += a2.count() - a1.count();
            // sendTIme += a3.count() - a2.count();
            if (pack){
                // sendSize += pack->getBuffersSamples();
                // packs++;
                unlockBufferF();
            }
        }
    }
    // timeNowEnd = std::chrono::system_clock::now();
    // auto p2 = std::chrono::time_point_cast<std::chrono::microseconds>(timeNowEnd).time_since_epoch();
    // printf("Send samples %lld packs %lld\n",sendSize,packs);
    // printf("Total time %lld lockTime %lld sendTIme %lld packTime %lld\n",p2.count() - p1.count(),lockTime,sendTIme,packTime);
}


auto CStreamingNet::sendBuffers(DataLib::CDataBuffersPack::Ptr pack) -> void {
    if (m_asionet && pack){
        if (m_asionet->isConnected()) {
            uint32_t split_size = (getProtocol() == net_lib::EProtocol::P_TCP ? TCP_BUFFER_LIMIT : UDP_BUFFER_LIMIT);
            // auto timeNowEnd = std::chrono::system_clock::now();
            // auto a1 = std::chrono::time_point_cast<std::chrono::microseconds>(timeNowEnd).time_since_epoch();
            auto packs = net_lib::buildPack(m_index_of_message++,pack,split_size);
            // timeNowEnd = std::chrono::system_clock::now();
            // auto a2 = std::chrono::time_point_cast<std::chrono::microseconds>(timeNowEnd).time_since_epoch();
            // packTime += a2.count() - a1.count();
            for(auto &buff : packs){
                m_asionet->sendSyncData(buff);
            }
        }
    }
}


