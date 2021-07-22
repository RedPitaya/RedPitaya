#include <fstream>
#include "AsioBroadcastSocket.h"

#define UNUSED(x) [&x]{}()
#define SOCKET_BUFFER_SIZE 1024

namespace  asionet_simple {


    CAsioBroadcastSocket::Ptr
    CAsioBroadcastSocket::Create(asio::io_service &io,string host,  std::string port) {
        return std::make_shared<CAsioBroadcastSocket>(io, host, port);
    }

    CAsioBroadcastSocket::CAsioBroadcastSocket(asio::io_service &io,string host, std::string port) :
            m_mode(Mode::NONE),
            m_sleep_time_ms(1000),
            m_host(host),
            m_port(port),
            m_io_service(io),
            m_socket(0)
    {
        m_SocketReadBuffer = new uint8_t[SOCKET_BUFFER_SIZE];
    }

    CAsioBroadcastSocket::~CAsioBroadcastSocket() {
        CloseSocket();
        delete[] m_SocketReadBuffer;
    }

    void CAsioBroadcastSocket::InitServer(Mode mode,int sleep_time_ms) {
        m_sleep_time_ms = sleep_time_ms;
        if (mode != SERVER_MASTER && mode != SERVER_SLAVE){
            error_code error = std::make_error_code(std::errc::invalid_argument);
            m_callback_Error.emitEvent(Events::ERROR,error);
            return;
        }
        error_code error;
        m_socket = std::make_shared<asio::ip::udp::socket>(m_io_service);
        m_socket->open(asio::ip::udp::v4(), error);
        if (!error){
            m_mode = mode;
            m_socket->set_option(asio::ip::udp::socket::reuse_address(true));
            m_socket->set_option(asio::socket_base::broadcast(true));
            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), std::stoi(m_port));
            string buf = string((mode == SERVER_MASTER ? "M" : "S")) + m_host;
            m_socket->async_send_to(asio::buffer(buf.c_str(),buf.size()),senderEndpoint,std::bind(&CAsioBroadcastSocket::HandlerSend, this, std::placeholders::_1 ,std::placeholders::_2 ));
        }else{
            m_callback_Error.emitEvent(Events::ERROR,error);
        }
    }

    void CAsioBroadcastSocket::InitClient(){
        error_code error;
        m_socket = std::make_shared<asio::ip::udp::socket>(m_io_service);
        m_socket->open(asio::ip::udp::v4(), error);
        if (!error){
            m_mode = Mode::CLIENT;
            m_socket->set_option(asio::ip::udp::socket::reuse_address(true));
            m_socket->set_option(asio::socket_base::broadcast(true));
            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), std::stoi(m_port));
            m_socket->async_receive_from(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),senderEndpoint,std::bind(&CAsioBroadcastSocket::HandlerReceive, this, std::placeholders::_1 ,std::placeholders::_2 ));
        }else{
            m_callback_Error.emitEvent(Events::ERROR,error);
        }
    }

    void CAsioBroadcastSocket::CloseSocket(){
        if (m_socket && m_socket->is_open()){
            m_socket->shutdown(socket_base::shutdown_type::shutdown_both);
            m_socket->close();
        }
    }

    void CAsioBroadcastSocket::HandlerReceive(const asio::error_code &error,size_t bytes_transferred) {
        if (!error){
            m_callbackErrorUInt8Int.emitEvent(Events::RECIVED_DATA,error,m_SocketReadBuffer,bytes_transferred);
            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), std::stoi(m_port));
            m_socket->async_receive_from(
                    asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),senderEndpoint,std::bind(&CAsioBroadcastSocket::HandlerReceive, this,
                            std::placeholders::_1,std::placeholders::_2)
                    );
        }else{
            m_callback_Error.emitEvent(Events::ERROR,error);
            CloseSocket();
        }
    }

    void CAsioBroadcastSocket::addHandler(Events _event, std::function<void(std::error_code error)> _func){
        this->m_callback_Error.addListener(_event,_func);
    }

    void CAsioBroadcastSocket::addHandler(Events _event, std::function<void(std::error_code error,size_t)> _func){
        this->m_callback_ErrorInt.addListener(_event,_func);
    }

    void CAsioBroadcastSocket::addHandler(Events _event, std::function<void(std::error_code error,uint8_t*,size_t)> _func){
        this->m_callbackErrorUInt8Int.addListener(_event,_func);
    }


    void CAsioBroadcastSocket::HandlerSend(const asio::error_code &_error, size_t _bytesTransferred){
        m_callback_ErrorInt.emitEvent(Events::SEND_DATA,_error,_bytesTransferred);
        if (!_error){
            if (m_mode == Mode::SERVER_MASTER || m_mode == Mode::SERVER_SLAVE) {
                m_callback_Error.emitEvent(Events::ERROR, _error);
                InitServer(m_mode,m_sleep_time_ms);
            }
        }else{
            usleep(m_sleep_time_ms * 1000);
            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), std::stoi(m_port));
            string buf = string((m_mode == SERVER_MASTER ? "M" : "S")) + m_host;
            m_socket->async_send_to(asio::buffer(buf.c_str(),buf.size()),senderEndpoint,std::bind(&CAsioBroadcastSocket::HandlerSend, this, std::placeholders::_1 ,std::placeholders::_2));
        }
    }

}