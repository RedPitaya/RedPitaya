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

auto CStreamingNet::task() -> void{
    while(m_threadRun){
        if (getBuffer && unlockBufferF){
            auto pack = getBuffer();
            sendBuffers(pack);
            if (pack)
                unlockBufferF();
            usleep(100);
        }
    }
}


auto CStreamingNet::sendBuffers(DataLib::CDataBuffersPack::Ptr pack) -> void {
    if (m_asionet && pack){
        if (m_asionet->isConnected()) {
            uint32_t split_size = (getProtocol() == net_lib::EProtocol::P_TCP ? TCP_BUFFER_LIMIT : UDP_BUFFER_LIMIT);
            auto packs = net_lib::buildPack(m_index_of_message++,pack,split_size);            
            for(auto &buff : packs){
                m_asionet->sendSyncData(buff);
            }
        }
    }
}


