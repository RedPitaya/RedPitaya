#include <fstream>
#include "asio.hpp"
#include "AsioNet.h"

#define UNUSED(x) [&x]{}()



namespace  asionet {

    constexpr char ID_PACK[] = "STREAMpackIDv1.0";

    CAsioSocket::Ptr
    CAsioSocket::Create(asio::io_service &io, asionet::Protocol _protocol, std::string host, std::string port) {
        return std::make_shared<CAsioSocket>(io, _protocol, host, port);
    }

    CAsioSocket::CAsioSocket(asio::io_service &io, asionet::Protocol _protocol, std::string host, std::string port) :
            m_mode(Mode::NONE),
            m_protocol(_protocol),
            m_host(host),
            m_port(port),
            m_io_service(io),
            m_udp_socket(0),
            m_tcp_socket(0),
            m_tcp_acceptor(0),
            m_udp_endpoint(),
            m_is_udp_connected(false),
            m_is_tcp_connected(false),
            m_pos_last_in_fifo(0),
            m_last_pack_id(0)
    {
        m_SocketReadBuffer = new uint8_t[SOCKET_BUFFER_SIZE];
        m_tcp_fifo_buffer = new uint8_t[FIFO_BUFFER_SIZE];
    }

    CAsioSocket::~CAsioSocket() {
        CloseSocket();
        delete [] m_SocketReadBuffer;
        delete [] m_tcp_fifo_buffer;

    }



    void CAsioSocket::InitServer() {


        m_is_udp_connected = false;
        m_is_tcp_connected = false;
        m_last_pack_id = 0;
        if (m_protocol == asionet::Protocol::UDP) {
            m_udp_socket = std::make_shared<asio::ip::udp::udp::socket>(m_io_service, asio::ip::udp::udp::endpoint(asio::ip::udp::udp::v4(), std::stoi(m_port)));
            m_udp_socket->set_option(asio::ip::udp::socket::reuse_address(true));
            WaitClient();
        }

        if (m_protocol == asionet::Protocol::TCP) {

            m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(m_io_service);
            m_tcp_acceptor = std::make_shared<asio::ip::tcp::acceptor>(m_io_service);
            asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), std::stoi(m_port));
            m_tcp_acceptor->open(endpoint.protocol());
            m_tcp_acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
            m_tcp_acceptor->bind(endpoint);
            m_tcp_acceptor->listen();
            m_tcp_acceptor->async_accept(*m_tcp_socket,m_tcp_endpoint, std::bind(&CAsioSocket::HandlerAcceptFromClient, this, std::placeholders::_1));
        }
        m_mode = Mode::SERVER;
    }

    void CAsioSocket::CloseSocket(){

        try {
            if (m_is_udp_connected)
                m_callback_Str.emitEvent(Events::DISCONNECT_SERVER, m_udp_endpoint.address().to_string());
            if (m_is_tcp_connected)
                m_callback_Str.emitEvent(Events::DISCONNECT_SERVER, m_tcp_endpoint.address().to_string());
            }catch (...){

        }
        try {
            if (m_tcp_socket && m_tcp_socket->is_open()){
                m_tcp_socket->cancel();
                m_tcp_socket->shutdown(socket_base::shutdown_type::shutdown_both);
                m_tcp_socket->close();
                m_is_tcp_connected = false;
            }
            if (m_udp_socket && m_udp_socket->is_open()) {
                m_udp_socket->cancel();
                m_udp_socket->shutdown(socket_base::shutdown_type::shutdown_both);
                m_udp_socket->close();
                m_is_udp_connected = false;
            }
        }catch (...){

        }
    }

    void CAsioSocket::WaitClient() {
        if (m_protocol == asionet::Protocol::UDP) {
            m_udp_socket->async_receive_from(
                    asio::buffer(m_udp_recv_server_buffer, 1), m_udp_endpoint,
                    std::bind(&CAsioSocket::HandlerReceiveFromClient, this,
                              std::placeholders::_1));
        }
    }

    void CAsioSocket::HandlerReceiveFromClient(const asio::error_code &error) {
        if (!error) {
            m_is_udp_connected = (bool) m_udp_recv_server_buffer[0];
            if (m_is_udp_connected){
                m_callback_Str.emitEvent(Events::CONNECT_SERVER,m_udp_endpoint.address().to_string());
            }
            else {
                m_callback_Str.emitEvent(Events::DISCONNECT_SERVER,m_udp_endpoint.address().to_string());
            }
        } else {
            m_callback_Str.emitEvent(Events::DISCONNECT_SERVER,m_udp_endpoint.address().to_string());
            m_is_udp_connected = false;
        }
        m_udp_socket->async_receive_from(
                asio::buffer(m_udp_recv_server_buffer, 1), m_udp_endpoint,
                std::bind(&CAsioSocket::HandlerReceiveFromClient, this,
                          std::placeholders::_1));
    }

    void CAsioSocket::HandlerReceiveFromServer(const asio::error_code &ErrorCode, size_t bytes_transferred){
        if (!ErrorCode) {
        //    std::cout << "Byte received: " << bytes_transferred << "\n";
            if (m_protocol == Protocol::TCP) {

                if (bytes_transferred + m_pos_last_in_fifo > FIFO_BUFFER_SIZE) {
                    std::cerr  << "[rspa] TCP received buffer overflow\n";
                    exit(5);
                }

                memcpy(m_tcp_fifo_buffer + m_pos_last_in_fifo,m_SocketReadBuffer,bytes_transferred);
                m_pos_last_in_fifo += bytes_transferred;

                uint8_t  size_id = sizeof(ID_PACK) - 1;
                bool find_all_flag = false;
//                cout << "Buff size " << m_pos_last_in_fifo << "\n";
                do{

                for (uint32_t i = 0; i < m_pos_last_in_fifo - size_id; ++i) {
                    bool find_flag = false;
                    for (int j = 0; j < size_id; ++j) {
                        if (m_tcp_fifo_buffer[i + j] == ID_PACK[j]) {
                            find_flag = true;
                        } else {
                            find_flag = false;
                            break;
                        }
                    }
 //                   std::cout << i << " pos " <<  m_pos_last_in_fifo << "\n";

                    if (find_flag) {
                        uint32_t pack_size = ((uint32_t *) (m_tcp_fifo_buffer + i))[9];
                        if ((pack_size + i) <= m_pos_last_in_fifo) {
                            m_callbackErrorUInt8Int.emitEvent(Events::RECIVED_DATA_FROM_SERVER, ErrorCode,
                                                              m_tcp_fifo_buffer + i,
                                                              (uint32_t) pack_size);

                            memcpy(m_tcp_fifo_buffer, m_tcp_fifo_buffer + i + pack_size,
                                   m_pos_last_in_fifo - pack_size - i);
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

            if (m_protocol == Protocol::UDP) {
                if (strncmp((const char*)m_SocketReadBuffer,ID_PACK,16) == 0) {
                    uint64_t id_pack = ((uint64_t *) (m_SocketReadBuffer))[2];
                    if (id_pack > m_last_pack_id)
                    {
                        m_callbackErrorUInt8Int.emitEvent(Events::RECIVED_DATA_FROM_SERVER, ErrorCode,
                                                          m_SocketReadBuffer,
                                                          (uint32_t) bytes_transferred);
                        m_last_pack_id = id_pack;
                    }
                }
            }


            if (m_protocol == Protocol::UDP) {
                m_udp_socket->async_receive_from(
                        asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE), m_udp_endpoint,
                        std::bind(&CAsioSocket::HandlerReceiveFromServer, this,
                                  std::placeholders::_1, std::placeholders::_2));
            }
            if (m_protocol == Protocol::TCP) {
                m_tcp_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
                                            std::bind(&CAsioSocket::HandlerReceiveFromServer, this,
                                                      std::placeholders::_1, std::placeholders::_2));
            }
        }else{
            m_callback_Error.emitEvent(Events::ERROR_CLIENT,ErrorCode);
            CloseSocket();
        }
    }

    bool CAsioSocket::IsConnected(){
        return m_is_tcp_connected || m_is_udp_connected;
    }

    void CAsioSocket::HandlerAcceptFromClient(const asio::error_code &_error)
    {
        if (!_error)
        {

            m_callback_Str.emitEvent(Events::CONNECT_SERVER,m_tcp_endpoint.address().to_string());
            m_is_tcp_connected = true;
        }
        else if (_error.value() != 1) // Already open connection
        {
            m_callback_Error.emitEvent(Events::ERROR_SERVER,_error);
            m_tcp_acceptor->async_accept(*m_tcp_socket, m_tcp_endpoint, std::bind(&CAsioSocket::HandlerAcceptFromClient, this, std::placeholders::_1));
            m_is_tcp_connected = false;
        }
    }

    void CAsioSocket::HandlerConnectToServer(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator)
    {
		try {
			if (!_error)
			{
				m_callback_Str.emitEvent(Events::CONNECT_CLIENT, m_tcp_endpoint.address().to_string());
				m_is_tcp_connected = true;
				m_tcp_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
					std::bind(&CAsioSocket::HandlerReceiveFromServer, this,
						std::placeholders::_1, std::placeholders::_2));
			}
			else if (endpoint_iterator != asio::ip::tcp::resolver::iterator()) {
				m_tcp_socket->close();
				m_tcp_endpoint = *endpoint_iterator;
				m_tcp_socket->async_connect(m_tcp_endpoint, std::bind(&CAsioSocket::HandlerConnectToServer, this,
					std::placeholders::_1, ++endpoint_iterator));
			}
			else
			{
				m_callback_Error.emitEvent(Events::ERROR_CLIENT, _error);
				m_is_tcp_connected = false;
			}
		}
		catch (...) {
			std::cerr << "Error connect to server\n";
			m_callback_Error.emitEvent(Events::ERROR_CLIENT, _error);
			m_is_tcp_connected = false;
		}
    }


    void CAsioSocket::InitClient(){
        m_is_udp_connected = false;
        m_is_tcp_connected = false;
        m_pos_last_in_fifo = 0;
        if (m_protocol == asionet::Protocol::UDP) {
            asio::ip::udp::udp::resolver resolver(m_io_service);
            asio::ip::udp::udp::resolver::query query(asio::ip::udp::udp::v4(), m_host, m_port);
            asio::ip::udp::udp::resolver::iterator iter = resolver.resolve(query);
            m_udp_socket = std::make_shared<asio::ip::udp::udp::socket>(m_io_service, asio::ip::udp::udp::endpoint(asio::ip::udp::udp::v4(), 0));
            m_udp_endpoint = *iter;
            m_udp_socket->send_to(asio::buffer("\x01",1),m_udp_endpoint);
            m_callback_Str.emitEvent(Events::CONNECT_CLIENT,m_udp_endpoint.address().to_string());
            m_udp_socket->async_receive_from(
                    asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE), m_udp_endpoint,
                    std::bind(&CAsioSocket::HandlerReceiveFromServer, this,
                              std::placeholders::_1, std::placeholders::_2));

        }

        if (m_protocol == asionet::Protocol::TCP) {

            m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(m_io_service);
            asio::ip::tcp::resolver resolver(m_io_service);
            asio::ip::tcp::resolver::query query(m_host, m_port);
            asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
            m_tcp_endpoint = *iter;
            m_tcp_socket->async_connect(m_tcp_endpoint, std::bind(&CAsioSocket::HandlerConnectToServer, this, std::placeholders::_1 , iter));

        }
        m_mode = Mode::CLIENT;
    }
    void CAsioSocket::addHandler(Events _event, std::function<void(std::string host)> _func){
        this->m_callback_Str.addListener(_event,_func);
    }

    void CAsioSocket::addHandler(Events _event, std::function<void(std::error_code error)> _func){
        this->m_callback_Error.addListener(_event,_func);
    }

    void CAsioSocket::addHandler(Events _event, std::function<void(std::error_code error,size_t)> _func){
        this->m_callback_ErrorInt.addListener(_event,_func);
    }

    void CAsioSocket::addHandler(Events _event, std::function<void(std::error_code error,uint8_t*,size_t)> _func){
        this->m_callbackErrorUInt8Int.addListener(_event,_func);
    }

    void CAsioSocket::SendBuffer(const void *_buffer, size_t _size) {
		try {
			if (m_protocol == Protocol::UDP) {
				if (m_udp_socket->is_open())
					m_udp_socket->send_to(asio::buffer(_buffer, _size), m_udp_endpoint);
			}
			if (m_protocol == Protocol::TCP) {
				if (m_tcp_socket->is_open())
					m_tcp_socket->send(asio::buffer(_buffer, _size));
			}
		}
		catch (...) {
		    if (m_is_udp_connected || m_is_tcp_connected)
			    std::cerr << "Error send buffer to socket\n";
		}
    }


    bool CAsioSocket::SendBuffer(bool async, send_buffer _buffer, size_t _size){

        asio::error_code _error;

        if (m_protocol == Protocol::UDP){
            if (m_is_udp_connected && m_udp_socket->is_open()) {
                if (!async) {
                    m_udp_socket->send_to(asio::buffer(_buffer, _size), m_udp_endpoint, 0, _error);
                    this->HandlerSend(_error,_size);
                } else {
                    m_udp_socket->async_send_to(asio::buffer(_buffer,_size),m_udp_endpoint,
                                            std::bind(&CAsioSocket::HandlerSend2, this, std::placeholders::_1 ,std::placeholders::_2,_buffer ));
                }
                return  true;
            }
        }
        if (m_protocol == Protocol::TCP){
            if (m_is_tcp_connected  && m_tcp_socket->is_open()) {
                if (!async) {
                    m_tcp_socket->send(asio::buffer(_buffer, _size), 0, _error);
                    this->HandlerSend(_error,_size);
                }else {
                    m_tcp_socket->async_send(asio::buffer(_buffer,_size),
                                                std::bind(&CAsioSocket::HandlerSend2, this, std::placeholders::_1 ,std::placeholders::_2,_buffer ));

                }
                return  true;
            }
        }

        return false;
    }

    void CAsioSocket::HandlerSend2(const asio::error_code &_error, size_t _bytesTransferred, uint8_t *buffer){
        HandlerSend(_error,_bytesTransferred);
        delete[] buffer;
    }
    void CAsioSocket::HandlerSend(const asio::error_code &_error, size_t _bytesTransferred){

        m_callback_ErrorInt.emitEvent(Events::SEND_DATA,_error,_bytesTransferred);
        if (!_error){
            // MODIFY LATER

        } else if ((_error == asio::error::eof) || (_error == asio::error::connection_reset) ||
                _error == asio::error::broken_pipe) {
            if (m_mode == Mode::SERVER) {
                if (m_is_udp_connected)
                    m_callback_Str.emitEvent(Events::DISCONNECT_SERVER, m_udp_endpoint.address().to_string());
                if (m_is_tcp_connected)
                    m_callback_Str.emitEvent(Events::DISCONNECT_SERVER, m_tcp_endpoint.address().to_string());
                InitServer();
            }

            if (m_mode == Mode::CLIENT) {
                if (m_is_udp_connected)
                    m_callback_Str.emitEvent(Events::DISCONNECT_SERVER, m_udp_endpoint.address().to_string());
                if (m_is_tcp_connected)
                    m_callback_Str.emitEvent(Events::DISCONNECT_SERVER, m_tcp_endpoint.address().to_string());
            }
            m_is_udp_connected = false;
            m_is_tcp_connected = false;

        }else{
            m_is_udp_connected = false;
            m_is_tcp_connected = false;
            // MODIFY LATER
        }
    }





}