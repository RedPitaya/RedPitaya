/**
 * GPIOStreamServer, the GPIO streaming of a board over CHardware: the server of the host build runs
 * on the dummy GPIO block, its clients connect to the data ports over TCP on localhost. The input
 * buffers reach the client in order with their header, the output buffers of the client reach the
 * block in order, a pack stays out of the ring while the block holds it, and what is reported.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

#include "board_lib/board.h"
#include "data_lib/buffers_cached.h"
#include "data_lib/network_header.h"
#include "gpio_streaming_lib/gpio_stream_server.h"
#include "net_lib/asio_net.h"
#include "uio_lib/gpio.h"
#include "uio_lib/gpio_dummy.h"
#include "uio_lib/memory_manager.h"

using namespace gpio_streaming_lib;
using namespace std::chrono_literals;
using DataLib::CBuffersCached;
using DataLib::CDataBuffersPackDMA;

namespace {

constexpr uint32_t kBlock = 4096;
constexpr uint32_t kWords = kBlock / 4;

class Recorder : public GPIOServerCallback {
   public:
    void started(GPIOStreamServer*) override {
        std::lock_guard lock(mtx);
        events.push_back("started");
    }
    void stopped(GPIOStreamServer*, GPIOStopReason r) override {
        std::lock_guard lock(mtx);
        reasons.push_back(r);
        events.push_back("stopped");
        cv.notify_all();
    }

    auto waitStopped(int seconds = 10) -> bool {
        std::unique_lock lock(mtx);
        return cv.wait_for(lock, std::chrono::seconds(seconds), [this]() { return !reasons.empty(); });
    }

    auto stops() -> std::vector<GPIOStopReason> {
        std::lock_guard lock(mtx);
        return reasons;
    }

    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::string> events;
    std::vector<GPIOStopReason> reasons;
};

// The words of a buffer of the server by its DMA address
auto wordsAt(uint32_t address) -> uint32_t* {
    for (const auto& r : uio_lib::CMemoryManager::instance()->getRegions(uio_lib::MM_GPIO)) {
        if (address == r.start + DataLib::sizeHeader())
            return reinterpret_cast<uint32_t*>(r.startMemory + DataLib::sizeHeader());
    }
    return nullptr;
}

template <typename P>
auto waitFor(P done, std::chrono::milliseconds limit = 5000ms) -> bool {
    auto end = std::chrono::steady_clock::now() + limit;
    while (!done()) {
        if (std::chrono::steady_clock::now() > end)
            return false;
        std::this_thread::sleep_for(5ms);
    }
    return true;
}

// The ring of a client in its own memory, one channel
struct ClientRing {
    uio_lib::CMemoryManager memory;
    CBuffersCached::Ptr ring = CBuffersCached::create();

    ClientRing() {
        memory.setMemoryBlockSize(kBlock);
        memory.reallocateBlocks();
        memory.reserveMemory(uio_lib::MM_GPIO, 16, 1);
        adc_channels_t one;
        one[ADCChannels::ADC_CH1] = true;
        ring->generateBuffersEmptyADC(one, memory.getRegions(uio_lib::MM_GPIO), DataLib::sizeHeader());
    }
};

// Takes the input stream of the server
struct InputClient {
    ClientRing side;
    net_lib::CAsioNet::Ptr net;
    std::mutex mtx;
    std::vector<DataLib::GPIOHeader> headers;
    std::vector<uint64_t> ids;
    std::vector<uint32_t> words;

    auto start() -> void {
        net = net_lib::CAsioNet::create(net_lib::M_CLIENT, "127.0.0.1", NET_GPIO_IN_STREAMING_PORT, side.ring);
        net->setPackID(DataLib::ID_PACK_GPIO);
        net->reciveNotify.connect([this](std::error_code e, CDataBuffersPackDMA::Ptr pack) {
            if (!e) {
                auto buffer = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1);
                DataLib::GPIOHeader h;
                uint64_t id = 0;
                if (DataLib::getHeaderGPIO(buffer, h, &id)) {
                    auto data = static_cast<const uint32_t*>(buffer->getMappedDataMemory());
                    std::lock_guard lock(mtx);
                    headers.push_back(h);
                    ids.push_back(id);
                    words.insert(words.end(), data, data + h.dataSize / 4);
                }
            }
            side.ring->unlockBufferRead();
        });
        net->start();
    }

    auto packs() -> size_t {
        std::lock_guard lock(mtx);
        return headers.size();
    }

    ~InputClient() {
        if (net)
            net->stop();
    }
};

// Sends the output stream to the server
struct OutputClient {
    ClientRing side;
    net_lib::CAsioNet::Ptr net;
    std::atomic<bool> connected{false};
    CDataBuffersPackDMA::Ptr pack;
    uint64_t id = 0;

    auto start() -> bool {
        net = net_lib::CAsioNet::create(net_lib::M_CLIENT, "127.0.0.1", NET_GPIO_OUT_STREAMING_PORT, nullptr);
        net->clientConnectNotify.connect([this](std::string&) { connected = true; });
        net->start();
        pack = side.ring->writeBuffer(true);
        return waitFor([this]() { return connected.load(); });
    }

    auto send(const std::vector<uint32_t>& words) -> bool {
        auto buffer = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1);
        std::memcpy(buffer->getMappedDataMemory(), words.data(), words.size() * 4);
        DataLib::GPIOHeader h;
        h.dataSize = words.size() * 4;
        DataLib::setHeaderGPIO(buffer, id++, h);
        return net->sendSyncData(pack);
    }

    ~OutputClient() {
        if (net)
            net->stop();
    }
};

// output buffer k: word i is k << 16 | i, with the lines in the upper half as the block takes them
auto outBuffer(uint32_t k, uint32_t words = kWords) -> std::vector<uint32_t> {
    std::vector<uint32_t> v(words);
    for (uint32_t i = 0; i < words; i++)
        v[i] = (k << 24) | ((i & 0xFF) << 16) | (i >> 8);
    return v;
}

class GPIOServer : public ::testing::Test {
   protected:
    void SetUp() override {
        uio_lib::dummy::resetGpioState();
        uio_lib::dummy::gpioState().present = true;
        uio_lib::dummy::gpioState().features = uio_lib::GPIO_FEATURE_OUT_FREE;
        auto memory = uio_lib::CMemoryManager::instance();
        memory->setMemoryBlockSize(kBlock);
        memory->reallocateBlocks();
        memory->setReserverdMemory(uio_lib::MM_GPIO, 24 * memory->getMemoryBlockSize());
        board = std::make_shared<board_lib::CDummyBoard>();
        hw = uio_lib::CHardware::create(board);
        server = std::make_shared<GPIOStreamServer>(hw);
        recorder = std::make_shared<Recorder>();
        server->setCallback(recorder);
        settings.setMemoryBlockSize(kBlock);
    }

    void TearDown() override {
        server = nullptr;
        uio_lib::dummy::resetGpioState();
    }

    auto directions(bool in, bool out) -> void {
        settings.setGPIOInStreaming(in ? CStreamSettings::State::ON : CStreamSettings::State::OFF);
        settings.setGPIOOutStreaming(out ? CStreamSettings::State::ON : CStreamSettings::State::OFF);
    }

    std::shared_ptr<board_lib::CDummyBoard> board;
    uio_lib::CHardware::Ptr hw;
    GPIOStreamServer::Ptr server;
    std::shared_ptr<Recorder> recorder;
    CStreamSettings settings;
};

}  // namespace

TEST_F(GPIOServer, WithoutAGPIOBlockTheStartIsAConfigurationError) {
    uio_lib::dummy::gpioState().present = false;
    hw = uio_lib::CHardware::create(board);
    server = std::make_shared<GPIOStreamServer>(hw);
    server->setCallback(recorder);
    directions(true, false);
    EXPECT_FALSE(server->start(settings));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::CONFIG_ERROR});
}

TEST_F(GPIOServer, NeitherDirectionIsAConfigurationError) {
    directions(false, false);
    EXPECT_FALSE(server->start(settings));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->events, (std::vector<std::string>{"stopped"}));
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::CONFIG_ERROR});
}

TEST_F(GPIOServer, NoMemoryIsAMemoryError) {
    uio_lib::CMemoryManager::instance()->setReserverdMemory(uio_lib::MM_GPIO, 0);
    directions(true, false);
    EXPECT_FALSE(server->start(settings));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::MEM_ERROR});
}

TEST_F(GPIOServer, TheSettingsReachTheBlock) {
    directions(true, true);
    settings.setGPIOOutputsP(0x0F);
    settings.setGPIOOutputsN(0x81);
    settings.setGPIOPolarity(0x0102);
    settings.setGPIODecimation(4);
    settings.setGPIORLE(CStreamSettings::State::ON);
    settings.setGPIOOutStep(2);
    ASSERT_TRUE(server->start(settings));
    auto& s = uio_lib::dummy::gpioState();
    EXPECT_EQ(s.outputsP, 0x0F);
    EXPECT_EQ(s.outputsN, 0x81);
    EXPECT_EQ(s.polarity, 0x0102);
    EXPECT_EQ(s.decimation, 4u);
    EXPECT_TRUE(s.rle);
    EXPECT_EQ(s.outStep, 2u);
    EXPECT_EQ(s.dataSize, kBlock);
    EXPECT_EQ(s.prepares, 1);
    EXPECT_EQ(s.starts, 0) << "with the output the block waits for its first buffer";
    EXPECT_EQ(recorder->events, std::vector<std::string>{"started"});
}

TEST_F(GPIOServer, TheInputReachesTheClientInOrder) {
    directions(true, false);
    settings.setGPIODecimation(8);
    std::atomic<uint32_t> next{0};
    uio_lib::dummy::gpioState().inFill = [&](uint8_t, uint32_t address) {
        std::this_thread::sleep_for(1ms);
        auto w = wordsAt(address);
        if (!w)
            return false;
        for (uint32_t i = 0; i < kWords; i++)
            w[i] = next++;
        return true;
    };
    uio_lib::dummy::gpioState().inLost = [](uint8_t) { return uint64_t(3); };
    InputClient client;
    ASSERT_TRUE(server->start(settings));
    EXPECT_EQ(uio_lib::dummy::gpioState().starts, 1) << "the input starts at once";
    client.start();
    ASSERT_TRUE(waitFor([&]() { return client.packs() >= 20; }));
    server->stop(GPIOStopReason::NORMAL);

    std::lock_guard lock(client.mtx);
    for (size_t i = 0; i < client.headers.size(); i++) {
        EXPECT_EQ(client.ids[i], i);
        EXPECT_EQ(client.headers[i].dataSize, kBlock);
        EXPECT_EQ(client.headers[i].decimation, 8u);
        EXPECT_EQ(client.headers[i].gpioClock, board->info().dacRate);
        EXPECT_EQ(client.headers[i].lostFPGA, 3u);
    }
    // the words of the packs follow each other, from where the client came in
    for (size_t i = 1; i < client.words.size(); i++)
        ASSERT_EQ(client.words[i], client.words[i - 1] + 1) << "word " << i;
}

TEST_F(GPIOServer, TheOutputReachesTheBlockInOrderAndStartsIt) {
    directions(false, true);
    std::mutex mtx;
    std::vector<uint32_t> firsts;
    std::vector<uint32_t> lasts;
    std::vector<bool> running;
    uio_lib::dummy::gpioState().outTaken = [&](uint8_t, uint32_t address) {
        auto w = wordsAt(address);
        std::lock_guard lock(mtx);
        running.push_back(uio_lib::dummy::gpioState().running);
        firsts.push_back(w ? w[0] : 0);
        lasts.push_back(w ? w[kWords - 1] : 0);
    };
    ASSERT_TRUE(server->start(settings));
    OutputClient client;
    ASSERT_TRUE(client.start());
    for (uint32_t k = 0; k < 5; k++)
        ASSERT_TRUE(client.send(outBuffer(k)));
    ASSERT_TRUE(client.send(outBuffer(5, 100)));  // the end, in part
    ASSERT_TRUE(waitFor([&]() {
        std::lock_guard lock(mtx);
        return firsts.size() == 6;
    }));
    EXPECT_EQ(uio_lib::dummy::gpioState().starts, 1);
    std::lock_guard lock(mtx);
    for (uint32_t k = 0; k < 6; k++)
        EXPECT_EQ(firsts[k], outBuffer(k)[0]) << "buffer " << k;
    // the part not sent holds the last lines sent, one sample per word
    EXPECT_EQ(lasts[5], outBuffer(5, 100)[99] & 0xFFFF0000u);
    // the ready flags are cleared while the output is stopped: only the first buffer goes before
    EXPECT_FALSE(running[0]);
    EXPECT_TRUE(running[1]);
}

TEST_F(GPIOServer, AnOutputOfOneBufferPlaysToo) {
    directions(false, true);
    std::atomic<int> taken{0};
    uio_lib::dummy::gpioState().outTaken = [&](uint8_t, uint32_t) { taken++; };
    ASSERT_TRUE(server->start(settings));
    OutputClient client;
    ASSERT_TRUE(client.start());
    ASSERT_TRUE(client.send(outBuffer(0)));
    EXPECT_TRUE(waitFor([&]() { return uio_lib::dummy::gpioState().starts == 1; }, 3000ms));
    EXPECT_EQ(taken, 1);
}

// An FPGA whose output does not tell its free buffers can't stream the output; the input it can
TEST_F(GPIOServer, AnOutputWithoutTheFreeFlagsIsAConfigurationError) {
    uio_lib::dummy::gpioState().features = 0;
    hw = uio_lib::CHardware::create(board);
    server = std::make_shared<GPIOStreamServer>(hw);
    server->setCallback(recorder);
    directions(true, true);
    EXPECT_FALSE(server->start(settings));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::CONFIG_ERROR});
    directions(true, false);
    EXPECT_TRUE(server->start(settings));
}

// The block reads the buffer of a slot until the slot takes the next but one: the client's next
// buffers must not be written into the two packs taken last
TEST_F(GPIOServer, AnOutputPackStaysOutOfTheRingWhileTheBlockHoldsIt) {
    directions(false, true);
    struct Taken {
        const uint32_t* data;
        std::vector<uint32_t> copy;
    };
    std::mutex mtx;
    std::vector<Taken> taken;
    int overwritten = 0;
    uio_lib::dummy::gpioState().outTaken = [&](uint8_t, uint32_t address) {
        std::this_thread::sleep_for(1ms);  // the block is slower than the client: the ring is full
        std::lock_guard lock(mtx);
        for (size_t back = 1; back <= 2 && back <= taken.size(); back++) {
            auto& t = taken[taken.size() - back];
            if (std::memcmp(t.data, t.copy.data(), kBlock) != 0)
                overwritten++;
        }
        auto w = wordsAt(address);
        if (w)
            taken.push_back({w, std::vector<uint32_t>(w, w + kWords)});
    };
    ASSERT_TRUE(server->start(settings));
    OutputClient client;
    ASSERT_TRUE(client.start());
    for (uint32_t k = 0; k < 100; k++)
        ASSERT_TRUE(client.send(outBuffer(k)));
    ASSERT_TRUE(waitFor([&]() {
        std::lock_guard lock(mtx);
        return taken.size() >= 90;
    }));
    server->stop(GPIOStopReason::NORMAL);
    std::lock_guard lock(mtx);
    EXPECT_EQ(overwritten, 0);
}

TEST_F(GPIOServer, AStopIsReportedAlsoWhenNothingRuns) {
    server->stop(GPIOStopReason::NORMAL);
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::NORMAL});
}

TEST_F(GPIOServer, AStopEndsBothDirections) {
    directions(true, true);
    uio_lib::dummy::gpioState().inFill = [&](uint8_t, uint32_t) {
        std::this_thread::sleep_for(1ms);
        return true;
    };
    ASSERT_TRUE(server->start(settings));
    InputClient in;
    in.start();
    OutputClient out;
    ASSERT_TRUE(out.start());
    for (uint32_t k = 0; k < 4; k++)
        ASSERT_TRUE(out.send(outBuffer(k)));
    ASSERT_TRUE(waitFor([&]() { return in.packs() >= 5; }));
    EXPECT_TRUE(server->isRunning());
    server->stop(GPIOStopReason::NORMAL);
    EXPECT_FALSE(server->isRunning());
    EXPECT_FALSE(uio_lib::dummy::gpioState().running);
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::NORMAL});
}

TEST_F(GPIOServer, ASecondStartOpensThePortsAgain) {
    directions(true, false);
    uio_lib::dummy::gpioState().inFill = [&](uint8_t, uint32_t) {
        std::this_thread::sleep_for(1ms);
        return true;
    };
    for (int run = 0; run < 3; run++) {
        ASSERT_TRUE(server->start(settings)) << "run " << run;
        InputClient client;
        client.start();
        ASSERT_TRUE(waitFor([&]() { return client.packs() >= 3; })) << "run " << run;
    }
    server->stop(GPIOStopReason::NORMAL);
}
