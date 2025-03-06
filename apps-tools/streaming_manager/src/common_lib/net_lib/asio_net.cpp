#include "asio_net.h"
#include "asio_socket_dma.h"

using namespace net_lib;

auto CAsioNet::create(net_lib::EMode _mode, std::string _host, uint16_t _port, DataLib::CBuffersCached::Ptr buffers) -> CAsioNet::Ptr {
    return std::make_shared<CAsioNet>(_mode, _host, _port, buffers);
}

CAsioNet::CAsioNet(net_lib::EMode _mode, std::string _host, uint16_t _port, DataLib::CBuffersCached::Ptr buffers)
    : m_mode(_mode), m_host(_host), m_port(_port), m_IsRun(false) {
    m_server = CAsioSocketDMA::create(m_host, m_port, buffers);

    m_server->connectClientNotify.connect([&](auto& host) { this->clientConnectNotify(host); });
    m_server->connectServerNotify.connect([&](auto& host) { this->serverConnectNotify(host); });
    m_server->disconnectClientNotify.connect([&](auto& host) { this->clientDisconnectNotify(host); });
    m_server->disconnectServerNotify.connect([&](auto& host) { this->serverDisconnectNotify(host); });
    m_server->errorClientNotify.connect([&](auto e) { this->clientErrorNotify(e); });
    m_server->errorServerNotify.connect([&](auto e) { this->serverErrorNotify(e); });
    m_server->recivedNotify.connect([&](auto e, auto b) { this->reciveNotify(e, b); });
    m_server->sendNotify.connect([&](auto e, auto s) { this->sendNotify(e, s); });
}

CAsioNet::~CAsioNet() {
    stop();
}

auto CAsioNet::isConnected() -> bool {
    return m_IsRun && m_server->isConnected();
}

auto CAsioNet::start() -> void {
    if (m_IsRun)
        return;
    if (m_mode == net_lib::EMode::M_SERVER) {
        m_server->initServer();
    }
    if (m_mode == net_lib::EMode::M_CLIENT) {
        m_server->initClient();
    }
    m_IsRun = true;
}

auto CAsioNet::stop() -> void {
    // sendServerStop();
    m_server->stopReceive();
    m_IsRun = false;
}

auto CAsioNet::cancel() -> void {
    m_server->cancelSocket();
}

auto CAsioNet::disconnect() -> void {
    m_server->closeSocket();
    m_IsRun = false;
}

auto CAsioNet::sendSyncData(DataLib::CDataBuffersPackDMA::Ptr _buffer) -> bool {
    if (m_server) {
        return m_server->sendSyncBuffer(_buffer);
    }
    return false;
}
