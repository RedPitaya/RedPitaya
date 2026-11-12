#ifndef NET_LIB_ASIO_SOCKET_DMA_H
#define NET_LIB_ASIO_SOCKET_DMA_H

#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <mutex>
#include <utility>
#include <vector>
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

    // Called once the memory of a pack may be written again.
    using Release = std::function<void()>;
    // As above, the pack handed back through release. Without zero copy release is called before
    // the return. With zero copy (MSG_ZEROCOPY) the network controller reads the pack straight from
    // its memory after the return: release is called once the kernel reports the send complete,
    // from a later send or flushZeroCopy(). A broken connection releases every pack at once.
    auto sendSyncBuffer(DataLib::CDataBuffersPackDMA::Ptr _buffer, Release release) -> bool;
    // Zero copy for the sends with a release; the pack memory must be pinnable (the rp_resmem
    // driver mapping, not /dev/mem: the kernel refuses that one and the send copies). At most
    // maxPending packs wait for their completion, a send waits beyond that: a ring of N packs
    // needs maxPending < N, or its writer and its reader can wait for each other. Before start.
    auto setZeroCopy(bool enable, size_t maxPending) -> void;
    // Waits up to timeout for the packs in flight, then releases those left. From the sender
    // thread, before the memory of the packs goes.
    auto flushZeroCopy(std::chrono::milliseconds timeout) -> void;
    struct ZeroCopyStats {
        bool used = false;     // zero copy was on for the last connection
        uint64_t sends = 0;    // sends with MSG_ZEROCOPY
        uint64_t copied = 0;   // of them, copied by the kernel all the same
    };
    auto zeroCopyStats() -> ZeroCopyStats;
    // The kind of the packs received (ID_PACK_*, 16 bytes, static); by default a client takes
    // ADC packs and a server DAC packs. Set before start.
    auto setPackID(const uint8_t* id) -> void;

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

    // Zero copy, under m_sendMtx.
    struct Pending {
        uint32_t end;  // the pack is done when every send id below end is
        Release release;
    };
    auto zcBind(const socket_ptr& socket) -> void;
    auto zcSend(const socket_ptr& socket, const char* data, size_t size, asio::error_code& error) -> size_t;
    auto zcReap() -> void;
    auto zcWait(int ms) -> void;
    auto zcReleaseDone() -> void;
    auto zcReleaseAll() -> void;

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
    const uint8_t* m_packId = nullptr;

    bool m_zcEnabled = false;
    size_t m_zcMaxPending = 1;
    socket_ptr m_zcSocket;      // the connection the ids below belong to
    bool m_zcOn = false;        // MSG_ZEROCOPY works on m_zcSocket
    uint32_t m_zcNext = 0;      // the id of the next send
    uint32_t m_zcDone = 0;      // every id below is complete
    std::vector<std::pair<uint32_t, uint32_t>> m_zcRanges;  // completed ranges above m_zcDone
    std::deque<Pending> m_zcPending;
    ZeroCopyStats m_zcStats;
    uint8_t m_watchBuffer[256];
};

}  // namespace net_lib

#endif
