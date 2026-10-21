#include "asio_service.h"
#include <functional>

using namespace net_lib;

CAsioService::CAsioService() : m_Ios(), m_asio_th(nullptr) {
    m_asio_th = new std::thread([this]() {
        asio::executor_work_guard<asio::io_context::executor_type> m_work = asio::make_work_guard(this->m_Ios);
        this->m_Ios.run();
    });
}

CAsioService::~CAsioService() {
    stop();
}

auto CAsioService::stop() -> void {
    m_Ios.stop();
    if (m_asio_th) {
        if (m_asio_th->get_id() == std::this_thread::get_id()) {
            m_asio_th->detach();
        } else {
            m_asio_th->join();
        }
        delete m_asio_th;
        m_asio_th = nullptr;
    }
}

auto CAsioService::runningInThisThread() -> bool {
    return m_Ios.get_executor().running_in_this_thread();
}

auto CAsioService::getIO() -> asio::io_context& {
    return m_Ios;
}
