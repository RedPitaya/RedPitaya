#include <functional>
#include <future>
#include <map>
#include <thread>

#include "asio.hpp"
#include "asio_broadcast_socket.h"
#include "logger_lib/file_logger.h"

#ifndef _WIN32
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#endif

#define SOCKET_BUFFER_SIZE 1024

using namespace broadcast_lib;

class CAsioBroadcastSocket::Impl {
   public:
    // First member: destroyed last, after the timer and the sockets that belong to it.
    asio::io_context m_Ios;
    std::thread* m_asio_th = nullptr;
    std::unique_ptr<asio::steady_timer> m_timer;
    std::shared_ptr<asio::ip::udp::socket> m_socket;

    // Everything below is used on the asio thread only.
    uint8_t m_model = 0;
    broadcast_lib::EMode m_mode = EMode::AB_NONE;
    std::string m_mac = "";
    std::string m_host;
    uint16_t m_port = 0;
    bool m_stopped = true;
    TargetProvider m_provider;
    // The last error per destination, so a lasting one is reported once.
    std::map<std::string, std::error_code> m_errors;

    auto genString(const std::string& host) -> std::string {
        std::string ret = "";
        ret += m_mac + "\n";
        ret += host + "\n";
        switch (m_mode) {
            case EMode::AB_SERVER_MASTER:
                ret += "M\n";
                break;
            case EMode::AB_SERVER_SLAVE:
                ret += "S\n";
                break;
            case EMode::AB_CLIENT:
                ret += "C\n";
                break;
            case EMode::AB_NONE:
                ret += "N\n";
                break;
            default:
                ret += "E\n";
                break;
        }
        ret += std::to_string(m_model) + "\n";
        return ret;
    };

    // Runs _fn on the asio thread and waits for it.
    auto run(const std::function<void()>& _fn) -> void {
        if (m_Ios.get_executor().running_in_this_thread()) {
            _fn();
            return;
        }
        std::promise<void> done;
        auto result = done.get_future();
        asio::post(m_Ios, [&]() {
            _fn();
            done.set_value();
        });
        result.wait();
    }

    auto close() -> void {
        m_stopped = true;
        m_timer->cancel();
        if (m_socket) {
            std::error_code ignored;
            m_socket->cancel(ignored);
            m_socket->close(ignored);
            m_socket.reset();
        }
        m_errors.clear();
    }

    auto report(CAsioBroadcastSocket* _owner, const std::string& _key, const std::error_code& _error) -> void {
        auto& last = m_errors[_key];
        if (last == _error)
            return;
        last = _error;
        if (_error)
            _owner->errorNotify(_error);
    }

    auto openSender(CAsioBroadcastSocket* _owner) -> bool {
        auto socket = std::make_shared<asio::ip::udp::socket>(m_Ios);
        std::error_code error;
        socket->open(asio::ip::udp::v4(), error);
        if (!error)
            socket->set_option(asio::ip::udp::socket::reuse_address(true), error);
        if (!error)
            socket->set_option(asio::socket_base::broadcast(true), error);
        report(_owner, "socket", error);
        if (error) {
            std::error_code ignored;
            socket->close(ignored);
            return false;
        }
        m_socket = socket;
        return true;
    }

    auto schedule(CAsioBroadcastSocket* _owner) -> void {
        m_timer->expires_after(std::chrono::milliseconds(_owner->m_sleep_time_ms));
        m_timer->async_wait([this, _owner](const std::error_code& error) {
            if (error || m_stopped)
                return;
            sendRound(_owner);
        });
    }

    auto sendRound(CAsioBroadcastSocket* _owner) -> void {
        if (m_stopped)
            return;
        if (!m_socket && !openSender(_owner)) {
            schedule(_owner);
            return;
        }
        auto targets = m_provider ? m_provider() : CAsioBroadcastSocket::systemTargets();
        if (targets.empty())
            targets.push_back({"", asio::ip::address_v4::broadcast().to_string()});
        for (const auto& target : targets) {
            std::error_code error;
            auto destination = asio::ip::make_address_v4(target.destination, error);
            if (error) {
                report(_owner, target.destination, error);
                continue;
            }
            // Each datagram owns its text: nothing it points to changes while it is sent.
            auto message = std::make_shared<std::string>(genString(target.address.empty() ? m_host : target.address));
            auto socket = m_socket;
            const std::string key = target.destination;
            socket->async_send_to(asio::buffer(*message), asio::ip::udp::endpoint(destination, m_port),
                                  [this, _owner, message, socket, key](const std::error_code& e, size_t n) {
                                      if (e == asio::error::operation_aborted || m_stopped)
                                          return;
                                      _owner->sendNotify(e, n);
                                      report(_owner, key, e);
                                  });
        }
        schedule(_owner);
    }

    auto startReceive(CAsioBroadcastSocket* _owner, std::shared_ptr<asio::ip::udp::socket> _socket) -> void {
        _socket->async_receive(asio::buffer(_owner->m_SocketReadBuffer, SOCKET_BUFFER_SIZE), [this, _owner, _socket](const std::error_code& e, size_t n) {
            if (e == asio::error::operation_aborted || m_stopped || _socket != m_socket)
                return;
            if (e) {
                TRACE_SHORT("Error %s (%d)", e.message().c_str(), e.value())
                _owner->errorNotify(e);
                close();
                return;
            }
            _owner->receivedNotify(e, _owner->m_SocketReadBuffer, n);
            if (!m_stopped && _socket == m_socket)
                startReceive(_owner, _socket);
        });
    }
};

auto CAsioBroadcastSocket::create(uint8_t model, std::string mac, std::string host, uint16_t port) -> CAsioBroadcastSocket::Ptr {
    return std::make_shared<CAsioBroadcastSocket>(model, mac, host, port);
}

CAsioBroadcastSocket::CAsioBroadcastSocket(uint8_t model, std::string mac, std::string host, uint16_t port) : m_sleep_time_ms(1000), m_pimpl(new Impl()) {
    m_SocketReadBuffer = new uint8_t[SOCKET_BUFFER_SIZE];
    m_pimpl->m_model = model;
    m_pimpl->m_mac = mac;
    m_pimpl->m_host = host;
    m_pimpl->m_port = port;
    m_pimpl->m_mode = EMode::AB_NONE;
    m_pimpl->m_timer = std::make_unique<asio::steady_timer>(m_pimpl->m_Ios);
    m_pimpl->m_asio_th = new std::thread([this]() {
        asio::executor_work_guard<asio::io_context::executor_type> m_work = asio::make_work_guard(this->m_pimpl->m_Ios);
        this->m_pimpl->m_Ios.run();
    });
}

CAsioBroadcastSocket::~CAsioBroadcastSocket() {
    closeSocket();
    m_pimpl->m_Ios.stop();
    if (m_pimpl->m_asio_th) {
        if (m_pimpl->m_asio_th->get_id() == std::this_thread::get_id()) {
            m_pimpl->m_asio_th->detach();
        } else {
            m_pimpl->m_asio_th->join();
        }
        delete m_pimpl->m_asio_th;
    }
    m_pimpl->m_socket.reset();
    m_pimpl->m_timer.reset();
    delete[] m_SocketReadBuffer;
}

auto CAsioBroadcastSocket::initServer(EMode mode, int sleep_time_ms) -> void {
    m_pimpl->run([&]() {
        if (mode != EMode::AB_SERVER_MASTER && mode != EMode::AB_SERVER_SLAVE) {
            errorNotify(std::make_error_code(std::errc::invalid_argument));
            return;
        }
        m_pimpl->close();
        m_sleep_time_ms = sleep_time_ms;
        m_pimpl->m_mode = mode;
        m_pimpl->m_stopped = false;
        m_pimpl->sendRound(this);
    });
}

auto CAsioBroadcastSocket::initClient() -> void {
    m_pimpl->run([&]() {
        m_pimpl->close();
        m_pimpl->m_mode = EMode::AB_CLIENT;
        auto socket = std::make_shared<asio::ip::udp::socket>(m_pimpl->m_Ios);
        std::error_code error;
        // The options before bind: several clients on one machine share the port.
        socket->open(asio::ip::udp::v4(), error);
        if (!error)
            socket->set_option(asio::ip::udp::socket::reuse_address(true), error);
        if (!error)
            socket->set_option(asio::socket_base::broadcast(true), error);
        if (!error)
            socket->bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), m_pimpl->m_port), error);
        if (error) {
            std::error_code ignored;
            socket->close(ignored);
            errorNotify(error);
            return;
        }
        m_pimpl->m_stopped = false;
        m_pimpl->m_socket = socket;
        m_pimpl->startReceive(this, socket);
    });
}

auto CAsioBroadcastSocket::closeSocket() -> void {
    m_pimpl->run([&]() { m_pimpl->close(); });
}

auto CAsioBroadcastSocket::setIPAddress(std::string host) -> void {
    m_pimpl->run([&]() { m_pimpl->m_host = host; });
}

auto CAsioBroadcastSocket::setTargetProvider(TargetProvider provider) -> void {
    m_pimpl->run([&]() { m_pimpl->m_provider = provider; });
}

auto CAsioBroadcastSocket::systemTargets() -> std::vector<BroadcastTarget> {
    std::vector<BroadcastTarget> targets;
#ifndef _WIN32
    struct ifaddrs* list = nullptr;
    if (getifaddrs(&list) == -1)
        return targets;
    for (auto* ifa = list; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            continue;
        if (!(ifa->ifa_flags & IFF_UP) || !(ifa->ifa_flags & IFF_BROADCAST) || (ifa->ifa_flags & IFF_LOOPBACK) || !ifa->ifa_broadaddr)
            continue;
        char address[INET_ADDRSTRLEN];
        char broadcast[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, address, sizeof(address));
        inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_broadaddr)->sin_addr, broadcast, sizeof(broadcast));
        targets.push_back({address, broadcast});
    }
    freeifaddrs(list);
#endif
    return targets;
}
