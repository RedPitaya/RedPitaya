#ifndef BROADCAST_LIB_ASIO_BROADCAST_SOCKET_H
#define BROADCAST_LIB_ASIO_BROADCAST_SOCKET_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "data_lib/signal.hpp"

namespace broadcast_lib {

enum EMode { AB_SERVER_MASTER = 0, AB_SERVER_SLAVE = 1, AB_CLIENT = 2, AB_NONE = 3 };

enum EEvents { AB_ERROR = 0, AB_SEND_DATA = 1, AB_RECIVED_DATA = 2 };

// Where a server announces itself: the address put in the message and the destination of the
// datagram (the directed broadcast address of that interface).
struct BroadcastTarget {
    std::string address;  // empty: the host given to the constructor or setIPAddress
    std::string destination;
};

// The server side sends its announcement every period on every IPv4 interface that is up and
// can broadcast, each with that interface's own address, so a new or changed address is
// announced at the next period. Without such an interface it falls back to 255.255.255.255
// with the configured host.
//
// Every socket operation runs on the object's asio thread; the public methods may be called
// from any thread, and closeSocket() returns once nothing is sent or received any more.
class CAsioBroadcastSocket {
   public:
    typedef uint8_t* ab_buffer;

    using Ptr = std::shared_ptr<CAsioBroadcastSocket>;
    using TargetProvider = std::function<std::vector<BroadcastTarget>()>;

    static auto create(uint8_t model, std::string mac, std::string host, uint16_t port) -> Ptr;
    CAsioBroadcastSocket(uint8_t model, std::string mac, std::string host, uint16_t port);
    ~CAsioBroadcastSocket();

    auto initServer(broadcast_lib::EMode mode, int sleep_time_ms = 1000) -> void;
    auto initClient() -> void;
    auto closeSocket() -> void;
    // The host announced when no interface is found.
    auto setIPAddress(std::string host) -> void;
    // Replaces the interface list, for tests and setups that announce something else.
    auto setTargetProvider(TargetProvider provider) -> void;

    // IPv4 interfaces that are up, not loopback and can broadcast.
    static auto systemTargets() -> std::vector<BroadcastTarget>;

    // An error is reported once; the same error again is not reported until a send succeeded.
    sigslot::signal<std::error_code> errorNotify;
    sigslot::signal<std::error_code, size_t> sendNotify;
    sigslot::signal<std::error_code, uint8_t*, size_t> receivedNotify;

   private:
    CAsioBroadcastSocket(const CAsioBroadcastSocket&) = delete;
    CAsioBroadcastSocket(CAsioBroadcastSocket&&) = delete;

    auto handlerReceive(const std::error_code& error, size_t bytes_transferred) -> void;
    auto handlerAccept(const std::error_code& _error) -> void;
    auto handlerSend(const std::error_code& _error, size_t _bytesTransferred) -> void;
    auto handlerSend2(const std::error_code& _error, size_t _bytesTransferred, uint8_t* buffer) -> void;
    auto handlerReceiveFromServer(const std::error_code& ErrorCode, size_t bytes_transferred) -> void;

    int m_sleep_time_ms;

    uint8_t* m_SocketReadBuffer;

    class Impl;
    std::unique_ptr<Impl> m_pimpl;
};

}  // namespace broadcast_lib

#endif
