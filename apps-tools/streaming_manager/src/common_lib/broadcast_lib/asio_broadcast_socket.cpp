#include <functional>

#include "asio.hpp"
#include "asio_broadcast_socket.h"

#define SOCKET_BUFFER_SIZE 1024

using namespace broadcast_lib;

class CAsioBroadcastSocket::Impl
{
public:
	Impl()
		: m_Ios()
		, m_asio_th(nullptr)
		, m_socket(0){};

	asio::io_context m_Ios;
	std::thread *m_asio_th;
	std::shared_ptr<asio::ip::udp::socket> m_socket;
};

auto CAsioBroadcastSocket::create(EModel model, std::string host, uint16_t port) -> CAsioBroadcastSocket::Ptr
{
	return std::make_shared<CAsioBroadcastSocket>(model, host, port);
}

CAsioBroadcastSocket::CAsioBroadcastSocket(EModel model, std::string host, uint16_t port)
	: m_mode(EMode::AB_NONE)
	, m_sleep_time_ms(1000)
	, m_host(host)
	, m_port(port)
	, m_pimpl(new Impl())
{
	m_model = model;
	m_SocketReadBuffer = new uint8_t[SOCKET_BUFFER_SIZE];
	m_pimpl->m_asio_th = new std::thread([this]() {
		asio::executor_work_guard<asio::io_context::executor_type> m_work = asio::make_work_guard(this->m_pimpl->m_Ios);
		this->m_pimpl->m_Ios.run();
	});
}

CAsioBroadcastSocket::~CAsioBroadcastSocket()
{
	closeSocket();
	m_pimpl->m_Ios.stop();
	if (m_pimpl->m_asio_th) {
		m_pimpl->m_asio_th->join();
		delete m_pimpl->m_asio_th;
	}
	delete[] m_SocketReadBuffer;
}

auto CAsioBroadcastSocket::initServer(EMode mode, int sleep_time_ms) -> void
{
	m_sleep_time_ms = sleep_time_ms;
	if (mode != EMode::AB_SERVER_MASTER && mode != EMode::AB_SERVER_SLAVE) {
		std::error_code error = std::make_error_code(std::errc::invalid_argument);
		errorNotify(error);
		return;
	}
	std::error_code error;
	m_pimpl->m_socket = std::make_shared<asio::ip::udp::socket>(m_pimpl->m_Ios);
	m_pimpl->m_socket->open(asio::ip::udp::v4(), error);
	if (!error) {
		m_mode = mode;
		m_pimpl->m_socket->set_option(asio::ip::udp::socket::reuse_address(true));
		m_pimpl->m_socket->set_option(asio::socket_base::broadcast(true));
		asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), m_port);
		std::string model = std::to_string(static_cast<uint8_t>(m_model));
		assert(model.size() == 1);
		std::string buf = m_host + std::string((mode == EMode::AB_SERVER_MASTER ? "M" : "S")) + model;
		m_pimpl->m_socket->async_send_to(asio::buffer(buf.c_str(), buf.size()),
										 senderEndpoint,
										 std::bind(&CAsioBroadcastSocket::handlerSend, this, std::placeholders::_1, std::placeholders::_2));
	} else {
		errorNotify(error);
	}
}

auto CAsioBroadcastSocket::initClient() -> void
{
	std::error_code error;
	asio::ip::udp::endpoint senderEndpoint(asio::ip::udp::v4(), m_port);
	m_pimpl->m_socket = std::make_shared<asio::ip::udp::socket>(m_pimpl->m_Ios, senderEndpoint);
	m_mode = EMode::AB_CLIENT;
	m_pimpl->m_socket->set_option(asio::ip::udp::socket::reuse_address(true));
	m_pimpl->m_socket->set_option(asio::socket_base::broadcast(true));
	m_pimpl->m_socket->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
									 std::bind(&CAsioBroadcastSocket::handlerReceive, this, std::placeholders::_1, std::placeholders::_2));
}

auto CAsioBroadcastSocket::closeSocket() -> void
{
	try {
		if (m_pimpl->m_socket && m_pimpl->m_socket->is_open()) {
			m_pimpl->m_socket->shutdown(asio::socket_base::shutdown_type::shutdown_both);
			m_pimpl->m_socket->cancel();
			m_pimpl->m_socket->close();
		}
	} catch (...) {
	}
}

auto CAsioBroadcastSocket::handlerReceive(const asio::error_code &error, size_t bytes_transferred) -> void
{
	if (!error) {
		receivedNotify(error, m_SocketReadBuffer, bytes_transferred);
		asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::any(), m_port);
		m_pimpl->m_socket
			->async_receive(asio::buffer(m_SocketReadBuffer, SOCKET_BUFFER_SIZE),
							std::bind(&CAsioBroadcastSocket::handlerReceive, this, std::placeholders::_1, std::placeholders::_2));
	} else {
		if (error.value() != 995) // Mute "The I/O operation has been aborted because of either a thread exit or an application request"
			errorNotify(error);
		closeSocket();
	}
}

void CAsioBroadcastSocket::handlerSend(const asio::error_code &_error, size_t _bytesTransferred)
{
	sendNotify(_error, _bytesTransferred);
	if (_error) {
		if (m_mode == EMode::AB_SERVER_MASTER || m_mode == EMode::AB_SERVER_SLAVE) {
			errorNotify(_error);
#ifdef _WIN32
			Sleep(m_sleep_time_ms);
#else
			usleep(m_sleep_time_ms * 1000);
#endif
			initServer(m_mode, m_sleep_time_ms);
		}
	} else {
#ifdef _WIN32
		Sleep(m_sleep_time_ms);
#else
		usleep(m_sleep_time_ms * 1000);
#endif
		asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), m_port);
		std::string model = std::to_string(static_cast<uint8_t>(m_model));
		assert(model.size() == 1);
		std::string buf = m_host + std::string((m_mode == EMode::AB_SERVER_MASTER ? "M" : "S")) + model;
		m_pimpl->m_socket->async_send_to(asio::buffer(buf.c_str(), buf.size()),
										 senderEndpoint,
										 std::bind(&CAsioBroadcastSocket::handlerSend, this, std::placeholders::_1, std::placeholders::_2));
	}
}
