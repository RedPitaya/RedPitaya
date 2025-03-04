#include "asio_service.h"
#include <functional>

using namespace net_lib;

CAsioService::CAsioService()
    : m_Ios()
      // , m_Work(m_Ios)
      ,
      m_asio_th(nullptr) {
    asio::executor_work_guard<asio::io_context::executor_type> work = asio::make_work_guard(m_Ios);
    auto func = std::bind(static_cast<size_t (asio::io_context::*)()>(&asio::io_context::run), &(m_Ios));
    m_asio_th = new std::thread(func);
}

CAsioService::~CAsioService() {
    m_Ios.stop();
    if (m_asio_th) {
        m_asio_th->join();
        delete m_asio_th;
        m_asio_th = nullptr;
    }
}

auto CAsioService::getIO() -> asio::io_context& {
    return m_Ios;
}
