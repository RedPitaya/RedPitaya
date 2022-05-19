
#include <fstream>
#include "asio.hpp"

#include "asio_socket_simple.h"
#include "asio_net_simple.h"
#include "asio_socket_simple.h"
#include "data_lib/thread_cout.h"

#define UNUSED(x) [&x]{}()

using namespace net_lib;

auto CAsioNetSimple::create(net_lib::EMode _mode,std::string _host , std::string _port) -> CAsioNetSimple::Ptr {
    return std::make_shared<CAsioNetSimple>(_mode,_host,_port);
}

CAsioNetSimple::CAsioNetSimple(net_lib::EMode _mode,std::string _host , std::string _port) :
        m_mode(_mode),
        m_host(_host),
        m_port(_port),
        m_IsRun(false)
{    
    m_server  = new CAsioSocketSimple(m_host, m_port);

    m_server->connectNotify.connect([&](auto &host){
        this->connectNotify(host);
    });
    m_server->connectTimeoutNotify.connect([&](auto err){this->connectTimeoutNotify(err);});
    m_server->disconnectNotify.connect([&](auto &host){this->disconnectNotify(host);});
    m_server->errorNotify.connect([&](auto err){this->errorNotify(err);});
    m_server->recivedNotify.connect([&](auto err,auto* b,auto s){this->recivedNotify(err,b,s);});
    m_server->sendNotify.connect([&](auto err,auto s){this->sendNotify(err,s);});
}

CAsioNetSimple::~CAsioNetSimple() {
    disconnect();   
    delete m_server;
}

auto CAsioNetSimple::isConnected() -> bool{
    return  m_IsRun && m_server->isConnected();
}

auto CAsioNetSimple::start() -> void {
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

auto CAsioNetSimple::disconnect() -> void {
    if (m_server) m_server->closeSocket();
    m_IsRun = false;
}

auto CAsioNetSimple::sendData(bool async,net_buffer _buffer,size_t _size) -> bool {
    if (m_server)
        return m_server->sendBuffer(async,_buffer,_size);
    return false;
}

