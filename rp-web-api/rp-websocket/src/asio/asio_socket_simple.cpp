#include <functional>
#include "asio_socket_simple.h"

#define SOCKET_BUFFER_SIZE 4096
#define CONNECT_TIMEOUT 5

auto CAsioSocketSimple::create(std::string host, uint16_t port) -> CAsioSocketSimple::Ptr
{
	return std::make_shared<CAsioSocketSimple>(host, port);
}

CAsioSocketSimple::CAsioSocketSimple(std::string host, uint16_t port)
	: m_host(host)
	, m_port(port)
	, m_tcp_socket()
	, m_tcp_acceptor()
	, m_asio(new CAsioService())
	, m_timoutTimer(m_asio->getIO())
	, m_disableRestartServer(true)
	, m_mtx()
{
	m_SocketReadBuffer = new uint8_t[SOCKET_BUFFER_SIZE];
}

CAsioSocketSimple::~CAsioSocketSimple()
{
	m_disableRestartServer = true;
	closeSocket();
	delete m_asio;
	delete[] m_SocketReadBuffer;
	m_SocketReadBuffer = nullptr;
	m_asio = nullptr;
}

auto CAsioSocketSimple::initServer() -> void
{
	closeSocket();
	std::lock_guard lock(m_mtx);
	m_disableRestartServer = false;
	m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(m_asio->getIO());
	m_tcp_acceptor = std::make_shared<asio::ip::tcp::acceptor>(m_asio->getIO());
	asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), (int) m_port);
	m_tcp_acceptor->open(endpoint.protocol());
	m_tcp_acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
	m_tcp_acceptor->bind(endpoint);
	m_tcp_acceptor->listen();
	m_tcp_acceptor->async_accept(*m_tcp_socket, m_tcp_endpoint, std::bind(&CAsioSocketSimple::handlerAccept, this, std::placeholders::_1));
}

auto CAsioSocketSimple::closeSocket() -> void
{
	std::lock_guard lock(m_mtx);
	bool emit = false;
	try {
		if (m_tcp_socket) {
			emit = true;
			if (m_tcp_acceptor)
				m_tcp_acceptor->cancel();
			m_tcp_acceptor = nullptr;
			m_tcp_socket = nullptr;
		}
	} catch (...) {
	}

	try {
		if (emit)
			disconnectNotify(m_tcp_endpoint.address().to_string());
	} catch (...) {
	}
}

auto CAsioSocketSimple::isConnected() -> bool
{
	std::lock_guard lock(m_mtx);
	if (m_tcp_socket) {
		return m_tcp_socket->is_open();
	}
	return false;
}

auto CAsioSocketSimple::handlerAccept(const system::error_code &_error) -> void
{
	// Operation aborted
	if (_error.value() == 125)
		return;

	if (!_error) {
		connectNotify(m_tcp_endpoint.address().to_string());
		std::lock_guard lock(m_mtx);
		if (m_tcp_socket) {
			m_tcp_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
										std::bind(&CAsioSocketSimple::handlerReceive, this, std::placeholders::_1, std::placeholders::_2));
		}
	} else if (_error.value() != 1) // Already open connection
	{
		errorNotify(_error);
		std::lock_guard lock(m_mtx);
		if (m_tcp_socket) {
			m_tcp_acceptor->async_accept(*m_tcp_socket,
										 m_tcp_endpoint,
										 std::bind(&CAsioSocketSimple::handlerAccept, this, std::placeholders::_1));
		}
	}
}

auto CAsioSocketSimple::handlerReceive(const system::error_code &error, size_t bytes_transferred) -> void
{
	// Operation aborted
	if (error.value() == 125)
		return;

	if (!error) {
		recivedNotify(error, m_SocketReadBuffer, bytes_transferred);
		std::lock_guard lock(m_mtx);
		if (m_tcp_socket) {
			m_tcp_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
										std::bind(&CAsioSocketSimple::handlerReceive, this, std::placeholders::_1, std::placeholders::_2));
		}
	} else {
		errorNotify(error);
		if (!m_disableRestartServer) {
			initServer();
		}
	}
}

auto CAsioSocketSimple::sendBuffer(uint8_t* _buffer, size_t _size) -> bool
{
	std::lock_guard lock(m_mtx);
	boost::system::error_code _error;
	if (m_tcp_socket && m_tcp_socket->is_open()) {
		size_t offset = 0;
		while (offset < _size) {
			size_t send_size = m_tcp_socket->send(asio::buffer(_buffer + offset, _size - offset), 0, _error);
			if (_error.value() != 0) {
				return false;
			}
			offset += send_size;
		}
		this->handlerSend(_error, offset);
		return true;
	}
	return false;
}

auto CAsioSocketSimple::handlerSend(const system::error_code &_error, size_t _bytesTransferred) -> void
{
	// Operation aborted
	if (_error.value() == 125)
		return;

	sendNotify(_error, _bytesTransferred);
	if (!_error) {
		// MODIFY LATER
	} else if ((_error == boost::asio::error::eof) || (_error == boost::asio::error::connection_reset) || _error == boost::asio::error::broken_pipe) {
		errorNotify(_error);
		if (!m_disableRestartServer) {
			initServer();
		}

	} else {
		errorNotify(_error);
		//       closeSocket();
	}
}
