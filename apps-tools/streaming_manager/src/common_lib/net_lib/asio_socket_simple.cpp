#include <fstream>
#include "asio_socket_simple.h"
#include "data_lib/thread_cout.h"
#include "asio_service.h"

#define UNUSED(x) [&x]{}()
#define SOCKET_BUFFER_SIZE 1024
#define CONNECT_TIMEOUT 5

using namespace net_lib;

uint64_t g_sendbufersSSId = 0;
std::map<uint64_t,net_buffer> g_sendbuffersSS;

auto CAsioSocketSimple::create(std::string host, std::string port) -> CAsioSocketSimple::Ptr {
    return std::make_shared<CAsioSocketSimple>(host, port);
}

CAsioSocketSimple::CAsioSocketSimple(std::string host, std::string port) :
        m_mode(EMode::M_NONE),
        m_host(host),
        m_port(port),
        m_tcp_socket(),
        m_tcp_acceptor(),
        m_timoutTimer(CAsioService::instance()->getIO()),
//        m_is_tcp_connected(false),
        m_disableRestartServer(true),
        m_mtx()
{
    m_SocketReadBuffer = new uint8_t[SOCKET_BUFFER_SIZE];
}

CAsioSocketSimple::~CAsioSocketSimple() {
    m_disableRestartServer = true;
    closeSocket();
    delete[] m_SocketReadBuffer;
}

auto CAsioSocketSimple::initServer() -> void {
    closeSocket();
    std::lock_guard<std::mutex> lock(m_mtx);
//    m_is_tcp_connected = false;
    m_disableRestartServer = false;        
    m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(CAsioService::instance()->getIO());
    m_tcp_acceptor = std::make_shared<asio::ip::tcp::acceptor>(CAsioService::instance()->getIO());
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), std::stoi(m_port));
    m_tcp_acceptor->open(endpoint.protocol());
    m_tcp_acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
    m_tcp_acceptor->bind(endpoint);
    m_tcp_acceptor->listen();
    m_tcp_acceptor->async_accept(*m_tcp_socket,m_tcp_endpoint, std::bind(&CAsioSocketSimple::handlerAccept, this, std::placeholders::_1));
    m_mode = EMode::M_SERVER;
}

auto CAsioSocketSimple::initClient() -> void {
    std::lock_guard<std::mutex> lock(m_mtx);
//    m_is_tcp_connected = false;
    m_tcp_acceptor = nullptr;
    m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(CAsioService::instance()->getIO());
    asio::ip::tcp::resolver resolver(CAsioService::instance()->getIO());
    asio::ip::tcp::resolver::query query(m_host, m_port);
    asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
    m_tcp_endpoint = *iter;
    m_tcp_socket->async_connect(m_tcp_endpoint, std::bind(&CAsioSocketSimple::handlerConnect, this, std::placeholders::_1 , iter));
    m_mode = EMode::M_CLIENT;
    m_timoutTimer.expires_after(std::chrono::seconds(CONNECT_TIMEOUT));
    m_timoutTimer.async_wait([this](std::error_code er){
        if (er.value() != asio::error::operation_aborted) {
            try {
                closeSocket();
            }catch (...){

            }
            this->m_timoutTimer.cancel();
            connectTimeoutNotify(er);
        }
    });
}

auto CAsioSocketSimple::handlerConnect(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator) -> void {
    try {
        this->m_timoutTimer.cancel();
        if (!_error)
        {
            connectNotify(m_tcp_endpoint.address().to_string());
            std::lock_guard<std::mutex> lock(m_mtx);
            if (m_tcp_socket){
//                m_is_tcp_connected = true;
                m_tcp_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
                    std::bind(&CAsioSocketSimple::handlerReceive, this,
                        std::placeholders::_1, std::placeholders::_2));
            }
        }
        else if (endpoint_iterator != asio::ip::tcp::resolver::iterator()) {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (m_tcp_socket){
                m_tcp_socket->close();
                m_tcp_endpoint = *endpoint_iterator;
                m_tcp_socket->async_connect(m_tcp_endpoint, std::bind(&CAsioSocketSimple::handlerConnect, this,
                    std::placeholders::_1, ++endpoint_iterator));
            }
        }
        else
        {
            errorNotify(_error);
        }
    }
    catch (...) {
        errorNotify(_error);
    }
}

auto CAsioSocketSimple::closeSocket() -> void {
    std::lock_guard<std::mutex> lock(m_mtx);
    bool emit = false;
    try {
            if (m_tcp_socket) {

                emit = true;
                if (m_tcp_acceptor)
                    m_tcp_acceptor->cancel();
                m_tcp_acceptor = nullptr;
                m_tcp_socket = nullptr;
            }        
    }catch (...){
    }

    try {
        if (emit)
            disconnectNotify(m_tcp_endpoint.address().to_string());
    }catch (...){
    }
}

auto CAsioSocketSimple::handlerReceive(const asio::error_code &error,size_t bytes_transferred) -> void {
    // Operation aborted
    if (error.value() == 125) return;

    if (!error){
        recivedNotify(error,m_SocketReadBuffer,bytes_transferred);
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_tcp_socket){
            m_tcp_socket->async_receive(
                    asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
                    std::bind(&CAsioSocketSimple::handlerReceive, this,
                            std::placeholders::_1,std::placeholders::_2));
        }
    }else{
        errorNotify(error);
        if (m_mode == EMode::M_SERVER && !m_disableRestartServer) {
                initServer();
           }
    }
}

auto CAsioSocketSimple::isConnected() -> bool{
    std::lock_guard<std::mutex> lock(m_mtx);
    if (m_tcp_socket) {
        return m_tcp_socket->is_open();
    }
    return false;
}

auto CAsioSocketSimple::handlerAccept(const asio::error_code &_error) -> void {
    // Operation aborted
    if (_error.value() == 125) return;

    if (!_error)
    {
        connectNotify(m_tcp_endpoint.address().to_string());
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_tcp_socket){
            m_tcp_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
                                        std::bind(&CAsioSocketSimple::handlerReceive, this,
                                                  std::placeholders::_1, std::placeholders::_2));
        }
    }
    else if (_error.value() != 1) // Already open connection
    {
        errorNotify(_error);
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_tcp_socket){
            m_tcp_acceptor->async_accept(*m_tcp_socket, m_tcp_endpoint,
                                         std::bind(&CAsioSocketSimple::handlerAccept, this, std::placeholders::_1));
        }
    }
}

auto CAsioSocketSimple::sendBuffer(bool async, net_buffer _buffer, size_t _size) -> bool{
    std::lock_guard<std::mutex> lock(m_mtx);
    asio::error_code _error;
    if (m_tcp_socket && m_tcp_socket->is_open()) {
        if (!async) {
            size_t offset = 0;
            while(offset < _size){
                size_t send_size = m_tcp_socket->send(asio::buffer((uint8_t*)(&(*_buffer.get())+offset), _size-offset), 0, _error);
                if (_error.value() != 0){
                    return false;
                }
                offset += send_size;
            }
            this->handlerSend(_error,offset);
        }else {            
            g_sendbufersSSId++;
            g_sendbuffersSS[g_sendbufersSSId] = std::shared_ptr<uint8_t[]>(_buffer);
            m_tcp_socket->async_send(asio::buffer(_buffer.get(),_size),
                                        std::bind(&CAsioSocketSimple::handlerSend2, this, std::placeholders::_1 ,std::placeholders::_2,g_sendbufersSSId));
        }
        return  true;
    }
    return false;
}

auto CAsioSocketSimple::handlerSend2(const asio::error_code &_error, size_t _bytesTransferred, uint64_t bufferId) -> void{
    g_sendbuffersSS.erase(bufferId);
    handlerSend(_error,_bytesTransferred);
}

auto CAsioSocketSimple::handlerSend(const asio::error_code &_error, size_t _bytesTransferred) -> void{
    // Operation aborted
    if (_error.value() == 125) return;

    sendNotify(_error,_bytesTransferred);
    if (!_error){
        // MODIFY LATER
    } else if ((_error == asio::error::eof) || (_error == asio::error::connection_reset) ||
               _error == asio::error::broken_pipe) {
        errorNotify(_error);
        if (m_mode == EMode::M_SERVER && !m_disableRestartServer) {
            initServer();
        }

   }else{
       errorNotify(_error);
//       closeSocket();
   }
}
