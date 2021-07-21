#include <fstream>
#include "AsioSocketSimple.h"

#define UNUSED(x) [&x]{}()
#define SOCKET_BUFFER_SIZE 1024

namespace  asionet_simple {


    CAsioSocketSimple::Ptr
    CAsioSocketSimple::Create(asio::io_service &io,  std::string host, std::string port) {
        return std::make_shared<CAsioSocketSimple>(io, host, port);
    }

    CAsioSocketSimple::CAsioSocketSimple(asio::io_service &io, std::string host, std::string port) :
            m_mode(Mode::NONE),
            m_host(host),
            m_port(port),
            m_io_service(io),
            m_tcp_socket(0),
            m_tcp_acceptor(0),
            m_is_tcp_connected(false)
    {
        m_SocketReadBuffer = new uint8_t[SOCKET_BUFFER_SIZE];
    }

    CAsioSocketSimple::~CAsioSocketSimple() {
        CloseSocket();
        delete[] m_SocketReadBuffer;
    }

    void CAsioSocketSimple::InitServer() {
        m_is_tcp_connected = false;
        m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(m_io_service);
        m_tcp_acceptor = std::make_shared<asio::ip::tcp::acceptor>(m_io_service);
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), std::stoi(m_port));
        m_tcp_acceptor->open(endpoint.protocol());
        m_tcp_acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
        m_tcp_acceptor->bind(endpoint);
        m_tcp_acceptor->listen();
        m_tcp_acceptor->async_accept(*m_tcp_socket,m_tcp_endpoint, std::bind(&CAsioSocketSimple::HandlerAccept, this, std::placeholders::_1));    
        m_mode = Mode::SERVER;
    }

    void CAsioSocketSimple::InitClient(){
        m_is_tcp_connected = false;
        m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(m_io_service);
        asio::ip::tcp::resolver resolver(m_io_service);
        asio::ip::tcp::resolver::query query(m_host, m_port);
        asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
        m_tcp_endpoint = *iter;
        m_tcp_socket->async_connect(m_tcp_endpoint, std::bind(&CAsioSocketSimple::HandlerConnect, this, std::placeholders::_1 , iter));
        m_mode = Mode::CLIENT;
    }

    void CAsioSocketSimple::HandlerConnect(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator)
    {
		try {
			if (!_error)
			{
				m_is_tcp_connected = true;
				m_callback_Str.emitEvent(Events::CONNECT, m_tcp_endpoint.address().to_string());
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
				m_callback_Error.emitEvent(Events::ERROR, _error);
			}
		}
		catch (...) {
			m_is_tcp_connected = false;
			m_callback_Error.emitEvent(Events::ERROR, _error);
		}
    }

    void CAsioSocketSimple::CloseSocket(){

        if (m_is_tcp_connected){
            m_is_tcp_connected = false;
            m_callback_Str.emitEvent(Events::DISCONNECT, m_tcp_endpoint.address().to_string());
            if (m_tcp_socket && m_tcp_socket->is_open()){
                m_tcp_socket->shutdown(socket_base::shutdown_type::shutdown_both);
                m_tcp_socket->close();
            }
        }
    }

    void CAsioSocketSimple::HandlerReceive(const asio::error_code &error,size_t bytes_transferred) {
        if (!error){
            m_callbackErrorUInt8Int.emitEvent(Events::RECIVED_DATA,error,m_SocketReadBuffer,bytes_transferred);
            m_tcp_socket->async_receive(
                    asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
                    std::bind(&CAsioSocketSimple::HandlerReceive, this,
                            std::placeholders::_1,std::placeholders::_2));
        }else{
            m_callback_Error.emitEvent(Events::ERROR,error);
            CloseSocket();
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
            m_callback_Str.emitEvent(Events::CONNECT,m_tcp_endpoint.address().to_string());
            m_tcp_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
                                        std::bind(&CAsioSocketSimple::HandlerReceive, this,
                                                  std::placeholders::_1, std::placeholders::_2));
        }
        else if (_error.value() != 1) // Already open connection
        {
            m_is_tcp_connected = false;
            m_callback_Error.emitEvent(Events::ERROR,_error);
            if (m_tcp_socket->is_open()) {
                m_tcp_acceptor->async_accept(*m_tcp_socket, m_tcp_endpoint,
                                             std::bind(&CAsioSocketSimple::HandlerAccept, this, std::placeholders::_1));
            }
        }
    }

    
    void CAsioSocketSimple::addHandler(Events _event, std::function<void(std::string host)> _func){
        this->m_callback_Str.addListener(_event,_func);
    }

    void CAsioSocketSimple::addHandler(Events _event, std::function<void(std::error_code error)> _func){
        this->m_callback_Error.addListener(_event,_func);
    }

    void CAsioSocketSimple::addHandler(Events _event, std::function<void(std::error_code error,size_t)> _func){
        this->m_callback_ErrorInt.addListener(_event,_func);
    }

    void CAsioSocketSimple::addHandler(Events _event, std::function<void(std::error_code error,uint8_t*,size_t)> _func){
        this->m_callbackErrorUInt8Int.addListener(_event,_func);
    }

    bool CAsioSocketSimple::sendBuffer(bool async, buffer _buffer, size_t _size){
        asio::error_code _error;
        if (m_is_tcp_connected  && m_tcp_socket->is_open()) {
            if (!async) {
                m_tcp_socket->send(asio::buffer(_buffer, _size), 0, _error);
                this->HandlerSend(_error,_size);
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
        m_callback_ErrorInt.emitEvent(Events::SEND_DATA,_error,_bytesTransferred);
        if (!_error){
            // MODIFY LATER

        } else if ((_error == asio::error::eof) || (_error == asio::error::connection_reset) ||
                _error == asio::error::broken_pipe) {
            if (m_mode == Mode::SERVER) {                
                if (m_is_tcp_connected)
                    m_callback_Str.emitEvent(Events::DISCONNECT, m_tcp_endpoint.address().to_string());
                InitServer();
            }

            if (m_mode == Mode::CLIENT) {
                if (m_is_tcp_connected)
                    m_callback_Str.emitEvent(Events::DISCONNECT, m_tcp_endpoint.address().to_string());
            }
            m_is_tcp_connected = false;

        }else{
            m_is_tcp_connected = false;
            // MODIFY LATER
        }
    }

}