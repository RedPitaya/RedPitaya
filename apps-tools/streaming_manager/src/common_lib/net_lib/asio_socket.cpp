#include <fstream>
#include "asio_socket.h"
#include "data_lib/thread_cout.h"

#define UNUSED(x) [&x]{}()

using namespace net_lib;

uint64_t g_sendbufersId = 0;
std::map<uint64_t,net_buffer> g_sendbuffers;

auto CAsioSocket::create(net_lib::EProtocol _protocol, std::string host, std::string port) -> CAsioSocket::Ptr {
    return std::make_shared<CAsioSocket>( _protocol, host, port);
}

CAsioSocket::CAsioSocket(net_lib::EProtocol _protocol, std::string host, std::string port) :
        m_mode(net_lib::EMode::M_NONE),
        m_protocol(_protocol),
        m_host(host),
        m_port(port),        
        m_udp_socket(0),
        m_tcp_socket(0),
        m_tcp_acceptor(0),
        m_udp_endpoint(),
        m_pos_last_in_fifo(0),
        m_last_pack_id(0),
        m_mtx(),
        m_asio(new CAsioService())
{
    m_SocketReadBuffer = new uint8_t[SOCKET_BUFFER_SIZE];
    m_tcp_fifo_buffer  = new uint8_t[FIFO_BUFFER_SIZE];
}

CAsioSocket::~CAsioSocket() {
    closeSocket();    
    delete m_asio;
    delete[] m_SocketReadBuffer;
    delete[] m_tcp_fifo_buffer;
}

void CAsioSocket::initServer() {
    closeSocket();
    std::lock_guard<std::mutex> lock(m_mtx);
    m_last_pack_id = 0;
    if (m_protocol == net_lib::EProtocol::P_UDP) {        
        m_udp_socket = std::make_shared<asio::ip::udp::udp::socket>(m_asio->getIO(), asio::ip::udp::udp::endpoint(asio::ip::udp::udp::v4(), std::stoi(m_port)));
        m_udp_socket->set_option(asio::ip::udp::socket::reuse_address(true));
        waitClient();
    }

    if (m_protocol == net_lib::EProtocol::P_TCP) {

        m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(m_asio->getIO());
        m_tcp_acceptor = std::make_shared<asio::ip::tcp::acceptor>(m_asio->getIO());
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), std::stoi(m_port));
        m_tcp_acceptor->open(endpoint.protocol());
        m_tcp_acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
        m_tcp_acceptor->bind(endpoint);
        m_tcp_acceptor->listen();
        m_tcp_acceptor->async_accept(*m_tcp_socket,m_tcp_endpoint, std::bind(&CAsioSocket::handlerAcceptFromClient, this, std::placeholders::_1));
    }
    m_mode = net_lib::EMode::M_SERVER;
}

auto CAsioSocket::closeSocket() -> void{
    std::lock_guard<std::mutex> lock(m_mtx);
    bool emitUDP = false;
    bool emitTCP = false;
    try {
        if (m_tcp_socket){
            emitTCP = true;
            if (m_tcp_acceptor)
                m_tcp_acceptor->cancel();
            m_tcp_acceptor = nullptr;
            m_tcp_socket = nullptr;
        }
        if (m_udp_socket) {
            emitUDP = true;
            m_udp_socket = nullptr;
        }
    }catch (...){
    }

    try {
    if (emitUDP)
        disconnectServerNotify(m_udp_endpoint.address().to_string());
    if (emitTCP){
        disconnectServerNotify(m_tcp_endpoint.address().to_string());
    }
    }catch (...){
    }
}

auto CAsioSocket::waitClient() -> void{

    if (m_protocol == net_lib::EProtocol::P_UDP) {
        if (m_udp_socket){            
            m_udp_socket->async_receive_from(
                    asio::buffer(m_udp_recv_server_buffer, 1), m_udp_endpoint,
                    std::bind(&CAsioSocket::handlerReceiveFromClient, this,
                              std::placeholders::_1));
        }
    }
}

auto CAsioSocket::handlerReceiveFromClient(const asio::error_code &error) -> void {
    // Operation aborted    
    if (error.value() == 125) return;

    if (!error) {
        auto is_udp_connected = (bool) m_udp_recv_server_buffer[0];
        if (is_udp_connected){
            connectServerNotify(m_udp_endpoint.address().to_string());
        }
        else {
            closeSocket();
        }
    } else {
        closeSocket();
    }
    std::lock_guard<std::mutex> lock(m_mtx);
    if (m_udp_socket){
        m_udp_socket->async_receive_from(
                asio::buffer(m_udp_recv_server_buffer, 1), m_udp_endpoint,
                std::bind(&CAsioSocket::handlerReceiveFromClient, this,
                          std::placeholders::_1));
    }
}

auto CAsioSocket::handlerReceiveFromServer(const asio::error_code &ErrorCode, size_t bytes_transferred) -> void{
    // Operation aborted    
    if (ErrorCode.value() == 125) {
        return;
    }

    if (!ErrorCode) {
    //    std::cout << "Byte received: " << bytes_transferred << "\n";
        if (m_protocol == net_lib::EProtocol::P_TCP || m_protocol == net_lib::EProtocol::P_UDP) {

            if (bytes_transferred + m_pos_last_in_fifo > FIFO_BUFFER_SIZE) {
                std::cerr  << "[CAsioSocket::handlerReceiveFromServer] TCP received buffer overflow\n";
                exit(5);
            }

            // Append new data
            memcpy_neon(m_tcp_fifo_buffer + m_pos_last_in_fifo,m_SocketReadBuffer,bytes_transferred);
            m_pos_last_in_fifo += bytes_transferred;


            uint8_t  size_id = 16;

            bool find_all_flag = false;
//                cout << "Buff size " << m_pos_last_in_fifo << "\n";
            do{
                for (uint32_t i = 0; i < m_pos_last_in_fifo - size_id; ++i) {
                    bool find_flag_pack = false;
                    bool find_flag_pack_end = false;
                    bool find_flag_buff_pack = false;

                    for (int j = 0; j < size_id; ++j) {
                        if (m_tcp_fifo_buffer[i + j] == ID_PACK[j]) {
                            find_flag_pack = true;
                        } else {
                            find_flag_pack = false;
                        }

                        if (m_tcp_fifo_buffer[i + j] == ID_PACK_END[j]) {
                            find_flag_pack_end = true;
                        } else {
                            find_flag_pack_end = false;
                        }

                        if (m_tcp_fifo_buffer[i + j] == ID_BUFFER[j]) {
                            find_flag_buff_pack = true;
                        } else {
                            find_flag_buff_pack = false;
                        }
                        if (!find_flag_pack && !find_flag_pack_end && !find_flag_buff_pack){
                            break;
                        }
                    }

    //                   std::cout << i << " pos " <<  m_pos_last_in_fifo << "\n";

                    if (find_flag_pack || find_flag_pack_end || find_flag_buff_pack) {
                        uint32_t pack_size = ((uint64_t *) (m_tcp_fifo_buffer + i))[2];
                        if ((pack_size + i) <= m_pos_last_in_fifo) {
                            recivedNotify(ErrorCode,
                                          m_tcp_fifo_buffer + i,
                                          (uint32_t) pack_size);
                            if (m_pos_last_in_fifo - pack_size - i > 0){
                                for(auto z = 0u; z < m_pos_last_in_fifo - pack_size - i; ++z){
                                    m_tcp_fifo_buffer[z] = (m_tcp_fifo_buffer + i + pack_size)[z];
                                }
//                                memcpy_neon(m_tcp_fifo_buffer, m_tcp_fifo_buffer + i + pack_size,
//                                       m_pos_last_in_fifo - pack_size - i);
                            }
                            m_pos_last_in_fifo = m_pos_last_in_fifo - pack_size - i;
                            find_all_flag = true;
                        }
                        else{
                            find_all_flag = false;
    //                            std::cout << "Header persent " << i << " size " << m_pos_last_in_fifo <<" \n";
                        }
                        break;
                    } else{
                        find_all_flag = false;
                    }
                }
                if (m_pos_last_in_fifo <= size_id)
                    break;
            } while (find_all_flag);
        }
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_udp_socket && m_protocol == net_lib::EProtocol::P_UDP) {
            m_udp_socket->async_receive_from(
                    asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE), m_udp_endpoint,
                    std::bind(&CAsioSocket::handlerReceiveFromServer, this,
                              std::placeholders::_1, std::placeholders::_2));
        }
        if (m_tcp_socket && m_protocol == net_lib::EProtocol::P_TCP) {
            m_tcp_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
                                        std::bind(&CAsioSocket::handlerReceiveFromServer, this,
                                                  std::placeholders::_1, std::placeholders::_2));
        }
    }else{
        errorClientNotify(ErrorCode);
        closeSocket();
    }
}

auto CAsioSocket::isConnected() -> bool{
    std::lock_guard<std::mutex> lock(m_mtx);
    if (m_tcp_socket) {
        return m_tcp_socket->is_open();
    }
    if (m_udp_socket) {
        return m_udp_socket->is_open();
    }
    return false;
}

auto CAsioSocket::handlerAcceptFromClient(const asio::error_code &_error) -> void {
    if (!_error){
        connectServerNotify(m_tcp_endpoint.address().to_string());
//        m_is_tcp_connected = true;
    }
    else if (_error.value() != 1) // Already open connection
    {
        errorServerNotify(_error);
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_tcp_acceptor)
            m_tcp_acceptor->async_accept(*m_tcp_socket, m_tcp_endpoint, std::bind(&CAsioSocket::handlerAcceptFromClient, this, std::placeholders::_1));
//        m_is_tcp_connected = false;
    }
}

void CAsioSocket::handlerConnectToServer(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator)
{
    try {
        if (!_error)
        {
            connectClientNotify(m_tcp_endpoint.address().to_string());
            std::lock_guard<std::mutex> lock(m_mtx);
            if (m_tcp_socket){
//                m_is_tcp_connected = true;
                m_tcp_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
                    std::bind(&CAsioSocket::handlerReceiveFromServer, this,
                        std::placeholders::_1, std::placeholders::_2));
            }
        }
        else if (endpoint_iterator != asio::ip::tcp::resolver::iterator()) {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (m_tcp_socket){
                m_tcp_endpoint = *endpoint_iterator;
                m_tcp_socket->async_connect(m_tcp_endpoint, std::bind(&CAsioSocket::handlerConnectToServer, this,
                    std::placeholders::_1, ++endpoint_iterator));
            }
        }
        else
        {
            errorClientNotify(_error);
//            m_is_tcp_connected = false;
        }
    }
    catch (...) {
        aprintf(stderr,"Error connect to server\n");
        errorClientNotify(_error);
//        m_is_tcp_connected = false;
    }
}


auto CAsioSocket::initClient() -> void{
    std::lock_guard<std::mutex> lock(m_mtx);
//    m_is_udp_connected = false;
//    m_is_tcp_connected = false;
    m_pos_last_in_fifo = 0;
    if (m_protocol == net_lib::EProtocol::P_UDP) {
        asio::ip::udp::udp::resolver resolver(m_asio->getIO());
        asio::ip::udp::udp::resolver::query query(asio::ip::udp::udp::v4(), m_host, m_port);
        asio::ip::udp::udp::resolver::iterator iter = resolver.resolve(query);
        m_udp_socket = std::make_shared<asio::ip::udp::udp::socket>(m_asio->getIO(), asio::ip::udp::udp::endpoint(asio::ip::udp::udp::v4(), 0));
        m_udp_endpoint = *iter;
        m_udp_socket->send_to(asio::buffer("\x01",1),m_udp_endpoint);
        connectClientNotify(m_udp_endpoint.address().to_string());
        m_udp_socket->async_receive_from(
                asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE), m_udp_endpoint,
                std::bind(&CAsioSocket::handlerReceiveFromServer, this,
                          std::placeholders::_1, std::placeholders::_2));

    }

    if (m_protocol == net_lib::EProtocol::P_TCP) {

        m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(m_asio->getIO());
        asio::ip::tcp::resolver resolver(m_asio->getIO());
        asio::ip::tcp::resolver::query query(m_host, m_port);
        asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
        m_tcp_endpoint = *iter;
        m_tcp_socket->async_connect(m_tcp_endpoint, std::bind(&CAsioSocket::handlerConnectToServer, this, std::placeholders::_1 , iter));

    }
    m_mode = net_lib::EMode::M_CLIENT;
}

void CAsioSocket::sendBuffer(net_buffer _buffer, size_t _size) {
    try {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_protocol == net_lib::EProtocol::P_UDP) {
            if (m_udp_socket)
                m_udp_socket->send_to(asio::buffer(_buffer.get(), _size), m_udp_endpoint);
        }
        if (m_protocol == net_lib::EProtocol::P_TCP) {
            if (m_tcp_socket)
                m_tcp_socket->send(asio::buffer(_buffer.get(), _size));
        }
    }
    catch (...) {
//        if (m_is_udp_connected || m_is_tcp_connected)
         aprintf(stderr,"Error send buffer to socket\n");
    }
}

auto CAsioSocket::sendSyncBuffer(AsioBufferNolder &_buffer) -> bool{
    std::lock_guard<std::mutex> lock(m_mtx);
    asio::error_code _error;
    if (m_protocol == net_lib::EProtocol::P_UDP){
        if (m_udp_socket) {
            std::vector<asio::const_buffer> buffers;
            buffers.push_back(asio::buffer(_buffer.header,_buffer.headerLen));
            if (_buffer.dataPtr){
                buffers.push_back(asio::buffer(_buffer.dataPtr,_buffer.dataLen));
            }
            m_udp_socket->send_to(buffers, m_udp_endpoint, 0, _error);
            this->handlerSend(_error,_buffer.headerLen + _buffer.dataLen);
            return  true;
        }
    }
    if (m_protocol == net_lib::EProtocol::P_TCP){
        if (m_tcp_socket) {
            std::vector<asio::const_buffer> buffers;
            buffers.push_back(asio::buffer(_buffer.header,_buffer.headerLen));
            if (_buffer.dataPtr){
                buffers.push_back(asio::buffer(_buffer.dataPtr,_buffer.dataLen));
            }
            m_tcp_socket->send(buffers, 0, _error);
            this->handlerSend(_error,_buffer.headerLen + _buffer.dataLen);
            return  true;
        }
    }
    return false;
}


auto CAsioSocket::sendBuffer(bool async, net_lib::net_buffer _buffer, size_t _size) -> bool{
    std::lock_guard<std::mutex> lock(m_mtx);
    asio::error_code _error;
    if (m_protocol == net_lib::EProtocol::P_UDP){
        if (m_udp_socket) {
            if (!async) {
                m_udp_socket->send_to(asio::buffer(_buffer.get(), _size), m_udp_endpoint, 0, _error);
                this->handlerSend(_error,_size);
            } else {
                g_sendbufersId++;
                g_sendbuffers[g_sendbufersId] = std::shared_ptr<uint8_t[]>(_buffer);
                m_udp_socket->async_send_to(asio::buffer(_buffer.get(),_size),m_udp_endpoint,
                                        std::bind(&CAsioSocket::handlerSend2, this, std::placeholders::_1 ,std::placeholders::_2,g_sendbufersId));
            }
            return  true;
        }
    }
    if (m_protocol == net_lib::EProtocol::P_TCP){
        if (m_tcp_socket) {
            if (!async) {
                m_tcp_socket->send(asio::buffer(_buffer.get(), _size), 0, _error);
                this->handlerSend(_error,_size);
            }else {
                g_sendbufersId++;
                g_sendbuffers[g_sendbufersId] = std::shared_ptr<uint8_t[]>(_buffer);
                m_tcp_socket->async_send(asio::buffer(_buffer.get(),_size),
                                            std::bind(&CAsioSocket::handlerSend2, this, std::placeholders::_1 ,std::placeholders::_2,g_sendbufersId));
            }
            return  true;
        }
    }
    return false;
}

auto CAsioSocket::handlerSend2(const asio::error_code &_error, size_t _bytesTransferred, uint64_t bufferId) -> void{
    g_sendbuffers.erase(bufferId);
    handlerSend(_error,_bytesTransferred);
}

void CAsioSocket::handlerSend(const asio::error_code &_error, size_t _bytesTransferred){
    sendNotify(_error,_bytesTransferred);
    if (!_error){
        // MODIFY LATER
    } else {
//        closeSocket();
    }
}

