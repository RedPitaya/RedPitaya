#ifndef NET_LIB_ASIO_SOCKET_DMA_H
#define NET_LIB_ASIO_SOCKET_DMA_H

#include <atomic>
#include <mutex>
#include "asio.hpp"
#include "asio_common.h"
#include "asio_service.h"
#include "data_lib/buffers_cached.h"
#include "data_lib/signal.hpp"

using namespace DataLib;

namespace net_lib {

// The data channel of one streaming session: one peer, raw packs of fixed-size channel buffers.
// Flow control is the blocking: a sender waits in send while the peer does not read, a receiver
// waits for a free buffer of its ring before it reads on. Both can last long and are normal.
//
// A server takes the first client of the session only; later connections are closed at once.
// A peer that leaves is closed and reported with the disconnect signal of the side. An endpoint
// without a receive ring (the ADC server, the DAC client) keeps a read pending only to see the
// peer close.
class CAsioSocketDMA {
   public:
    using Ptr = std::shared_ptr<CAsioSocketDMA>;

    static Ptr create(std::string host, uint16_t port, CBuffersCached::Ptr buffers);

    CAsioSocketDMA(std::string host, uint16_t port, CBuffersCached::Ptr buffers);
    ~CAsioSocketDMA();

    auto initServer() -> void;
    auto initClient() -> void;
    // Stops taking new buffers; the pack being received is finished.
    auto stopReceive() -> void;
    auto cancelSocket() -> void;
    // Callable from any thread; releases a sender blocked on a peer that does not read.
    auto closeSocket() -> void;
    auto isConnected() -> bool;
    // Sends every channel buffer of the pack; false if the connection broke (it is then closed).
    auto sendSyncBuffer(DataLib::CDataBuffersPackDMA::Ptr _buffer) -> bool;

    sigslot::signal<std::string&> connectServerNotify;
    sigslot::signal<std::string&> disconnectServerNotify;

    sigslot::signal<std::string&> connectClientNotify;
    sigslot::signal<std::string&> disconnectClientNotify;

    sigslot::signal<std::error_code> errorServerNotify;
    sigslot::signal<std::error_code> errorClientNotify;

    sigslot::signal<std::error_code, size_t> sendNotify;
    // Emitted on the network thread; the pack is reused as soon as the handler returns.
    sigslot::signal<std::error_code, DataLib::CDataBuffersPackDMA::Ptr> recivedNotify;

   private:
    CAsioSocketDMA(const CAsioSocketDMA&) = delete;
    CAsioSocketDMA(CAsioSocketDMA&&) = delete;
    CAsioSocketDMA& operator=(const CAsioSocketDMA&) = delete;
    CAsioSocketDMA& operator=(const CAsioSocketDMA&&) = delete;

    using socket_ptr = std::shared_ptr<asio::ip::tcp::socket>;

    auto startAccept() -> void;
    auto handlerAccept(const asio::error_code& _error, socket_ptr socket) -> void;
    auto handlerConnect(const asio::error_code& _error, socket_ptr socket) -> void;
    auto startReceive(socket_ptr socket) -> void;
    auto handlerReceive(const asio::error_code& _error, size_t bytes_transferred, socket_ptr socket) -> void;
    auto startWatch(socket_ptr socket) -> void;
    auto handlerSend(const asio::error_code& _error, size_t _bytesTransferred) -> void;
    // Closes the connection if it is still this socket and emits the disconnect signal once.
    auto dropConnection(socket_ptr socket) -> void;
    auto emitError(const asio::error_code& _error) -> void;
    auto emitDisconnect(std::string peer) -> void;

    // Network thread only.
    auto getBuffer() -> void;
    auto unlockBuffer() -> void;

    net_lib::EMode m_mode;
    std::string m_host;
    uint16_t m_port;

    // Guarded by m_mtx.
    socket_ptr m_tcp_socket;
    std::shared_ptr<asio::ip::tcp::acceptor> m_tcp_acceptor;
    bool m_connected = false;
    bool m_sessionTaken = false;
    std::string m_peer;

    std::mutex m_mtx;
    // One sender at a time on the socket.
    std::mutex m_sendMtx;
    CAsioService* m_asio;
    uint64_t m_bufferSize;
    CBuffersCached::Ptr m_buffers;
    CDataBuffersPackDMA::Ptr m_currentBuffer;
    std::atomic<bool> m_isStopReceive;
    uint8_t m_watchBuffer[256];
};

}  // namespace net_lib

#endif
