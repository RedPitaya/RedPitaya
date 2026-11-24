/**
 * GPIOInStreamServer and GPIOOutStreamServer, the GPIO input and output of a board over CHardware:
 * the servers of the host build run on the dummy GPIO block, their clients connect to the data
 * ports over TCP on localhost. The input buffers reach the client in order with their header,
 * the output buffers of the client reach the block in order once they filled the ring, a pack
 * stays out of the ring while the block holds it, the stop tells the clocks the output waited,
 * and the two directions run on their own.
 */

#include <gtest/gtest.h>
#include <algorithm>

#include <filesystem>
#include <fstream>

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
#include "gpio_streaming_lib/gpio_in_stream_server.h"
#include "gpio_streaming_lib/gpio_out_stream_server.h"
#include "net_lib/asio_net.h"
#include "bin_lib/bin_file.h"
#include "bin_lib/gpio_words.h"
#include "uio_lib/gpio.h"
#include "uio_lib/gpio_dummy.h"
#include "uio_lib/memory_manager.h"
#include "writer_lib/time_limit_clock.h"

using namespace gpio_streaming_lib;
using namespace std::chrono_literals;
using DataLib::CBuffersCached;
using DataLib::CDataBuffersPackDMA;

namespace {

constexpr uint32_t kBlock = 4096;
constexpr uint32_t kWords = kBlock / 4;
constexpr uint32_t kOutPacks = 12;  // the ring of the output

// What a server reports
struct Recorder : public GPIOInServerCallback, public GPIOOutServerCallback {
    void started(GPIOInStreamServer*) override { add("in started"); }
    void stopped(GPIOInStreamServer*, GPIOStopReason r) override { stop("in stopped", r, 0); }
    void started(GPIOOutStreamServer*) override { add("out started"); }
    void startedSD(GPIOInStreamServer*) override { add("in started sd"); }
    void startedSD(GPIOOutStreamServer*) override { add("out started sd"); }
    void stopped(GPIOOutStreamServer*, GPIOStopReason r, uint64_t w) override { stop("out stopped", r, w); }

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
    uint64_t wait = 0;

   private:
    void add(const std::string& e) {
        std::lock_guard lock(mtx);
        events.push_back(e);
    }
    void stop(const std::string& e, GPIOStopReason r, uint64_t w) {
        std::lock_guard lock(mtx);
        events.push_back(e);
        reasons.push_back(r);
        wait = w;
        cv.notify_all();
    }
};

auto regionsOf() -> std::vector<uio_lib::MemoryRegionT> {
    auto memory = uio_lib::CMemoryManager::instance();
    auto in = memory->getRegions(uio_lib::MM_GPIO_IN);
    auto out = memory->getRegions(uio_lib::MM_GPIO_OUT);
    in.insert(in.end(), out.begin(), out.end());
    return in;
}

// The words of a buffer of the server by its DMA address
auto wordsAt(uint32_t address) -> uint32_t* {
    for (const auto& r : regionsOf()) {
        if (address == r.start + DataLib::sizeHeader())
            return reinterpret_cast<uint32_t*>(r.startMemory + DataLib::sizeHeader());
    }
    return nullptr;
}

// Waits for a success only, returning at once on it: the limit is for the slow machines (the
// emulated ARM of CI, the sanitizers), not for the time a test takes.
template <typename P>
auto waitFor(P done, std::chrono::milliseconds limit = 20000ms) -> bool {
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
        memory.reserveMemory(uio_lib::MM_GPIO_IN, 16, 1);
        adc_channels_t one;
        one[ADCChannels::ADC_CH1] = true;
        ring->generateBuffersEmptyADC(one, memory.getRegions(uio_lib::MM_GPIO_IN), DataLib::sizeHeader());
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

    // the network goes first, its thread joined: a pack it still delivers must not reach the
    // members below, which go before it otherwise
    ~InputClient() {
        if (net)
            net->stop();
        net = nullptr;
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

    // as the input client: the network and its thread go before the members its handlers use
    ~OutputClient() {
        if (net)
            net->stop();
        net = nullptr;
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
        uio_lib::dummy::gpioState().features = uio_lib::GPIO_FEATURE_OUT_FREE | uio_lib::GPIO_FEATURE_OUT_WAIT;
        auto memory = uio_lib::CMemoryManager::instance();
        memory->setMemoryBlockSize(kBlock);
        memory->reallocateBlocks();
        memory->setReserverdMemory(uio_lib::MM_GPIO_IN, 12 * memory->getMemoryBlockSize());
        memory->setReserverdMemory(uio_lib::MM_GPIO_OUT, kOutPacks * memory->getMemoryBlockSize());
        board = std::make_shared<board_lib::CDummyBoard>();
        recreate();
        settings.setMemoryBlockSize(kBlock);
    }

    void TearDown() override {
        in = nullptr;
        out = nullptr;
        uio_lib::dummy::resetGpioState();
    }

    // a new hardware object, after a change of the dummy state its features depend on
    void recreate() {
        in = nullptr;
        out = nullptr;
        hw = uio_lib::CHardware::create(board);
        in = std::make_shared<GPIOInStreamServer>(hw);
        out = std::make_shared<GPIOOutStreamServer>(hw);
        in->setCallback(std::shared_ptr<GPIOInServerCallback>(recorder, recorder.get()));
        out->setCallback(std::shared_ptr<GPIOOutServerCallback>(recorder, recorder.get()));
    }

    // the input fills a buffer every millisecond with a counter
    void countingInput() {
        uio_lib::dummy::gpioState().inFill = [this](uint8_t, uint32_t address) {
            std::this_thread::sleep_for(1ms);
            auto w = wordsAt(address);
            if (!w)
                return false;
            for (uint32_t i = 0; i < kWords; i++)
                w[i] = next++;
            return true;
        };
    }

    std::shared_ptr<board_lib::CDummyBoard> board;
    uio_lib::CHardware::Ptr hw;
    GPIOInStreamServer::Ptr in;
    GPIOOutStreamServer::Ptr out;
    std::shared_ptr<Recorder> recorder = std::make_shared<Recorder>();
    CStreamSettings settings;
    std::atomic<uint32_t> next{0};
};

}  // namespace

// --- the input

TEST_F(GPIOServer, WithoutAGPIOBlockTheStartIsAConfigurationError) {
    uio_lib::dummy::gpioState().present = false;
    recreate();
    EXPECT_FALSE(in->start(settings));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::CONFIG_ERROR});
}

TEST_F(GPIOServer, NoMemoryIsAMemoryError) {
    uio_lib::CMemoryManager::instance()->setReserverdMemory(uio_lib::MM_GPIO_IN, 0);
    EXPECT_FALSE(in->start(settings));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::MEM_ERROR});
}

TEST_F(GPIOServer, TheInputSettingsReachTheBlock) {
    settings.setGPIOPolarity(0x0102);
    settings.setGPIODecimation(4);
    settings.setGPIORLE(CStreamSettings::State::ON);
    ASSERT_TRUE(in->start(settings));
    auto& s = uio_lib::dummy::gpioState();
    EXPECT_EQ(s.polarity, 0x0102);
    EXPECT_EQ(s.decimation, 4u);
    EXPECT_TRUE(s.rle);
    EXPECT_EQ(s.dataSize, kBlock);
    EXPECT_EQ(s.inPrepares, 1);
    EXPECT_EQ(s.inStarts, 1) << "the input starts at once";
    EXPECT_EQ(s.outStarts, 0);
    EXPECT_EQ(recorder->events, std::vector<std::string>{"in started"});
}

TEST_F(GPIOServer, TheInputReachesTheClientInOrder) {
    settings.setGPIODecimation(8);
    countingInput();
    uio_lib::dummy::gpioState().inLost = [](uint8_t) { return uint64_t(3); };
    InputClient client;
    ASSERT_TRUE(in->start(settings));
    client.start();
    ASSERT_TRUE(waitFor([&]() { return client.packs() >= 20; }));
    in->stop(GPIOStopReason::NORMAL);

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

TEST_F(GPIOServer, AStopIsReportedAlsoWhenNothingRuns) {
    in->stop(GPIOStopReason::NORMAL);
    out->stop(GPIOStopReason::NORMAL);
    EXPECT_EQ(recorder->stops(), (std::vector<GPIOStopReason>{GPIOStopReason::NORMAL, GPIOStopReason::NORMAL}));
}

TEST_F(GPIOServer, ASecondStartOpensThePortAgain) {
    countingInput();
    for (int run = 0; run < 3; run++) {
        ASSERT_TRUE(in->start(settings)) << "run " << run;
        InputClient client;
        client.start();
        ASSERT_TRUE(waitFor([&]() { return client.packs() >= 3; })) << "run " << run;
    }
    in->stop(GPIOStopReason::NORMAL);
}

// --- the output

// An FPGA whose output does not tell its free buffers can't stream the output; the input it can
TEST_F(GPIOServer, AnOutputWithoutTheFreeFlagsIsAConfigurationError) {
    uio_lib::dummy::gpioState().features = 0;
    recreate();
    EXPECT_FALSE(out->start(settings));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::CONFIG_ERROR});
    EXPECT_TRUE(in->start(settings));
}

TEST_F(GPIOServer, TheOutputSettingsReachTheBlock) {
    settings.setGPIOOutputsP(0x0F);
    settings.setGPIOOutputsN(0x81);
    settings.setGPIORLE(CStreamSettings::State::ON);
    settings.setGPIOOutStep(2);
    ASSERT_TRUE(out->start(settings));
    auto& s = uio_lib::dummy::gpioState();
    EXPECT_EQ(s.outputsP, 0x0F);
    EXPECT_EQ(s.outputsN, 0x81);
    EXPECT_TRUE(s.rle);
    EXPECT_EQ(s.outStep, 2u);
    EXPECT_EQ(s.dataSize, kBlock);
    EXPECT_EQ(s.outPrepares, 1);
    EXPECT_EQ(s.outStarts, 0) << "the output waits for its first buffers";
    EXPECT_EQ(s.inStarts, 0);
    EXPECT_EQ(recorder->events, std::vector<std::string>{"out started"});
}

// A few buffers play 100 ms after the last came; a full ring plays at once
TEST_F(GPIOServer, TheOutputFillsItsRingBeforeItPlays) {
    std::atomic<int> taken{0};
    uio_lib::dummy::gpioState().outTaken = [&](uint8_t, uint32_t) { taken++; };
    ASSERT_TRUE(out->start(settings));
    OutputClient client;
    ASSERT_TRUE(client.start());
    for (uint32_t k = 0; k < 3; k++)
        ASSERT_TRUE(client.send(outBuffer(k)));
    auto sent = std::chrono::steady_clock::now();
    ASSERT_TRUE(waitFor([&]() { return uio_lib::dummy::gpioState().outStarts == 1; }));
    EXPECT_GE(std::chrono::steady_clock::now() - sent, 90ms);
    out->stop(GPIOStopReason::NORMAL);

    taken = 0;
    uio_lib::dummy::gpioState().outDone = [](uint8_t) { return false; };  // nothing plays out
    ASSERT_TRUE(out->start(settings));
    OutputClient client2;
    ASSERT_TRUE(client2.start());
    for (uint32_t k = 0; k < kOutPacks - 1; k++)
        ASSERT_TRUE(client2.send(outBuffer(k)));
    ASSERT_TRUE(waitFor([&]() { return uio_lib::dummy::gpioState().outStarts == 2; }, 80ms)) << "a full ring plays at once";
    out->stop(GPIOStopReason::NORMAL);
}

TEST_F(GPIOServer, TheOutputReachesTheBlockInOrder) {
    std::mutex mtx;
    std::vector<uint32_t> firsts;
    std::vector<uint32_t> lasts;
    std::vector<bool> running;
    uio_lib::dummy::gpioState().outTaken = [&](uint8_t, uint32_t address) {
        auto w = wordsAt(address);
        std::lock_guard lock(mtx);
        running.push_back(uio_lib::dummy::gpioState().outRunning);
        firsts.push_back(w ? w[0] : 0);
        lasts.push_back(w ? w[kWords - 1] : 0);
    };
    ASSERT_TRUE(out->start(settings));
    OutputClient client;
    ASSERT_TRUE(client.start());
    for (uint32_t k = 0; k < 5; k++)
        ASSERT_TRUE(client.send(outBuffer(k)));
    ASSERT_TRUE(client.send(outBuffer(5, 100)));  // the end, in part
    ASSERT_TRUE(waitFor([&]() {
        std::lock_guard lock(mtx);
        return firsts.size() == 6;
    }));
    std::lock_guard lock(mtx);
    for (uint32_t k = 0; k < 6; k++)
        EXPECT_EQ(firsts[k], outBuffer(k)[0]) << "buffer " << k;
    // the part not sent holds the last lines sent, one sample per word
    EXPECT_EQ(lasts[5], outBuffer(5, 100)[99] & 0xFFFF0000u);
    // the ready flags are cleared while the output is stopped: only the first buffer goes before
    EXPECT_FALSE(running[0]);
    EXPECT_TRUE(running[1]);
}

// The block reads the buffer of a slot until the slot takes the next but one: the client's next
// buffers must not be written into the two packs taken last
TEST_F(GPIOServer, AnOutputPackStaysOutOfTheRingWhileTheBlockHoldsIt) {
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
    ASSERT_TRUE(out->start(settings));
    OutputClient client;
    ASSERT_TRUE(client.start());
    for (uint32_t k = 0; k < 100; k++)
        ASSERT_TRUE(client.send(outBuffer(k)));
    ASSERT_TRUE(waitFor([&]() {
        std::lock_guard lock(mtx);
        return taken.size() >= 90;
    }));
    out->stop(GPIOStopReason::NORMAL);
    std::lock_guard lock(mtx);
    EXPECT_EQ(overwritten, 0);
}

// The clocks the output waited for the client's data come with the stop
TEST_F(GPIOServer, TheStopOfTheOutputTellsItsWait) {
    uio_lib::dummy::gpioState().outWait = []() { return uint64_t(0x123456789); };
    ASSERT_TRUE(out->start(settings));
    OutputClient client;
    ASSERT_TRUE(client.start());
    ASSERT_TRUE(client.send(outBuffer(0)));
    ASSERT_TRUE(waitFor([&]() { return uio_lib::dummy::gpioState().outStarts == 1; }));
    EXPECT_EQ(out->waitClocks(), 0x123456789u);
    out->stop(GPIOStopReason::NORMAL);
    EXPECT_EQ(recorder->wait, 0x123456789u);
    EXPECT_FALSE(uio_lib::dummy::gpioState().outRunning);
}

// --- both

// Each starts and stops without the other: the output stops, the input goes on
TEST_F(GPIOServer, TheInputAndTheOutputRunOnTheirOwn) {
    countingInput();
    ASSERT_TRUE(in->start(settings));
    InputClient inClient;
    inClient.start();
    ASSERT_TRUE(out->start(settings));
    OutputClient outClient;
    ASSERT_TRUE(outClient.start());
    for (uint32_t k = 0; k < 4; k++)
        ASSERT_TRUE(outClient.send(outBuffer(k)));
    ASSERT_TRUE(waitFor([&]() { return uio_lib::dummy::gpioState().outRunning.load(); }));
    const int inStops = uio_lib::dummy::gpioState().inStops;
    out->stop(GPIOStopReason::NORMAL);
    EXPECT_FALSE(uio_lib::dummy::gpioState().outRunning);
    EXPECT_TRUE(uio_lib::dummy::gpioState().inRunning);
    EXPECT_EQ(uio_lib::dummy::gpioState().inStops, inStops) << "the stop of the output leaves the input";
    auto packs = inClient.packs();
    EXPECT_TRUE(waitFor([&]() { return inClient.packs() >= packs + 5; })) << "the input goes on";
    EXPECT_TRUE(in->isRunning());
    in->stop(GPIOStopReason::NORMAL);
    EXPECT_FALSE(uio_lib::dummy::gpioState().inRunning);
}

// What both use is set by the first; the second takes it only as it is
TEST_F(GPIOServer, ASharedSettingThatDiffersIsRefusedWhileTheOtherRuns) {
    settings.setGPIORLE(CStreamSettings::State::ON);
    ASSERT_TRUE(in->start(settings));
    CStreamSettings other = settings;
    other.setGPIORLE(CStreamSettings::State::OFF);
    EXPECT_FALSE(out->start(other));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::CONFIG_ERROR});
    EXPECT_TRUE(out->start(settings));
    EXPECT_TRUE(in->isRunning());
}

// --- the client of the data leaves

TEST_F(GPIOServer, AnInputClientThatLeavesStopsTheInput) {
    countingInput();
    ASSERT_TRUE(in->start(settings));
    {
        InputClient client;
        client.start();
        ASSERT_TRUE(waitFor([&]() { return client.packs() > 2; }));
        EXPECT_TRUE(recorder->stops().empty()) << "streaming while the client is there";
    }
    ASSERT_TRUE(recorder->waitStopped());
    std::this_thread::sleep_for(300ms);
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::DISCONNECTED});
    EXPECT_FALSE(uio_lib::dummy::gpioState().inRunning);
}

// The client sends what it has and leaves: the output plays all of it, then stops
TEST_F(GPIOServer, WhatTheOutputClientSentBeforeItLeftIsPlayedOut) {
    std::atomic<int> taken{0};
    uio_lib::dummy::gpioState().outTaken = [&](uint8_t, uint32_t) { taken++; };
    // the output waits once the last buffer is handed over and played: the counter runs then
    std::atomic<uint64_t> waited{0};
    uio_lib::dummy::gpioState().outWait = [&]() -> uint64_t { return taken >= 5 ? ++waited : 0; };
    ASSERT_TRUE(out->start(settings));
    {
        OutputClient client;
        ASSERT_TRUE(client.start());
        for (uint32_t k = 0; k < 5; k++)
            ASSERT_TRUE(client.send(outBuffer(k)));
    }
    ASSERT_TRUE(recorder->waitStopped());
    std::this_thread::sleep_for(300ms);
    EXPECT_EQ(taken.load(), 5) << "every buffer sent before the client left";
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::DISCONNECTED});
    EXPECT_FALSE(uio_lib::dummy::gpioState().outRunning);
}

TEST_F(GPIOServer, AnOutputClientThatLeavesWithoutDataStopsTheOutput) {
    ASSERT_TRUE(out->start(settings));
    {
        OutputClient client;
        ASSERT_TRUE(client.start());
    }
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::DISCONNECTED});
}

TEST_F(GPIOServer, AStopWithTheClientsConnectedIsReportedOncePerDirection) {
    countingInput();
    ASSERT_TRUE(in->start(settings));
    ASSERT_TRUE(out->start(settings));
    InputClient inClient;
    inClient.start();
    OutputClient outClient;
    ASSERT_TRUE(outClient.start());
    ASSERT_TRUE(waitFor([&]() { return inClient.packs() > 2; }));
    in->stop(GPIOStopReason::NORMAL);
    out->stop(GPIOStopReason::NORMAL);
    std::this_thread::sleep_for(300ms);
    EXPECT_EQ(recorder->stops(), (std::vector<GPIOStopReason>{GPIOStopReason::NORMAL, GPIOStopReason::NORMAL}));
}

// --- the file mode

namespace {

auto FileDir() -> std::string {
    auto dir = std::filesystem::temp_directory_path() / ("gpio_files_" + std::to_string(::getpid()));
    std::filesystem::create_directories(dir);
    return dir.string();
}

}  // namespace

TEST_F(GPIOServer, TheInputWritesABinFileUpToItsLimit) {
    countingInput();
    const auto dir = FileDir();
    for (auto& e : std::filesystem::directory_iterator(dir))
        std::filesystem::remove(e.path());
    settings.setGPIOInPassMode(CStreamSettings::PassMode::FILE);
    settings.setGPIOInFormat(CStreamSettings::GPIOFormat::BIN);
    settings.setGPIOInSamples(3 * kWords);  // no RLE: a word is a sample
    GPIOStreamOptions o;
    o.dirPath = dir;
    ASSERT_TRUE(in->start(settings, o));
    EXPECT_EQ(in->filePath().rfind(dir + "/gpio_", 0), 0u) << in->filePath();
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::DONE});
    EXPECT_EQ(recorder->events.front(), "in started sd");

    bin_lib::FileReader r;
    ASSERT_TRUE(r.open(in->filePath())) << r.error();
    EXPECT_EQ(r.info().source, bin_lib::Source::GPIO_IN);
    bin_lib::BlockInfo b;
    std::vector<uint8_t> data;
    uint32_t expect = 0;
    int blocks = 0;
    while (r.nextBlock(b, data)) {
        const auto* w = reinterpret_cast<const uint32_t*>(data.data());
        for (size_t i = 0; i < data.size() / 4; i++)
            ASSERT_EQ(w[i], expect++) << "block " << blocks << " word " << i;
        blocks++;
    }
    EXPECT_EQ(blocks, 3) << "the third buffer reaches the limit";
}

// A trigger that never comes (no buffer at all): the time limit ends the recording on the clock,
// a done run with an empty file (streaming_improvements 4.11)
TEST_F(GPIOServer, TheTimeLimitEndsTheInputWhenNoBufferComes) {
    uio_lib::dummy::gpioState().inFill = [](uint8_t, uint32_t) {
        std::this_thread::sleep_for(1ms);
        return false;
    };
    const auto dir = FileDir();
    for (auto& e : std::filesystem::directory_iterator(dir))
        std::filesystem::remove(e.path());
    settings.setGPIOInPassMode(CStreamSettings::PassMode::FILE);
    settings.setGPIOInFormat(CStreamSettings::GPIOFormat::BIN);
    CStreamSettings::FileLimits l;
    l.time = 0.1;
    ASSERT_TRUE(settings.setGPIOInLimits(l));
    GPIOStreamOptions o;
    o.dirPath = dir;
    const auto t0 = std::chrono::steady_clock::now();
    ASSERT_TRUE(in->start(settings, o));
    ASSERT_TRUE(recorder->waitStopped());
    const double took = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::DONE});
    EXPECT_GE(took, 0.1 + TimeLimitClock::kMargin - 0.05) << "the limit and the margin";
    EXPECT_LT(took, 8.0);
    bin_lib::FileReader r;
    ASSERT_TRUE(r.open(in->filePath())) << r.error();
    EXPECT_EQ(r.samples(0, false), 0u);
}

// The input in parts of a size (streaming_improvements 4.11): every word once, in order, across
// the numbered parts; the record names the last part, its number and the limits
TEST_F(GPIOServer, TheInputInPartsKeepsEveryWordAcrossThem) {
    countingInput();
    const auto dir = FileDir();
    for (auto& e : std::filesystem::directory_iterator(dir))
        std::filesystem::remove(e.path());
    settings.setGPIOInPassMode(CStreamSettings::PassMode::FILE);
    settings.setGPIOInFormat(CStreamSettings::GPIOFormat::BIN);
    settings.setGPIOInSamples(6 * kWords);
    CStreamSettings::FileLimits l;
    l.splitSize = 2 * kWords * 4;  // about two buffers a part
    ASSERT_TRUE(settings.setGPIOInLimits(l));
    GPIOStreamOptions o;
    o.dirPath = dir;
    ASSERT_TRUE(in->start(settings, o));
    EXPECT_NE(in->filePath().find("_0001.bin"), std::string::npos) << in->filePath();
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::DONE});
    std::vector<std::string> parts;
    for (auto& e : std::filesystem::directory_iterator(dir))
        if (e.path().extension() == ".bin")
            parts.push_back(e.path().string());
    std::sort(parts.begin(), parts.end());
    ASSERT_GE(parts.size(), 3u);
    const auto r = in->getRecord();
    EXPECT_EQ(r.part, static_cast<int>(parts.size()));
    EXPECT_EQ(r.file, parts.back());
    EXPECT_EQ(r.limits.splitSize, l.splitSize);
    uint32_t expect = 0;
    for (const auto& p : parts) {
        bin_lib::FileReader fr;
        ASSERT_TRUE(fr.open(p)) << p << ": " << fr.error();
        bin_lib::BlockInfo b;
        std::vector<uint8_t> data;
        while (fr.nextBlock(b, data)) {
            const auto* w = reinterpret_cast<const uint32_t*>(data.data());
            for (size_t i = 0; i < data.size() / 4; i++)
                ASSERT_EQ(w[i], expect++) << p;
        }
    }
    EXPECT_EQ(expect, 6u * kWords) << "every word of the six buffers";
}

// The losses the page shows (item 1.4 of the streaming improvements): what the block reports lost
// before each buffer, and the samples of the buffers; kept after the stop, reset by the next start
TEST_F(GPIOServer, TheLossesOfTheInputAreCountedKeptAndResetByTheNextStart) {
    countingInput();
    uio_lib::dummy::gpioState().inLost = [](uint8_t) { return uint64_t(5); };
    const auto dir = FileDir();
    for (auto& e : std::filesystem::directory_iterator(dir))
        std::filesystem::remove(e.path());
    settings.setGPIOInPassMode(CStreamSettings::PassMode::FILE);
    settings.setGPIOInFormat(CStreamSettings::GPIOFormat::BIN);
    settings.setGPIOInSamples(3 * kWords);  // no RLE: a word is a sample
    GPIOStreamOptions o;
    o.dirPath = dir;
    ASSERT_TRUE(in->start(settings, o));
    ASSERT_TRUE(recorder->waitStopped());
    const uint64_t samples = in->getBoardSamples();
    ASSERT_GE(samples, 3u * kWords);
    EXPECT_EQ(samples % kWords, 0u) << "whole buffers";
    EXPECT_EQ(in->getBoardLost(), samples / kWords * 5) << "5 before each of " << samples / kWords << " buffers";

    uio_lib::dummy::gpioState().inLost = nullptr;
    settings.setGPIOInPassMode(CStreamSettings::PassMode::NET);
    ASSERT_TRUE(in->start(settings, GPIOStreamOptions()));
    EXPECT_EQ(in->getBoardLost(), 0u) << "reset by the start";
    in->stop(GPIOStopReason::NORMAL);
}

// A limit the first buffer reaches ends the run at once: started still goes out first (the
// client takes a stop before the started as a failed start)
TEST_F(GPIOServer, AFileThatEndsWithItsFirstBufferIsStartedFirst) {
    countingInput();
    const auto dir = FileDir();
    settings.setGPIOInPassMode(CStreamSettings::PassMode::FILE);
    settings.setGPIOInFormat(CStreamSettings::GPIOFormat::BIN);
    settings.setGPIOInSamples(1);
    GPIOStreamOptions o;
    o.dirPath = dir;
    for (int run = 0; run < 20; run++) {
        {
            std::lock_guard lock(recorder->mtx);
            recorder->events.clear();
            recorder->reasons.clear();
        }
        ASSERT_TRUE(in->start(settings, o)) << "run " << run;
        ASSERT_TRUE(recorder->waitStopped()) << "run " << run;
        std::lock_guard lock(recorder->mtx);
        ASSERT_EQ(recorder->events, (std::vector<std::string>{"in started sd", "in stopped"})) << "run " << run;
        EXPECT_EQ(recorder->reasons, std::vector<GPIOStopReason>{GPIOStopReason::DONE}) << "run " << run;
    }
    for (auto& e : std::filesystem::directory_iterator(dir))
        std::filesystem::remove(e.path());
}

TEST_F(GPIOServer, TheOutputPlaysAFileToItsEnd) {
    const auto dir = FileDir();
    // 5 buffers of output words, no RLE: word i of buffer k is outBuffer(k)[i]
    bin_lib::FileInfo info;
    info.source = bin_lib::Source::GPIO_OUT;
    info.channels = {{1, bin_lib::SampleType::GPIO_OUT_WORD, 0, 0}};
    auto bytes = bin_lib::fileHeader(info);
    bin_lib::Timeline line;
    for (uint32_t k = 0; k < 5; k++) {
        auto words = outBuffer(k);
        bin_lib::BlockInfo b;
        b.samples = words.size();
        b.position = line.next(0, 0, 0, b.samples);
        bin_lib::appendBlock(bytes, b, words.data(), static_cast<uint32_t>(words.size() * 4));
    }
    std::ofstream(dir + "/play.bin", std::ios::binary).write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

    std::mutex m;
    std::vector<uint32_t> firsts;
    uio_lib::dummy::gpioState().outTaken = [&](uint8_t, uint32_t address) {
        auto w = wordsAt(address);
        std::lock_guard lock(m);
        firsts.push_back(w ? w[0] : 0);
    };
    std::atomic<uint64_t> waited{0};
    uio_lib::dummy::gpioState().outWait = [&]() -> uint64_t {
        std::lock_guard lock(m);
        return firsts.size() >= 5 ? ++waited : 0;
    };
    settings.setGPIOOutPassMode(CStreamSettings::PassMode::FILE);
    settings.setGPIOOutFile("play.bin");
    GPIOStreamOptions o;
    o.dirPath = dir;
    ASSERT_TRUE(out->start(settings, o));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<GPIOStopReason>{GPIOStopReason::DONE});
    EXPECT_EQ(recorder->events.front(), "out started sd");
    std::lock_guard lock(m);
    ASSERT_EQ(firsts.size(), 5u);
    for (uint32_t k = 0; k < 5; k++)
        EXPECT_EQ(firsts[k], outBuffer(k)[0]) << "buffer " << k;
}

TEST_F(GPIOServer, TheOutputRefusesAFileItCannotPlay) {
    const auto dir = FileDir();
    GPIOStreamOptions o;
    o.dirPath = dir;
    settings.setGPIOOutPassMode(CStreamSettings::PassMode::FILE);
    settings.setGPIOOutFile("none.bin");
    EXPECT_FALSE(out->start(settings, o));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops().back(), GPIOStopReason::MISSING_FILE);

    bin_lib::FileInfo info;
    info.channels = {{1, bin_lib::SampleType::INT16, 0, 14}};
    auto bytes = bin_lib::fileHeader(info);
    std::ofstream(dir + "/adc.bin", std::ios::binary).write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    settings.setGPIOOutFile("adc.bin");
    EXPECT_FALSE(out->start(settings, o));
    ASSERT_TRUE(waitFor([&]() { return recorder->stops().size() == 2; }));
    EXPECT_EQ(recorder->stops().back(), GPIOStopReason::BROKEN);
}
