#include <fstream>
#include "AsioSocketSimple.h"

#define UNUSED(x) [&x]{}()
#define SOCKET_BUFFER_SIZE 1024
#define CONNECT_TIMEOUT 5

namespace  asionet_simple {


    CAsioSocketSimple::Ptr
    CAsioSocketSimple::Create(asio::io_service &io,  std::string host, std::string port) {
        return std::make_shared<CAsioSocketSimple>(io, host, port);
    }

    CAsioSocketSimple::CAsioSocketSimple(asio::io_service &io, std::string host, std::string port) :
            m_mode(ASMode::AS_NONE),
            m_host(host),
            m_port(port),
            m_io_service(io),
            m_tcp_socket(0),
            m_tcp_acceptor(0),
            m_timoutTimer(m_io_service),
            m_is_tcp_connected(false),
            m_disableRestartServer(true)
    {
        m_SocketReadBuffer = new uint8_t[SOCKET_BUFFER_SIZE];
    }

    CAsioSocketSimple::~CAsioSocketSimple() {
        m_disableRestartServer = true;
        CloseSocket();
        delete[] m_SocketReadBuffer;
    }

    void CAsioSocketSimple::InitServer() {
        m_is_tcp_connected = false;
        m_disableRestartServer = false;
        m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(m_io_service);
        m_tcp_acceptor = std::make_shared<asio::ip::tcp::acceptor>(m_io_service);
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), std::stoi(m_port));
        m_tcp_acceptor->open(endpoint.protocol());
        m_tcp_acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
        m_tcp_acceptor->bind(endpoint);
        m_tcp_acceptor->listen();
        m_tcp_acceptor->async_accept(*m_tcp_socket,m_tcp_endpoint, std::bind(&CAsioSocketSimple::HandlerAccept, this, std::placeholders::_1));    
        m_mode = ASMode::AS_SERVER;
    }

    void CAsioSocketSimple::InitClient(){
        m_is_tcp_connected = false;
        m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(m_io_service);
        asio::ip::tcp::resolver resolver(m_io_service);
        asio::ip::tcp::resolver::query query(m_host, m_port);
        asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
        m_tcp_endpoint = *iter;
        m_tcp_socket->async_connect(m_tcp_endpoint, std::bind(&CAsioSocketSimple::HandlerConnect, this, std::placeholders::_1 , iter));
        m_mode = ASMode::AS_CLIENT;
        m_timoutTimer.expires_after(std::chrono::seconds(CONNECT_TIMEOUT));
        m_timoutTimer.async_wait([this](std::error_code er){
            if (er.value() != asio::error::operation_aborted) {
                try {
                    if (m_tcp_socket && m_tcp_socket->is_open()) {
                        m_tcp_socket->cancel();
                        m_tcp_socket->shutdown(asio::socket_base::shutdown_type::shutdown_both);
                        m_tcp_socket->close();
                    }
                }catch (...){

                }
                this->m_timoutTimer.cancel();
                m_callback_Error.emitEvent((int) ASEvents::AS_CONNECT_TIMEOUT, er);
            }
        });
    }

    void CAsioSocketSimple::HandlerConnect(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator)
    {
		try {
            this->m_timoutTimer.cancel();
			if (!_error)
			{
				m_is_tcp_connected = true;
				m_callback_Str.emitEvent((int)ASEvents::AS_CONNECT, m_tcp_endpoint.address().to_string());
				m_tcp_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
					std::bind(&CAsioSocketSimple::HandlerReceive, this,
						std::placeholders::_1, std::placeholders::_2));
			}
			else if (endpoint_iterator != asio::ip::tcp::resolver::iterator()) {
				m_tcp_socket->close();
				m_tcp_endpoint = *endpoint_iterator;
				m_tcp_socket->async_connect(m_tcp_endpoint, std::bind(&CAsioSocketSimple::HandlerConnect, this,
					std::placeholders::_1, ++endpoint_iterator));
			}
			else
			{
				m_is_tcp_connected = false;
				m_callback_Error.emitEvent((int)ASEvents::AS_ERROR, _error);
			}
		}
		catch (...) {
			m_is_tcp_connected = false;
			m_callback_Error.emitEvent((int)ASEvents::AS_ERROR, _error);
		}
    }

    void CAsioSocketSimple::CloseSocket(){

        try {
            if (m_is_tcp_connected) {
                m_is_tcp_connected = false;
                m_callback_Str.emitEvent((int)ASEvents::AS_DISCONNECT, m_tcp_endpoint.address().to_string());
                if (m_tcp_socket && m_tcp_socket->is_open()) {
                    m_tcp_socket->shutdown(asio::socket_base::shutdown_type::shutdown_both);
                    m_tcp_socket->close();
                }
            }
        }catch (...){

        }
    }

    void CAsioSocketSimple::HandlerReceive(const asio::error_code &error,size_t bytes_transferred) {
        if (!error){
            m_callbackErrorUInt8Int.emitEvent((int)ASEvents::AS_RECIVED_DATA,error,m_SocketReadBuffer,bytes_transferred);
            m_tcp_socket->async_receive(
                    asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
                    std::bind(&CAsioSocketSimple::HandlerReceive, this,
                            std::placeholders::_1,std::placeholders::_2));
        }else{
            m_callback_Error.emitEvent((int)ASEvents::AS_ERROR,error);
            CloseSocket();
            if (m_mode == ASMode::AS_SERVER  && !m_disableRestartServer) {
                InitServer();
            }
        }
    }

    bool CAsioSocketSimple::IsConnected(){
        return m_is_tcp_connected;
    }

    void CAsioSocketSimple::HandlerAccept(const asio::error_code &_error)
    {
        if (!_error)
        {
            m_is_tcp_connected = true;
            m_callback_Str.emitEvent((int)ASEvents::AS_CONNECT,m_tcp_endpoint.address().to_string());
            m_tcp_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
                                        std::bind(&CAsioSocketSimple::HandlerReceive, this,
                                                  std::placeholders::_1, std::placeholders::_2));
        }
        else if (_error.value() != 1) // Already open connection
        {
            m_is_tcp_connected = false;
            m_callback_Error.emitEvent((int)ASEvents::AS_ERROR,_error);
            if (m_tcp_socket->is_open()) {
                m_tcp_acceptor->async_accept(*m_tcp_socket, m_tcp_endpoint,
                                             std::bind(&CAsioSocketSimple::HandlerAccept, this, std::placeholders::_1));
            }
        }
    }

    
    void CAsioSocketSimple::addHandler(ASEvents _event, std::function<void(std::string host)> _func){
        this->m_callback_Str.addListener((int)_event,_func);
    }

    void CAsioSocketSimple::addHandler(ASEvents _event, std::function<void(std::error_code error)> _func){
        this->m_callback_Error.addListener((int)_event,_func);
    }

    void CAsioSocketSimple::addHandler(ASEvents _event, std::function<void(std::error_code error,size_t)> _func){
        this->m_callback_ErrorInt.addListener((int)_event,_func);
    }

    void CAsioSocketSimple::addHandler(ASEvents _event, std::function<void(std::error_code error,uint8_t*,size_t)> _func){
        this->m_callbackErrorUInt8Int.addListener((int)_event,_func);
    }

    bool CAsioSocketSimple::sendBuffer(bool async, as_buffer _buffer, size_t _size){
        asio::error_code _error;
        if (m_is_tcp_connected  && m_tcp_socket->is_open()) {
            if (!async) {
                size_t offset = 0;
                while(offset < _size){
                    size_t send_size = m_tcp_socket->send(asio::buffer((uint8_t*)(&(*_buffer)+offset), _size-offset), 0, _error);
                    if (_error.value() != 0){
                        return false;
                    }
                    offset += send_size;
                }
                this->HandlerSend(_error,offset);
            }else {
                m_tcp_socket->async_send(asio::buffer(_buffer,_size),
                                            std::bind(&CAsioSocketSimple::HandlerSend2, this, std::placeholders::_1 ,std::placeholders::_2,_buffer ));
            }
            return  true;
        }
        return false;
    }

    void CAsioSocketSimple::HandlerSend2(const asio::error_code &_error, size_t _bytesTransferred, uint8_t *buffer){
        HandlerSend(_error,_bytesTransferred);
        delete[] buffer;
    }

    void CAsioSocketSimple::HandlerSend(const asio::error_code &_error, size_t _bytesTransferred){
        m_callback_ErrorInt.emitEvent((int)ASEvents::AS_SEND_DATA,_error,_bytesTransferred);
        if (!_error){
            // MODIFY LATER

        } else if ((_error == asio::error::eof) || (_error == asio::error::connection_reset) ||
                _error == asio::error::broken_pipe) {
            if (m_mode == ASMode::AS_SERVER && !m_disableRestartServer) {
                if (m_is_tcp_connected)
                    m_callback_Str.emitEvent((int)ASEvents::AS_DISCONNECT, m_tcp_endpoint.address().to_string());
                InitServer();
            }

            if (m_mode == ASMode::AS_CLIENT) {
                if (m_is_tcp_connected)
                    m_callback_Str.emitEvent((int)ASEvents::AS_DISCONNECT, m_tcp_endpoint.address().to_string());
            }
            m_is_tcp_connected = false;

        }else{
            m_is_tcp_connected = false;
            // MODIFY LATER
        }
    }

}