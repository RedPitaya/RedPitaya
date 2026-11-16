#include <time.h>
#include <unistd.h>
#include <chrono>
#include <cinttypes>
#include <cstdlib>
#include <functional>

#include "data_lib/network_header.h"
#include "logger_lib/file_logger.h"
#include "streaming_net.h"

using namespace streaming_lib;

auto CStreamingNet::create(std::string &_host, uint16_t _port) -> CStreamingNet::Ptr
{
	return std::make_shared<CStreamingNet>(_host, _port);
}

CStreamingNet::CStreamingNet(std::string &_host, uint16_t _port)
	: m_host(_host)
	, m_port(_port)
	, m_asionet(nullptr)
	, m_index_of_message(0)
	, m_thread()
	, m_mtx()
{
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
    m_asionet->serverDisconnectNotify.connect([this](std::string host) {
        aprintf(stdout, "Disconnect %s\n", host.c_str());
        if (clientGone)
            clientGone();
    });
    m_asionet->setZeroCopy(m_zeroCopy, m_zeroCopyPending);
    m_asionet->start();
}

auto CStreamingNet::setZeroCopy(bool enable, size_t maxPending) -> void {
    m_zeroCopy = enable;
    m_zeroCopyPending = maxPending;
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
    // The sender may be blocked on a client that stopped reading: closing the socket releases it.
    if (m_asionet) {
        m_asionet->disconnect();
    }
    if (m_thread.joinable()) {
        m_thread.join();
    }
    stopServer();
}

uint64_t packTime = 0;

auto CStreamingNet::task() -> void {
    const int cpu = net_lib::pinSenderThread();
    if (cpu >= 0)
        aprintf(stdout, "[Streaming] Sender thread on CPU %d\n", cpu);
    while (m_threadRun) {
        if (getBuffer && unlockBufferF) {
            auto pack = getBuffer();
            if (pack)
                sendBuffers(pack);
            else if (m_asionet)
                // the writer may wait for a pack the kernel is done with: no later send frees it
                m_asionet->releaseSentZeroCopy();
        }
    }
    if (m_asionet) {
        // the packs still read by the kernel go back before the ring does
        m_asionet->flushZeroCopy(std::chrono::milliseconds(500));
        uint64_t sends = 0, copied = 0;
        if (m_asionet->zeroCopyStats(sends, copied) && sends)
            aprintf(stdout, "[Streaming] Zero copy send: %" PRIu64 " sends, %" PRIu64 " copied by the kernel\n", sends, copied);
    }
}

// The pack goes back to the ring through unlockBufferF: at once without a client or without zero
// copy, later with it.
auto CStreamingNet::sendBuffers(DataLib::CDataBuffersPackDMA::Ptr pack) -> void {
    if (!pack)
        return;
    if (m_asionet && m_asionet->isConnected()) {
        for (auto i = (int)DataLib::EDataBuffersPackChannel::CH1; i <= (int)DataLib::EDataBuffersPackChannel::CH4; i++) {
            auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)i);
            if (buff != NULL) {
                DataLib::setHeaderADC(buff, m_index_of_message);
            }
        }
        m_asionet->sendSyncData(pack, unlockBufferF);
        m_index_of_message++;
    } else if (unlockBufferF) {
        unlockBufferF();
    }
}
