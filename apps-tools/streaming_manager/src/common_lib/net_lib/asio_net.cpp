#include <fstream>
#include "asio_net.h"
#include "asio.hpp"
#include "asio_socket.h"
#include "data_lib/thread_cout.h"
#include "asio_service.h"

#define UNUSED(x) [&x]{}()

using namespace net_lib;


auto CAsioNet::create(net_lib::EMode _mode,net_lib::EProtocol _protocol,std::string _host , std::string _port) -> CAsioNet::Ptr {
    return std::make_shared<CAsioNet>(_mode,_protocol,_host,_port);
}

CAsioNet::CAsioNet(net_lib::EMode _mode,net_lib::EProtocol _protocol,std::string _host , std::string _port) :
        m_mode(_mode),
        m_protocol(_protocol),
        m_host(_host),
        m_port(_port),
        m_IsRun(false)
{
    m_server = CAsioSocket::create(m_protocol, m_host, m_port);

    m_server->connectClientNotify.connect([&](auto &host){this->clientConnectNotify(host);});
    m_server->connectServerNotify.connect([&](auto &host){this->serverConnectNotify(host);});
    m_server->disconnectClientNotify.connect([&](auto &host){this->clientDisconnectNotify(host);});
    m_server->disconnectServerNotify.connect([&](auto &host){this->serverDisconnectNotify(host);});
    m_server->errorClientNotify.connect([&](auto e){this->clientErrorNotify(e);});
    m_server->errorServerNotify.connect([&](auto e){this->serverErrorNotify(e);});
    m_server->recivedNotify.connect([&](auto e,auto* b,auto s){this->reciveNotify(e,b,s);});
    m_server->sendNotify.connect([&](auto e,auto s){this->sendNotify(e,s);});

}

CAsioNet::~CAsioNet() {    
    stop();
}

auto CAsioNet::getProtocol() -> net_lib::EProtocol{
    return  m_protocol;
}

auto CAsioNet::isConnected() -> bool{
    return  m_IsRun && m_server->isConnected();
}

auto CAsioNet::start() -> void {
    if (m_IsRun)
        return;
    if (m_mode == net_lib::EMode::M_SERVER) {
        m_server->initServer();
    }
    if (m_mode == net_lib::EMode::M_CLIENT){
        m_server->initClient();
    }
    m_IsRun = true;
}

auto CAsioNet::stop() -> void {
    sendServerStop();
    m_IsRun = false;
}

auto CAsioNet::disconnect() -> void{
    m_server->closeSocket();
    m_IsRun = false;
}

auto CAsioNet::sendServerStop() -> void{
    if (isConnected() && m_mode == net_lib::EMode::M_CLIENT){         
        m_server->sendBuffer(net_lib::createBuffer("\x00",1),1);
    }
    m_server->closeSocket();
}

auto CAsioNet::sendData(bool async,net_lib::net_buffer _buffer,size_t _size) -> bool{
    if (m_server){
        return m_server->sendBuffer(async,_buffer,_size);
    }
    return false;
}

auto CAsioNet::sendSyncData(AsioBufferNolder &_buffer) -> bool{
    if (m_server){
        return m_server->sendSyncBuffer(_buffer);
    }
    return false;
}
