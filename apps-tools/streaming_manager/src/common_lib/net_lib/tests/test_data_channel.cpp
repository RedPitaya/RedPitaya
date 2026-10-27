/**
 * The streaming data channel (CAsioNet over CAsioSocketDMA) as the applications use it.
 *
 * ADC: the board is the server and sends packs from its thread (CStreamingNet), a client
 * receives them into a ring whose buffers it prepared from the block size and the channel list
 * it got over the config channel. DAC: the client sends, the board receives into its ring and
 * the generator takes the packs at its own pace.
 *
 * The packs are raw channel buffers of a fixed size, the blocking of send and of the wait for
 * a free ring buffer is the flow control: a slow side slows the other down, nothing is lost.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "asio.hpp"
#include "data_lib/buffers_cached.h"
#include "data_lib/network_header.h"
#include "net_lib/asio_net.h"
#include "streaming_lib/streaming_net.h"

using namespace std::chrono_literals;
using DataLib::CDataBufferDMA;
using DataLib::CDataBuffersPackDMA;

namespace {

constexpr size_t kBlock = 16 * 1024;

auto WaitFor(const std::function<bool()>& done, std::chrono::milliseconds timeout = 5000ms) -> bool {
    const auto end = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < end) {
        if (done())
            return true;
        std::this_thread::sleep_for(2ms);
    }
    return done();
}

auto NextPort() -> uint16_t {
    static std::atomic<uint16_t> port{27500};
    return port++;
}

// Heap memory cut into regions shaped like the DMA regions of the memory manager.
class Arena {
   public:
    Arena(size_t count, size_t block) : m_storage(new uint8_t[count * block]()) {
        for (size_t i = 0; i < count; i++) {
            uio_lib::MemoryRegionT r;
            r.start = static_cast<uint32_t>(0x1E000000u + i * block);
            r.end = static_cast<uint32_t>(r.start + block);
            r.startMemory = m_storage.get() + i * block;
            r.size = block;
            m_regions.push_back(r);
        }
    }
    auto regions() const -> const std::vector<uio_lib::MemoryRegionT>& { return m_regions; }

   private:
    std::unique_ptr<uint8_t[]> m_storage;
    std::vector<uio_lib::MemoryRegionT> m_regions;
};

auto Fill(CDataBufferDMA::Ptr b, uint64_t id, int ch) -> void {
    auto* data = static_cast<uint8_t*>(b->getMappedDataMemory());
    for (size_t k = 0; k < b->getDataLenght(); k++)
        data[k] = static_cast<uint8_t>(id * 7 + ch * 31 + k);
}

auto Check(CDataBufferDMA::Ptr b, uint64_t id, int ch) -> bool {
    auto* data = static_cast<uint8_t*>(b->getMappedDataMemory());
    for (size_t k = 0; k < b->getDataLenght(); k++)
        if (data[k] != static_cast<uint8_t>(id * 7 + ch * 31 + k))
            return false;
    return true;
}

// The sending side's packs: channel buffers with an initialised header, like the board's ADC
// ring after initHeadersADC or the client's DAC ring after initHeadersDAC.
struct SenderPacks {
    Arena arena;
    std::vector<CDataBuffersPackDMA::Ptr> packs;

    SenderPacks(int count, bool adc) : arena(count * 2, kBlock) {
        size_t r = 0;
        for (int i = 0; i < count; i++) {
            auto pack = CDataBuffersPackDMA::Create();
            for (int ch = 0; ch < 2; ch++) {
                const auto& reg = arena.regions()[r++];
                auto b = CDataBufferDMA::Create(reg.start, reg.size, reg.startMemory, 16);
                b->initHeaderAddress(DataLib::sizeHeader());
                if (adc)
                    DataLib::initHeaderADC(b, 125000000, 14, 2 * kBlock, ch);
                else
                    DataLib::initHeaderDAC(b, 2 * kBlock, 2);
                pack->addBuffer(static_cast<DataLib::EDataBuffersPackChannel>(ch), b);
            }
            packs.push_back(pack);
        }
    }
    // Pack `id` of the stream, in ring slot id % size.
    auto prepare(uint64_t id) -> CDataBuffersPackDMA::Ptr {
        auto pack = packs[id % packs.size()];
        for (int ch = 0; ch < 2; ch++) {
            auto b = pack->getBuffer(static_cast<DataLib::EDataBuffersPackChannel>(ch));
            Fill(b, id, ch);
            DataLib::setHeaderADC(b, id);
        }
        return pack;
    }
};

// The receiving ring of a client (ADC) or of the board (DAC).
struct ReceiverRing {
    Arena arena;
    DataLib::CBuffersCached::Ptr ring = DataLib::CBuffersCached::create();
    ReceiverRing(int packs, bool adc) : arena(packs * 2, kBlock) {
        if (adc) {
            adc_channels_t ch;
            ch.enable(ADCChannels::ADC_CH1);
            ch.enable(ADCChannels::ADC_CH2);
            ring->generateBuffersEmptyADC(ch, arena.regions(), DataLib::sizeHeader());
        } else {
            dac_channels_t ch;
            ch.enable(DACChannels::DAC_CH1);
            ch.enable(DACChannels::DAC_CH2);
            ring->generateBuffersEmptyDAC(ch, arena.regions(), DataLib::sizeHeader());
        }
    }
};

// A client of the ADC stream as rpsa_client builds it: the handler checks the pack and hands
// the buffer back before it returns.
struct AdcClient {
    ReceiverRing rx{8, true};
    net_lib::CAsioNet::Ptr net;
    std::atomic<int> connects{0}, disconnects{0}, errors{0}, packs{0}, broken{0};
    std::atomic<int> delayMs{0};
    uint64_t next = 0;

    explicit AdcClient(uint16_t port) {
        net = net_lib::CAsioNet::create(net_lib::M_CLIENT, "127.0.0.1", port, rx.ring);
        net->clientConnectNotify.connect([this](std::string&) { connects++; });
        net->clientDisconnectNotify.connect([this](std::string&) { disconnects++; });
        net->clientErrorNotify.connect([this](std::error_code) { errors++; });
        net->reciveNotify.connect([this](std::error_code e, CDataBuffersPackDMA::Ptr pack) {
            if (e)
                return;
            pack->getInfoFromHeaderADC();
            for (int ch = 0; ch < 2; ch++) {
                auto b = pack->getBuffer(static_cast<DataLib::EDataBuffersPackChannel>(ch));
                if (!b || b->getADCPackId() != next || !Check(b, next, ch))
                    broken++;
            }
            next++;
            if (delayMs)
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            rx.ring->unlockBufferRead();
            packs++;
        });
    }
};

struct AdcServer {
    SenderPacks tx{4, true};
    net_lib::CAsioNet::Ptr net;
    std::atomic<int> connects{0}, disconnects{0}, sendErrors{0};

    explicit AdcServer(uint16_t port) {
        net = net_lib::CAsioNet::create(net_lib::M_SERVER, "127.0.0.1", port, nullptr);
        net->serverConnectNotify.connect([this](std::string&) { connects++; });
        net->serverDisconnectNotify.connect([this](std::string&) { disconnects++; });
        net->sendNotify.connect([this](std::error_code e, size_t) {
            if (e)
                sendErrors++;
        });
        net->start();
    }
};

// A raw TCP peer.
struct RawPeer {
    asio::io_context io;
    asio::ip::tcp::socket s{io};
    auto connect(uint16_t port) -> bool {
        asio::error_code ec;
        s.connect({asio::ip::make_address("127.0.0.1"), port}, ec);
        return !ec;
    }
    // True if the peer closed within the time.
    auto closedWithin(std::chrono::milliseconds t) -> bool {
        s.non_blocking(true);
        const auto end = std::chrono::steady_clock::now() + t;
        char buf[4096];
        while (std::chrono::steady_clock::now() < end) {
            asio::error_code ec;
            s.read_some(asio::buffer(buf), ec);
            if (ec && ec != asio::error::would_block)
                return true;
            std::this_thread::sleep_for(5ms);
        }
        return false;
    }
};

}  // namespace

// ---- ADC: board sends, client receives ----

TEST(DataChannel, AdcPacksArriveWholeAndInOrder) {
    const uint16_t port = NextPort();
    AdcServer server(port);
    AdcClient client(port);
    client.net->start();
    ASSERT_TRUE(WaitFor([&] { return server.connects == 1 && client.connects == 1; }));
    for (uint64_t id = 0; id < 100; id++)
        ASSERT_TRUE(server.net->sendSyncData(server.tx.prepare(id)));
    ASSERT_TRUE(WaitFor([&] { return client.packs == 100; }));
    EXPECT_EQ(client.broken, 0);
    EXPECT_EQ(server.sendErrors, 0);
}

// A client slower than the stream is not an error: the server waits in send, nothing is lost.
TEST(DataChannel, ASlowClientSlowsTheServerDownWithoutLoss) {
    const uint16_t port = NextPort();
    AdcServer server(port);
    AdcClient client(port);
    client.delayMs = 15;
    client.net->start();
    ASSERT_TRUE(WaitFor([&] { return server.connects == 1; }));
    const auto start = std::chrono::steady_clock::now();
    for (uint64_t id = 0; id < 60; id++)
        ASSERT_TRUE(server.net->sendSyncData(server.tx.prepare(id)));
    ASSERT_TRUE(WaitFor([&] { return client.packs == 60; }));
    EXPECT_EQ(client.broken, 0);
    EXPECT_EQ(server.sendErrors, 0);
    EXPECT_GT(std::chrono::steady_clock::now() - start, 300ms) << "the sender was held back by the client";
}

// Qt creates its ring in the connect handler; the first buffer is taken after it.
TEST(DataChannel, AClientMayCreateItsRingInTheConnectHandler) {
    const uint16_t port = NextPort();
    AdcServer server(port);
    Arena arena(16, kBlock);
    auto ring = DataLib::CBuffersCached::create();
    std::atomic<int> packs{0};
    auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, "127.0.0.1", port, ring);
    net->clientConnectNotify.connect([&](std::string&) {
        adc_channels_t ch;
        ch.enable(ADCChannels::ADC_CH1);
        ch.enable(ADCChannels::ADC_CH2);
        ring->generateBuffersEmptyADC(ch, arena.regions(), DataLib::sizeHeader());
    });
    net->reciveNotify.connect([&](std::error_code, CDataBuffersPackDMA::Ptr) {
        ring->unlockBufferRead();
        packs++;
    });
    net->start();
    ASSERT_TRUE(WaitFor([&] { return server.connects == 1; }));
    for (uint64_t id = 0; id < 10; id++)
        ASSERT_TRUE(server.net->sendSyncData(server.tx.prepare(id)));
    EXPECT_TRUE(WaitFor([&] { return packs == 10; }));
}

// The server used to keep a client that left as connected and fail every send after it.
TEST(DataChannel, TheServerSeesTheClientLeave) {
    const uint16_t port = NextPort();
    AdcServer server(port);
    {
        RawPeer peer;
        ASSERT_TRUE(peer.connect(port));
        ASSERT_TRUE(WaitFor([&] { return server.net->isConnected(); }));
    }
    ASSERT_TRUE(WaitFor([&] { return server.disconnects == 1; }));
    EXPECT_FALSE(server.net->isConnected());
    EXPECT_FALSE(server.net->sendSyncData(server.tx.prepare(0)));
}

// One client per session: a later connection is closed instead of left without data.
TEST(DataChannel, ALaterClientOfTheSessionIsClosed) {
    const uint16_t port = NextPort();
    AdcServer server(port);
    AdcClient client(port);
    client.net->start();
    ASSERT_TRUE(WaitFor([&] { return server.connects == 1; }));
    RawPeer late;
    ASSERT_TRUE(late.connect(port));
    EXPECT_TRUE(late.closedWithin(2000ms));
    ASSERT_TRUE(server.net->sendSyncData(server.tx.prepare(0)));
    EXPECT_TRUE(WaitFor([&] { return client.packs == 1; })) << "the session's client keeps its stream";
    EXPECT_EQ(server.connects, 1);
}

// The disconnect of a client used to come as the server's signal, so clients never got theirs.
TEST(DataChannel, TheClientSeesTheServerLeaveWithItsOwnSignal) {
    const uint16_t port = NextPort();
    auto server = std::make_unique<AdcServer>(port);
    AdcClient client(port);
    std::atomic<int> serverSignal{0};
    client.net->serverDisconnectNotify.connect([&](std::string&) { serverSignal++; });
    client.net->start();
    ASSERT_TRUE(WaitFor([&] { return client.connects == 1; }));
    server.reset();
    ASSERT_TRUE(WaitFor([&] { return client.disconnects == 1; }));
    EXPECT_GE(client.errors, 1) << "rpsa_client stops on the error it gets when the board closes";
    EXPECT_EQ(serverSignal, 0);
    EXPECT_FALSE(client.net->isConnected());
}

TEST(DataChannel, StoppingAClientIsNotAnError) {
    const uint16_t port = NextPort();
    AdcServer server(port);
    auto client = std::make_unique<AdcClient>(port);
    client->net->start();
    ASSERT_TRUE(WaitFor([&] { return client->connects == 1; }));
    client->net->stop();
    auto* c = client.get();
    c->net = nullptr;
    EXPECT_EQ(c->errors, 0);
}

TEST(DataChannel, ARefusedOrUnresolvableServerIsAnErrorNotAnException) {
    for (const char* host : {"127.0.0.1", "no-such-host.invalid"}) {
        ReceiverRing rx(4, true);
        std::atomic<int> errors{0};
        auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, host, NextPort(), rx.ring);
        net->clientErrorNotify.connect([&](std::error_code) { errors++; });
        EXPECT_NO_THROW(net->start()) << host;
        EXPECT_TRUE(WaitFor([&] { return errors == 1; }, 10000ms)) << host;
        EXPECT_FALSE(net->isConnected()) << host;
    }
}

// ---- stopping with a client that does not read ----

TEST(DataChannel, DisconnectReleasesASenderBlockedOnAClientThatDoesNotRead) {
    const uint16_t port = NextPort();
    AdcServer server(port);
    RawPeer peer;  // never reads
    ASSERT_TRUE(peer.connect(port));
    ASSERT_TRUE(WaitFor([&] { return server.net->isConnected(); }));
    auto sender = std::async(std::launch::async, [&] {
        for (uint64_t id = 0;; id++)
            if (!server.net->sendSyncData(server.tx.prepare(id)))
                return id;
    });
    ASSERT_EQ(sender.wait_for(500ms), std::future_status::timeout) << "the sender is blocked";
    server.net->disconnect();
    EXPECT_EQ(sender.wait_for(2s), std::future_status::ready);
}

// streaming-server stops the ADC stream through CStreamingNet::stop(); it used to wait for the
// blocked sender before closing the socket and never returned.
TEST(DataChannel, StreamingNetStopsWithAClientThatDoesNotRead) {
    const uint16_t port = NextPort();
    std::string host = "127.0.0.1";
    auto net = streaming_lib::CStreamingNet::create(host, port);
    SenderPacks tx(2, true);
    std::atomic<uint64_t> id{0};
    net->getBuffer = [&]() { return tx.prepare(id++); };
    net->unlockBufferF = []() {};
    net->run();
    RawPeer peer;  // never reads
    ASSERT_TRUE(peer.connect(port));
    std::this_thread::sleep_for(1s);
    auto stopped = std::async(std::launch::async, [&] { net->stop(); });
    EXPECT_EQ(stopped.wait_for(3s), std::future_status::ready);
    if (stopped.wait_for(0s) != std::future_status::ready)
        std::_Exit(1);  // the stop is stuck for good; do not hang the test run
}

// ---- DAC: client sends, the board receives and plays at its own pace ----

TEST(DataChannel, DacPacksReachABoardThatPlaysSlowly) {
    const uint16_t port = NextPort();
    ReceiverRing board(4, false);
    auto server = net_lib::CAsioNet::create(net_lib::M_SERVER, "127.0.0.1", port, board.ring);
    server->start();
    // The generator side: takes packs from the ring at its own pace.
    std::atomic<bool> play{true};
    std::atomic<int> played{0}, broken{0};
    std::thread generator([&] {
        uint64_t next = 0;
        while (play) {
            auto pack = board.ring->readBuffer();
            if (!pack)
                continue;
            for (int ch = 0; ch < 2; ch++)
                if (!Check(pack->getBuffer(static_cast<DataLib::EDataBuffersPackChannel>(ch)), next, ch))
                    broken++;
            next++;
            std::this_thread::sleep_for(20ms);
            board.ring->unlockBufferRead();
            played++;
        }
    });

    SenderPacks tx(2, false);
    std::atomic<int> connects{0}, sendErrors{0};
    auto client = net_lib::CAsioNet::create(net_lib::M_CLIENT, "127.0.0.1", port, nullptr);
    client->clientConnectNotify.connect([&](std::string&) { connects++; });
    client->sendNotify.connect([&](std::error_code e, size_t) {
        if (e)
            sendErrors++;
    });
    client->start();
    ASSERT_TRUE(WaitFor([&] { return connects == 1; }));
    for (uint64_t id = 0; id < 30; id++)
        ASSERT_TRUE(client->sendSyncData(tx.prepare(id)));
    EXPECT_TRUE(WaitFor([&] { return played == 30; }));
    EXPECT_EQ(broken, 0);
    EXPECT_EQ(sendErrors, 0);
    play = false;
    generator.join();
}

// rpsa_client stops its DAC loop on the send error it gets once the board is gone.
TEST(DataChannel, ADacClientGetsASendErrorAndADisconnectWhenTheBoardIsGone) {
    const uint16_t port = NextPort();
    ReceiverRing board(4, false);
    auto server = net_lib::CAsioNet::create(net_lib::M_SERVER, "127.0.0.1", port, board.ring);
    server->start();
    SenderPacks tx(2, false);
    std::atomic<int> connects{0}, disconnects{0}, sendErrors{0};
    auto client = net_lib::CAsioNet::create(net_lib::M_CLIENT, "127.0.0.1", port, nullptr);
    client->clientConnectNotify.connect([&](std::string&) { connects++; });
    client->clientDisconnectNotify.connect([&](std::string&) { disconnects++; });
    client->sendNotify.connect([&](std::error_code e, size_t) {
        if (e)
            sendErrors++;
    });
    client->start();
    ASSERT_TRUE(WaitFor([&] { return connects == 1; }));
    server = nullptr;
    ASSERT_TRUE(WaitFor([&] { return disconnects == 1; }));
    EXPECT_FALSE(client->sendSyncData(tx.prepare(0)));
    EXPECT_GE(sendErrors, 1);
}

// ---- the size check stays a protection ----

// The ring of the receiver is cut from the settings the config channel gave; a pack of another
// size means the two sides disagree, and the process ends, as for a broken config packet.
TEST(DataChannelDeathTest, APackOfAnotherSizeEndsTheReceiver) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(
        {
            const uint16_t port = 27490;
            AdcServer server(port);
            Arena arena(8, kBlock / 2);  // half the block size of the server
            auto ring = DataLib::CBuffersCached::create();
            adc_channels_t ch;
            ch.enable(ADCChannels::ADC_CH1);
            ch.enable(ADCChannels::ADC_CH2);
            ring->generateBuffersEmptyADC(ch, arena.regions(), DataLib::sizeHeader());
            auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, "127.0.0.1", port, ring);
            net->start();
            WaitFor([&] { return server.connects == 1; });
            server.net->sendSyncData(server.tx.prepare(0));
            std::this_thread::sleep_for(2s);
            std::_Exit(0);
        },
        "not the correct size");
}
