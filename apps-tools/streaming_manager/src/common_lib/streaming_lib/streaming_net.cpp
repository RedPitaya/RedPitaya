#include <time.h>
#include <unistd.h>
#include <cstdlib>
#include <functional>

#include "data_lib/network_header.h"
#include "logger_lib/file_logger.h"
#include "streaming_net.h"

using namespace streaming_lib;

auto CStreamingNet::create(std::string& _host, uint16_t _port) -> CStreamingNet::Ptr {
    return std::make_shared<CStreamingNet>(_host, _port);
}

CStreamingNet::CStreamingNet(std::string& _host, uint16_t _port)
    : m_host(_host), m_port(_port), m_asionet(nullptr), m_index_of_message(0), m_thread(), m_mtx() {
    getBuffer = nullptr;
    unlockBufferF = nullptr;
}

CStreamingNet::~CStreamingNet() {
    stop();
}

void CStreamingNet::startServer() {
    if (m_asionet) {
        return;
    }

    m_index_of_message = 0;
    m_asionet = new net_lib::CAsioNet(net_lib::EMode::M_SERVER, m_host, m_port, nullptr);
    m_asionet->serverConnectNotify.connect([](std::string host) { aprintf(stdout, "Connected %s\n", host.c_str()); });
    m_asionet->serverDisconnectNotify.connect([](std::string host) { aprintf(stdout, "Disconnect %s\n", host.c_str()); });
    m_asionet->start();
}

auto CStreamingNet::stopServer() -> void {
    if (m_asionet) {
        m_asionet->stop();
        delete m_asionet;
        m_asionet = nullptr;
    }
}

auto CStreamingNet::runNonThread() -> void {
    std::lock_guard lock(m_mtx);
    startServer();
}

auto CStreamingNet::run() -> void {
    std::lock_guard lock(m_mtx);
    startServer();
    try {
        m_threadRun = true;
        m_thread = std::thread(&CStreamingNet::task, this);
    } catch (const std::system_error& e) {
        aprintf(stderr, "Error: CStreamingApplication::runNonBlock() %s\n", e.what());
    }
}

auto CStreamingNet::stop() -> void {
    std::lock_guard lock(m_mtx);
    m_threadRun = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
    stopServer();
}

uint64_t packTime = 0;

auto CStreamingNet::task() -> void {
    while (m_threadRun) {
        if (getBuffer && unlockBufferF) {
            auto pack = getBuffer();
            sendBuffers(pack);
            if (pack) {
                unlockBufferF();
            }
        }
    }
}

auto CStreamingNet::sendBuffers(DataLib::CDataBuffersPackDMA::Ptr pack) -> void {
    if (m_asionet && pack) {
        if (m_asionet->isConnected()) {
            for (auto i = (int)DataLib::EDataBuffersPackChannel::CH1; i <= (int)DataLib::EDataBuffersPackChannel::CH4; i++) {
                auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)i);
                if (buff != NULL) {
                    DataLib::setHeaderADC(buff, m_index_of_message);
                }
            }
            m_asionet->sendSyncData(pack);
            m_index_of_message++;
        }
    }
}
