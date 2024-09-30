#include <functional>
#include "asio_service.h"

CAsioService::CAsioService()
	: m_Ios()
	, m_Work(m_Ios)
	, m_asio_th(nullptr)
{
	auto func = std::bind(static_cast<size_t (asio::io_service::*)()>(&asio::io_service::run), &(m_Ios));
	m_asio_th = new std::thread(func);
}

CAsioService::~CAsioService()
{
	m_Ios.reset();
	m_Ios.stop();
	if (m_asio_th) {
		m_asio_th->join();
		delete m_asio_th;
		m_asio_th = nullptr;
	}
}

auto CAsioService::getIO() -> asio::io_service &
{
	return m_Ios;
}
