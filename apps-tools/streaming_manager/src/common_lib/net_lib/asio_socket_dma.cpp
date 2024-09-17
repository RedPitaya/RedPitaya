#include <functional>

#include "asio_socket_dma.h"
#include "logger_lib/file_logger.h"

#define MIN_BUFFER_SIZE (10 * 1024 * 1024)

using namespace net_lib;

auto CAsioSocketDMA::create(std::string host, uint16_t port, CBuffersCached::Ptr buffers) -> CAsioSocketDMA::Ptr
{
	return std::make_shared<CAsioSocketDMA>(host, port, buffers);
}

CAsioSocketDMA::CAsioSocketDMA(std::string host, uint16_t port, CBuffersCached::Ptr buffers)
	: m_mode(net_lib::EMode::M_SERVER)
	, m_host(host)
	, m_port(port)
	, m_tcp_socket(0)
	, m_tcp_acceptor(0)
	, m_mtx()
	, m_asio(new CAsioService())
	, m_bufferSize(MIN_BUFFER_SIZE)
	, m_buffers(buffers)
	, m_currentBuffer(nullptr)
	, m_isStopReceive(false)
{
	auto calculateSocketBuffer = []() -> uint64_t {
		auto getTotalSystemMemory = []() -> uint64_t {
#ifndef _WIN32
			uint64_t pages = sysconf(_SC_PHYS_PAGES);
			uint64_t page_size = sysconf(_SC_PAGE_SIZE);
			return pages * page_size;
#else
			MEMORYSTATUSEX status;
			status.dwLength = sizeof(status);
			GlobalMemoryStatusEx(&status);
			return status.ullTotalPhys;
#endif
		};

		auto totalSize = getTotalSystemMemory();
		auto calc = totalSize * 0.05;
		return MIN_BUFFER_SIZE > calc ? MIN_BUFFER_SIZE : calc;
	};

	m_bufferSize = calculateSocketBuffer();
}

CAsioSocketDMA::~CAsioSocketDMA()
{
	stopReceive();
	closeSocket();
	delete m_asio;
	m_asio = nullptr;
}

void CAsioSocketDMA::initServer()
{
	closeSocket();
	std::lock_guard lock(m_mtx);

	m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(m_asio->getIO());
	m_tcp_acceptor = std::make_shared<asio::ip::tcp::acceptor>(m_asio->getIO());
	asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), m_port);
	m_tcp_acceptor->open(endpoint.protocol());
	m_tcp_acceptor->set_option(asio::ip::tcp::no_delay(true));
	m_tcp_acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
#ifndef _WIN32
	using quickack = asio::detail::socket_option::boolean<IPPROTO_TCP, TCP_QUICKACK>;
	m_tcp_acceptor->set_option(quickack(true));
#endif
	asio::socket_base::send_buffer_size option(m_bufferSize * 3);
	m_tcp_acceptor->set_option(option);
	m_tcp_acceptor->bind(endpoint);
	m_tcp_acceptor->listen();
	m_tcp_acceptor->async_accept(*m_tcp_socket,
								 m_tcp_endpoint,
								 std::bind(&CAsioSocketDMA::handlerAcceptFromClient, this, std::placeholders::_1));

	m_mode = net_lib::EMode::M_SERVER;
}

auto CAsioSocketDMA::handlerAcceptFromClient(const asio::error_code &_error) -> void
{
	if (!_error) {
		connectServerNotify(m_tcp_endpoint.address().to_string());

		getBuffer();
		if (m_currentBuffer != nullptr) {
			std::lock_guard lock(m_mtx);
			if (m_tcp_socket) {
				auto buff = m_currentBuffer->getNextDMABufferForWrite();
				auto sizeLeft = buff->getWriteSizeLeft();
				auto offset = buff->getBufferFullLenght() - sizeLeft;
				m_tcp_socket->async_receive(asio::buffer((char *) buff->getMappedMemory() + offset, sizeLeft),
											std::bind(&CAsioSocketDMA::handlerReceive, this, std::placeholders::_1, std::placeholders::_2));
			}
		}
	} else if (_error.value() != 1) // Already open connection
	{
		errorServerNotify(_error);
		std::lock_guard lock(m_mtx);
		if (m_tcp_acceptor)
			m_tcp_acceptor->async_accept(*m_tcp_socket,
										 m_tcp_endpoint,
										 std::bind(&CAsioSocketDMA::handlerAcceptFromClient, this, std::placeholders::_1));
	}
}

auto CAsioSocketDMA::initClient() -> void
{
	std::lock_guard lock(m_mtx);

	m_tcp_socket = std::make_shared<asio::ip::tcp::socket>(m_asio->getIO());
	asio::ip::tcp::resolver resolver(m_asio->getIO());
	asio::ip::tcp::resolver::query query(m_host, std::to_string(m_port));
	asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
	m_tcp_endpoint = *iter;
	m_tcp_socket->async_connect(m_tcp_endpoint, std::bind(&CAsioSocketDMA::handlerConnectToServer, this, std::placeholders::_1, iter));

	m_mode = net_lib::EMode::M_CLIENT;
}

void CAsioSocketDMA::handlerConnectToServer(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator)
{
	try {
		if (!_error) {
			connectClientNotify(m_tcp_endpoint.address().to_string());
			getBuffer();
			std::lock_guard lock(m_mtx);
			if (m_tcp_socket && m_currentBuffer) {
				m_tcp_socket->set_option(asio::ip::tcp::no_delay(true));
#ifndef _WIN32
				using quickack = asio::detail::socket_option::boolean<IPPROTO_TCP, TCP_QUICKACK>;
				m_tcp_socket->set_option(quickack(true));
#endif
				asio::socket_base::receive_buffer_size optionSize(m_bufferSize * 3);
				m_tcp_socket->set_option(optionSize);

				auto buff = m_currentBuffer->getNextDMABufferForWrite();
				if (buff == nullptr) {
					FATAL("Buffer is null")
				}
				auto sizeLeft = buff->getWriteSizeLeft();
				auto offset = buff->getBufferFullLenght() - sizeLeft;
				if (m_currentBuffer != nullptr) {
					m_tcp_socket
						->async_receive(asio::buffer((char *) buff->getMappedMemory() + offset, sizeLeft),
										std::bind(&CAsioSocketDMA::handlerReceive, this, std::placeholders::_1, std::placeholders::_2));
				}
			}
		} else if (endpoint_iterator != asio::ip::tcp::resolver::iterator()) {
			std::lock_guard lock(m_mtx);
			if (m_tcp_socket) {
				m_tcp_endpoint = *endpoint_iterator;
				m_tcp_socket
					->async_connect(m_tcp_endpoint,
									std::bind(&CAsioSocketDMA::handlerConnectToServer, this, std::placeholders::_1, ++endpoint_iterator));
			}
		} else {
			errorClientNotify(_error);
		}
	} catch (...) {
		aprintf(stderr, "Error connect to server\n");
		errorClientNotify(_error);
	}
}

auto CAsioSocketDMA::stopReceive() -> void
{
	m_isStopReceive = true;
}

auto CAsioSocketDMA::getBuffer() -> void
{
	if (m_buffers == nullptr)
		return;
	std::lock_guard lock(m_mtx);
	while (!m_isStopReceive && m_currentBuffer == nullptr) {
		m_currentBuffer = m_buffers->writeBuffer(true);
		if (m_currentBuffer)
			m_currentBuffer->resetWriteSizeAll();
	}
}

auto CAsioSocketDMA::unlockBuffer() -> void
{
	if (m_buffers == nullptr)
		return;
	std::lock_guard lock(m_mtx);
	if (m_currentBuffer != nullptr) {
		m_buffers->unlockBufferWrite();
		m_currentBuffer = nullptr;
	}
}

auto CAsioSocketDMA::isConnected() -> bool
{
	if (m_tcp_socket) {
		return m_tcp_socket->is_open();
	}
	return false;
}

auto CAsioSocketDMA::cancelSocket() -> void
{
	try {
		if (m_tcp_acceptor) {
			m_tcp_acceptor->cancel();
			m_tcp_socket->shutdown(asio::socket_base::shutdown_both);
		}
		if (m_tcp_socket) {
			m_tcp_socket->cancel();
			m_tcp_socket->shutdown(asio::socket_base::shutdown_both);
		}
	} catch (...) {
	}
}

auto CAsioSocketDMA::closeSocket() -> void
{
	try {
		if (m_tcp_socket) {
			m_tcp_socket->shutdown(asio::socket_base::shutdown_both);
		}
	} catch (...) {
	}

	std::lock_guard lock(m_mtx);
	bool emitTCP = false;
	try {
		if (m_tcp_socket) {
			emitTCP = true;
			m_tcp_acceptor = nullptr;
			m_tcp_socket = nullptr;
		}
	} catch (...) {
	}

	try {
		if (emitTCP) {
			disconnectServerNotify(m_tcp_endpoint.address().to_string());
		}
	} catch (...) {
	}
}

auto CAsioSocketDMA::handlerReceive(const asio::error_code &ErrorCode, size_t bytes_transferred) -> void
{
	// Operation aborted
	if (ErrorCode.value() == 125) {
		return;
	}
	if (!ErrorCode) {
		if (m_currentBuffer) {
			auto b = m_currentBuffer->getNextDMABufferForWrite();
			if (b) {
				b->addWriteSize(bytes_transferred);
			}
			if (m_currentBuffer->isAllDataWrite()) {
				m_currentBuffer->verifyPack();
				recivedNotify(ErrorCode, m_currentBuffer);
				unlockBuffer();
				getBuffer();
			}
		} else {
			FATAL("There is no buffer for writing.")
		}

		std::lock_guard lock(m_mtx);
		if (m_tcp_socket && m_currentBuffer) {
			auto buff = m_currentBuffer->getNextDMABufferForWrite();
			if (buff == nullptr) {
				FATAL("Buffer is null")
			}
			auto sizeLeft = buff->getWriteSizeLeft();
			auto offset = buff->getBufferFullLenght() - sizeLeft;
			m_tcp_socket->async_receive(asio::buffer((char*)buff->getMappedMemory() + offset, sizeLeft),
										std::bind(&CAsioSocketDMA::handlerReceive, this, std::placeholders::_1, std::placeholders::_2));
		}
	} else {
		errorClientNotify(ErrorCode);
		closeSocket();
	}
}

auto CAsioSocketDMA::sendSyncBuffer(DataLib::CDataBuffersPackDMA::Ptr _buffer) -> bool
{
	std::lock_guard lock(m_mtx);
	asio::error_code _error;

	if (m_tcp_socket) {
		for (auto i = (int) DataLib::EDataBuffersPackChannel::CH1; i <= (int) DataLib::EDataBuffersPackChannel::CH4; i++) {
			auto buff = _buffer->getBuffer((DataLib::EDataBuffersPackChannel) i);
			if (buff != NULL) {
				uint32_t needSendSize = buff->getBufferFullLenght();
				uint32_t offset = 0;
				while (needSendSize) {
					uint32_t sendedSize = (uint32_t) m_tcp_socket->send(asio::buffer((char *) buff->getMappedMemory() + offset,
																					 needSendSize),
																		0,
																		_error);
					offset += sendedSize;
					needSendSize -= sendedSize;
					if (_error.value() != 0) {
						break;
					}
				}
			}
		}
		this->handlerSend(_error, _buffer->getLenghtBuffers());
		return true;
	}
	return false;
}

void CAsioSocketDMA::handlerSend(const asio::error_code &_error, size_t _bytesTransferred)
{
	sendNotify(_error, _bytesTransferred);
	if (!_error) {
		//  MODIFY LATER
	} else {
		//  closeSocket();
	}
}
