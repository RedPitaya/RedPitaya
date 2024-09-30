#ifndef _ASIO_SOCKET_SIMPLE_H
#define _ASIO_SOCKET_SIMPLE_H

#include <map>
#include <string>

#include "signal.hpp"
#include "asio_service.h"

using namespace std;
using namespace boost;

class CAsioSocketSimple
{
public:
	using Ptr = std::shared_ptr<CAsioSocketSimple>;

	static Ptr create(std::string host, uint16_t port);

	CAsioSocketSimple(std::string host, uint16_t port);
	~CAsioSocketSimple();

	auto initServer() -> void;
	auto closeSocket() -> void;
	auto isConnected() -> bool;

	auto sendBuffer(uint8_t* _buffer, size_t _size) -> bool;

	sigslot::signal<string &> connectNotify;
	sigslot::signal<string &> disconnectNotify;

	sigslot::signal<const system::error_code> errorNotify;
	sigslot::signal<const system::error_code> connectTimeoutNotify;

	sigslot::signal<const system::error_code, size_t> sendNotify;
	sigslot::signal<const system::error_code, uint8_t *, size_t> recivedNotify;

private:
	CAsioSocketSimple(const CAsioSocketSimple &) = delete;
	CAsioSocketSimple(CAsioSocketSimple &&) = delete;
	CAsioSocketSimple &operator=(const CAsioSocketSimple &) = delete;
	CAsioSocketSimple &operator=(const CAsioSocketSimple &&) = delete;

	auto handlerReceive(const system::error_code &error, size_t bytes_transferred) -> void;
	auto handlerAccept(const system::error_code &_error) -> void;
	auto handlerConnect(const system::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator) -> void;
	auto handlerSend(const system::error_code &_error, size_t _bytesTransferred) -> void;

	std::string m_host;
	uint16_t m_port;

	std::shared_ptr<asio::ip::tcp::socket> m_tcp_socket;
	std::shared_ptr<asio::ip::tcp::acceptor> m_tcp_acceptor;
	asio::ip::tcp::endpoint m_tcp_endpoint;
	CAsioService *m_asio;
	asio::steady_timer m_timoutTimer;
	uint8_t *m_SocketReadBuffer;
	bool m_disableRestartServer;
	std::mutex m_mtx;

};

#endif
