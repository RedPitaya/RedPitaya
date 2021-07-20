#include <fstream>
#include "AsioSocketSimple.h"
#include "AsioNetSimple.h"

#define UNUSED(x) [&x]{}()

namespace  asionet_simple {

    CAsioNetSimple::Ptr CAsioNetSimple::Create(Mode _mode,std::string _host , std::string _port) {

        return std::make_shared<CAsioNetSimple>(_mode,_host,_port);
    }

    CAsioNetSimple::CAsioNetSimple(Mode _mode,std::string _host , std::string _port) :
            m_mode(_mode),
            m_host(_host),
            m_port(_port),
            m_Ios(),
            m_Work(m_Ios),
            m_asio_th(nullptr),
            m_IsRun(false)
    {
        m_server = CAsioSocketSimple::Create(m_Ios, m_host, m_port);
        auto func = std::bind(static_cast<size_t (asio::io_service::*)()>(&asio::io_service::run), &m_Ios);
        m_asio_th = new asio::thread(func);
    }

    CAsioNetSimple::~CAsioNetSimple() {
        disconnect();
        m_Ios.stop();
        if (m_asio_th != nullptr){
            m_asio_th->join();
            delete  m_asio_th;
        }

    }

    bool CAsioNetSimple::isConnected(){
        return  m_IsRun && m_server->IsConnected();
    }

    void CAsioNetSimple::start()  {
        if (m_IsRun)
            return;
        if (m_mode == Mode::SERVER) {
            m_server->InitServer();
        }
        if (m_mode == Mode::CLIENT){
            m_server->InitClient();
        }
        m_IsRun = true;
    }

    void CAsioNetSimple::disconnect(){
        if (m_server) m_server->CloseSocket();
        m_IsRun = false;
    }

    void CAsioNetSimple::addCall_Connect(std::function<void(std::string host)> _func){
        if (m_server) m_server->addHandler(Events::CONNECT, _func);        
    }

    void CAsioNetSimple::addCall_Disconnect(std::function<void(std::string host)> _func){
        if (m_server) m_server->addHandler(Events::DISCONNECT, _func);        
    }

    void CAsioNetSimple::addCall_Error(std::function<void(std::error_code error)> _func){
        if (m_server) m_server->addHandler(Events::ERROR, _func);        
    }

    void CAsioNetSimple::addCall_Send(std::function<void(std::error_code error,size_t)> _func){
        if (m_server) m_server->addHandler(Events::SEND_DATA, _func);
    }

    void CAsioNetSimple::addCall_Received(std::function<void(std::error_code error,asionet_simple::buffer, size_t)> _func){
        if (m_server) m_server->addHandler(Events::RECIVED_DATA, _func);
    }

    bool CAsioNetSimple::sendData(bool async,asionet_simple::buffer _buffer,size_t _size){
        if (m_server) 
            return m_server->sendBuffer(async,_buffer,_size);
        return false;
    }
}