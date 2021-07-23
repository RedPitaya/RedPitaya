#include <fstream>
#include "AsioBroadcastSocket.h"

#define UNUSED(x) [&x]{}()
#define SOCKET_BUFFER_SIZE 1024

namespace  asionet_broadcast {


    CAsioBroadcastSocket::Ptr
    CAsioBroadcastSocket::Create(std::string host,  std::string port) {
        return std::make_shared<CAsioBroadcastSocket>(host, port);
    }

    CAsioBroadcastSocket::CAsioBroadcastSocket(std::string host, std::string port) :
            m_mode(ABMode::AB_NONE),
            m_sleep_time_ms(1000),
            m_host(host),
            m_port(port),
            m_Ios(),
            m_Work(m_Ios),
            m_socket(0)
    {
        m_SocketReadBuffer = new uint8_t[SOCKET_BUFFER_SIZE];
        auto func = std::bind(static_cast<size_t (asio::io_service::*)()>(&asio::io_service::run),&m_Ios);
        m_asio_th = new asio::thread(func);
    }

    CAsioBroadcastSocket::~CAsioBroadcastSocket() {
        CloseSocket();
        m_Ios.stop();
        if (m_asio_th != nullptr){
            m_asio_th->join();
            delete  m_asio_th;
        }
        delete[] m_SocketReadBuffer;
    }

    void CAsioBroadcastSocket::InitServer(ABMode mode,int sleep_time_ms) {
        m_sleep_time_ms = sleep_time_ms;
        if (mode != ABMode::AB_SERVER_MASTER && mode != ABMode::AB_SERVER_SLAVE){
            std::error_code error = std::make_error_code(std::errc::invalid_argument);
            m_callback_Error.emitEvent((int)ABEvents::AB_ERROR,error);
            return;
        }
        std::error_code error;
        m_socket = std::make_shared<asio::ip::udp::socket>(m_Ios);
        m_socket->open(asio::ip::udp::v4(), error);
        if (!error){
            m_mode = mode;
            m_socket->set_option(asio::ip::udp::socket::reuse_address(true));
            m_socket->set_option(asio::socket_base::broadcast(true));
            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), std::stoi(m_port));
            std::string buf = m_host + std::string((mode == ABMode::AB_SERVER_MASTER ? "M" : "S"));
            m_socket->async_send_to(asio::buffer(buf.c_str(),buf.size()),senderEndpoint,std::bind(&CAsioBroadcastSocket::HandlerSend, this, std::placeholders::_1 ,std::placeholders::_2 ));
        }else{
            m_callback_Error.emitEvent((int)ABEvents::AB_ERROR,error);
        }
    }

    void CAsioBroadcastSocket::InitClient(){
        std::error_code error;
        asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::any(), std::stoi(m_port));
        m_socket = std::make_shared<asio::ip::udp::socket>(m_Ios,senderEndpoint);
        m_mode = ABMode::AB_CLIENT;
        m_socket->set_option(asio::ip::udp::socket::reuse_address(true));
        m_socket->set_option(asio::socket_base::broadcast(true));
        m_socket->async_receive_from(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),senderEndpoint,std::bind(&CAsioBroadcastSocket::HandlerReceive, this, std::placeholders::_1 ,std::placeholders::_2 ));
    }

    void CAsioBroadcastSocket::CloseSocket(){
        try {
            if (m_socket && m_socket->is_open()) {
                m_socket->shutdown(asio::socket_base::shutdown_type::shutdown_both);
                m_socket->close();
            }
        }catch (...){
        }
    }

    void CAsioBroadcastSocket::HandlerReceive(const asio::error_code &error,size_t bytes_transferred) {
        if (!error){
            m_callbackErrorUInt8Int.emitEvent((int)ABEvents::AB_RECIVED_DATA,error,m_SocketReadBuffer,bytes_transferred);
            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::any(), std::stoi(m_port));
            m_socket->async_receive_from(
                    asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),senderEndpoint,std::bind(&CAsioBroadcastSocket::HandlerReceive, this,
                            std::placeholders::_1,std::placeholders::_2)
                    );
        }else{
            m_callback_Error.emitEvent((int)ABEvents::AB_ERROR,error);
            CloseSocket();
        }
    }

    void CAsioBroadcastSocket::addHandler(ABEvents _event, std::function<void(std::error_code error)> _func){
        this->m_callback_Error.addListener((int)_event,_func);
    }

    void CAsioBroadcastSocket::addHandler(ABEvents _event, std::function<void(std::error_code error,size_t)> _func){
        this->m_callback_ErrorInt.addListener((int)_event,_func);
    }

    void CAsioBroadcastSocket::addHandler(ABEvents _event, std::function<void(std::error_code error,uint8_t*,size_t)> _func){
        this->m_callbackErrorUInt8Int.addListener((int)_event,_func);
    }


    void CAsioBroadcastSocket::HandlerSend(const asio::error_code &_error, size_t _bytesTransferred){
        m_callback_ErrorInt.emitEvent((int)ABEvents::AB_SEND_DATA,_error,_bytesTransferred);
        if (_error){
            if (m_mode == ABMode::AB_SERVER_MASTER || m_mode == ABMode::AB_SERVER_SLAVE) {
                m_callback_Error.emitEvent((int)ABEvents::AB_ERROR, _error);
                InitServer(m_mode,m_sleep_time_ms);
            }
        }else{
#ifdef _WIN32
            Sleep(m_sleep_time_ms);
#else
            usleep(m_sleep_time_ms * 1000);
#endif // _WIN32

            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), std::stoi(m_port));
            std::string buf = m_host + std::string((m_mode == ABMode::AB_SERVER_MASTER ? "M" : "S"));
            m_socket->async_send_to(asio::buffer(buf.c_str(),buf.size()),senderEndpoint,std::bind(&CAsioBroadcastSocket::HandlerSend, this, std::placeholders::_1 ,std::placeholders::_2));
        }
    }

}