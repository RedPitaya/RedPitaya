/**
 * ADCStreamServer, the ADC streaming of a board over CHardware: the server of the host build runs
 * on the dummy scope (a buffer full every 10 ms) and writes files; what it reports, when and why
 * it stops.
 */

#include <gtest/gtest.h>
#include <algorithm>
#include <cstdio>

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>
#include <vector>

#include "board_lib/board.h"
#include "streaming_lib/adc_stream_server.h"
#include "uio_lib/memory_manager.h"
#include "uio_lib/oscilloscope_dummy.h"
#include "uio_lib/trigger.h"
#include "bin_lib/bin_file.h"

#include "asio.hpp"
#include "net_lib/asio_common.h"

using namespace streaming_lib;

namespace {

class Recorder : public ADCServerCallback {
   public:
    void startedTCP(ADCStreamServer*) override { add("tcp"); }
    void startedSD(ADCStreamServer*) override { add("sd"); }
    void stopped(ADCStreamServer*, ADCStopReason r) override {
        std::lock_guard lock(mtx);
        reasons.push_back(r);
        events.push_back("stopped");
        cv.notify_all();
    }

    auto waitStopped(int seconds = 10) -> bool {
        std::unique_lock lock(mtx);
        return cv.wait_for(lock, std::chrono::seconds(seconds), [this]() { return !reasons.empty(); });
    }

    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::string> events;
    std::vector<ADCStopReason> reasons;

   private:
    void add(const std::string& e) {
        std::lock_guard lock(mtx);
        events.push_back(e);
    }
};

class ADCServer : public ::testing::Test {
   protected:
    void SetUp() override {
        uio_lib::dummy::resetOscState();
        auto memory = uio_lib::CMemoryManager::instance();
        memory->reallocateBlocks();
        memory->setReserverdMemory(uio_lib::MM_ADC, 64 * memory->getMemoryBlockSize());
        board = std::make_shared<board_lib::CDummyBoard>();
        hw = uio_lib::CHardware::create(board);
        server = std::make_shared<ADCStreamServer>(hw);
        recorder = std::make_shared<Recorder>();
        server->setCallback(recorder);
        dir = std::filesystem::temp_directory_path() / ("adc_server_" + std::to_string(::getpid()));
        std::filesystem::remove_all(dir);
        std::filesystem::create_directories(dir);
        options.filePath = dir.string();
    }

    // a new hardware object, after a change of the dummy state its features depend on
    void recreate() {
        server = nullptr;
        hw = uio_lib::CHardware::create(board);
        server = std::make_shared<ADCStreamServer>(hw);
        server->setCallback(recorder);
    }

    void TearDown() override {
        server = nullptr;
        std::filesystem::remove_all(dir);
        uio_lib::dummy::resetOscState();
    }

    static auto fileSettings(CStreamSettings& s, uint64_t samples) -> CStreamSettings& {
        s.setADCPassMode(CStreamSettings::PassMode::FILE);
        s.setADCFormat(CStreamSettings::DataFormat::BIN);
        s.setADCSamples(samples);
        s.setADCDecimation(64);
        s.setADCChannels(1, CStreamSettings::State::ON);
        s.setADCChannels(2, CStreamSettings::State::OFF);
        return s;
    }

    std::shared_ptr<board_lib::CDummyBoard> board;
    uio_lib::CHardware::Ptr hw;
    ADCStreamServer::Ptr server;
    std::shared_ptr<Recorder> recorder;
    std::filesystem::path dir;
    ADCStreamOptions options;
};

}  // namespace

TEST_F(ADCServer, AFileWithASampleLimitEndsDone) {
    CStreamSettings s;
    ASSERT_TRUE(server->start(fileSettings(s, 20000), options));
    EXPECT_EQ(recorder->events, std::vector<std::string>{"sd"});
    EXPECT_EQ(uio_lib::dummy::oscState().decimation, 64u) << "the scope configured through the facade";
    server->startADC();
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->reasons[0], ADCStopReason::DONE);
    bool written = false;
    for (auto& f : std::filesystem::directory_iterator(dir))
        written = written || (f.path().extension() == ".bin" && std::filesystem::file_size(f.path()) > 0);
    EXPECT_TRUE(written) << "a BIN file in " << dir;
    EXPECT_FALSE(uio_lib::dummy::oscState().running) << "the scope stopped";
}

// A recording in parts (streaming_improvements 4.11): the parts are in the folder, numbered, and
// the record names the part written last with its number; the time limit ends it as done
TEST_F(ADCServer, AFileInPartsEndsAtItsTimeLimitWithItsPartsNumbered) {
    CStreamSettings s;
    fileSettings(s, 0);
    CStreamSettings::FileLimits l;
    l.time = 0.01;
    l.splitTime = 0.002;
    ASSERT_TRUE(s.setADCLimits(l));
    ASSERT_TRUE(server->start(s, options));
    server->startADC();
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->reasons[0], ADCStopReason::DONE);
    std::vector<std::string> parts;
    for (auto& f : std::filesystem::directory_iterator(dir))
        if (f.path().extension() == ".bin")
            parts.push_back(f.path().filename().string());
    std::sort(parts.begin(), parts.end());
    ASSERT_GE(parts.size(), 2u) << "10 ms in parts of 2 ms";
    for (size_t k = 0; k < parts.size(); k++) {
        char n[16];
        snprintf(n, sizeof(n), "_%04d.bin", static_cast<int>(k + 1));
        EXPECT_NE(parts[k].find(n), std::string::npos) << parts[k];
    }
    const auto r = server->getRecord();
    EXPECT_EQ(r.part, static_cast<int>(parts.size()));
    EXPECT_EQ(std::filesystem::path(r.file).filename().string(), parts.back());
    EXPECT_EQ(r.limits.time, 0.01);
    EXPECT_EQ(r.limits.splitTime, 0.002);
    bin_lib::FileReader fr;
    uint64_t samples = 0;
    for (auto& p : parts) {
        ASSERT_TRUE(fr.open((dir / p).string())) << p;
        samples += fr.samples(0, true);
    }
    // 10 ms at 125 MHz / 64, taken at a buffer
    EXPECT_GE(samples, 19531u);
}

// The losses the page shows (item 1.4 of the streaming improvements): what the FPGA reports lost
// before each buffer is summed per channel with the samples it gave; kept after the stop, reset
// by the next start. The file took every buffer: nothing dropped by its queue.
TEST_F(ADCServer, TheLossesOfARunAreCountedKeptAndResetByTheNextStart) {
    uio_lib::dummy::oscState().lost = [](uint8_t, uint8_t ch) -> uint64_t { return ch == 0 ? 7 : 0; };
    CStreamSettings s;
    ASSERT_TRUE(server->start(fileSettings(s, 20000), options));
    server->startADC();
    ASSERT_TRUE(recorder->waitStopped());
    const uint64_t samples = server->getBoardSamples();
    const uint64_t lost = server->getBoardLost();
    ASSERT_GT(samples, 0u);
    const uint64_t perBuffer = samples / (lost / 7);
    EXPECT_EQ(lost % 7, 0u) << "7 per buffer";
    EXPECT_EQ(lost / 7 * perBuffer, samples) << "every buffer counted once: " << lost / 7 << " buffers of " << perBuffer;
    EXPECT_EQ(server->getFileDropped(), 0u);
    EXPECT_EQ(server->getBoardLost(), lost) << "kept after the stop";
    // the record of the run (item 1.5): the file it wrote, kept after the stop
    const auto rec = server->getRecord();
    EXPECT_EQ(rec.mode, 2);
    EXPECT_FALSE(rec.running);
    EXPECT_EQ(rec.limit, 20000u);
    EXPECT_GE(rec.samples, 20000u) << "the limit is reached";
    EXPECT_GT(rec.written, 0u);
    EXPECT_EQ(rec.file.rfind(dir.string(), 0), 0u) << rec.file;

    uio_lib::dummy::oscState().lost = nullptr;
    ASSERT_TRUE(server->start(fileSettings(s, 20000), options));
    EXPECT_EQ(server->getBoardLost(), 0u) << "reset by the start";
    EXPECT_EQ(server->getBoardSamples(), 0u);
    server->stop(ADCStopReason::NORMAL);
}

TEST_F(ADCServer, NoChannelsStopsWithItsReason) {
    CStreamSettings s;
    fileSettings(s, 1000);
    s.setADCChannels(1, CStreamSettings::State::OFF);
    EXPECT_FALSE(server->start(s, options));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->reasons[0], ADCStopReason::NO_CHANNELS);
}

TEST_F(ADCServer, AStopIsReportedAlsoWhenNothingRuns) {
    server->stop(ADCStopReason::NORMAL);
    ASSERT_EQ(recorder->reasons.size(), 1u);
    EXPECT_EQ(recorder->reasons[0], ADCStopReason::NORMAL);
}

TEST_F(ADCServer, AnUnreadableCalibrationIsAConfigurationError) {
    board->calibrationReadable = false;
    CStreamSettings s;
    EXPECT_FALSE(server->start(fileSettings(s, 1000), options));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->reasons[0], ADCStopReason::CONFIG_ERROR);
}

TEST_F(ADCServer, AStopInTheMiddleEndsTheRun) {
    CStreamSettings s;
    ASSERT_TRUE(server->start(fileSettings(s, 0), options));  // no limit
    server->startADC();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(server->isRunning());
    server->stop(ADCStopReason::NORMAL);
    EXPECT_FALSE(server->isRunning());
    ASSERT_EQ(recorder->reasons.size(), 1u);
    EXPECT_EQ(recorder->reasons[0], ADCStopReason::NORMAL);
    EXPECT_FALSE(uio_lib::dummy::oscState().running);
}

TEST_F(ADCServer, ASecondStartOfTheADCIsIgnored) {
    CStreamSettings s;
    ASSERT_TRUE(server->start(fileSettings(s, 0), options));
    server->startADC();
    server->startADC();  // a client that repeats the FPGA start command
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(server->isRunning());
    server->stop(ADCStopReason::NORMAL);
    EXPECT_FALSE(server->isRunning());
    ASSERT_EQ(recorder->reasons.size(), 1u);
    EXPECT_EQ(recorder->reasons[0], ADCStopReason::NORMAL);
}

TEST_F(ADCServer, AStopLeftFromAnEarlierSessionDoesNotStopTheNextOne) {
    // a start that fails stops from a thread of its own; the next start comes at once
    for (int i = 0; i < 20; i++) {
        CStreamSettings bad;
        fileSettings(bad, 0);
        bad.setADCChannels(1, CStreamSettings::State::OFF);
        EXPECT_FALSE(server->start(bad, options));
        CStreamSettings s;
        ASSERT_TRUE(server->start(fileSettings(s, 0), options));
        server->startADC();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        ASSERT_TRUE(server->isRunning()) << "run " << i << ": the stop of the failed start ended the new session";
        {
            std::lock_guard lock(recorder->mtx);
            for (auto r : recorder->reasons)
                EXPECT_NE(r, ADCStopReason::NORMAL) << "run " << i;
        }
        server->stop(ADCStopReason::NORMAL);
        std::lock_guard lock(recorder->mtx);
        recorder->reasons.clear();
        recorder->events.clear();
    }
}

TEST_F(ADCServer, TheTriggerSettingsReachTheScope) {
    uio_lib::dummy::oscState().blockMode = true;
    recreate();
    CStreamSettings s;
    fileSettings(s, 0);
    s.setADCTriggerMode(CStreamSettings::State::ON);
    s.setADCTriggerSources(uio_lib::TRIG_SRC_ADC_CH1 | uio_lib::TRIG_SRC_EXT);
    s.setADCTriggerLevelLow(-100);
    s.setADCTriggerLevelHigh(200);
    s.setADCTriggerEdge(CStreamSettings::TriggerEdge::FALLING);
    s.setADCTriggerBlocks(2);
    ASSERT_TRUE(server->start(s, options));
    auto& st = uio_lib::dummy::oscState();
    EXPECT_TRUE(st.triggerMode);
    EXPECT_EQ(st.triggerSources, uio_lib::TRIG_SRC_ADC_CH1 | uio_lib::TRIG_SRC_EXT);
    EXPECT_EQ(st.triggerLow, -100);
    EXPECT_EQ(st.triggerHigh, 200);
    EXPECT_TRUE(st.triggerFalling);
    EXPECT_EQ(st.triggerBlocks, 2);
    server->stop(ADCStopReason::NORMAL);
}

TEST_F(ADCServer, WithoutTheTriggerModeTheScopeKeepsItsDefaultSources) {
    CStreamSettings s;
    fileSettings(s, 0);
    s.setADCTriggerSources(uio_lib::TRIG_SRC_LA);  // kept in the settings, the mode off
    ASSERT_TRUE(server->start(s, options));
    EXPECT_FALSE(uio_lib::dummy::oscState().triggerMode);
    EXPECT_TRUE(uio_lib::dummy::oscState().triggerSourcesDefault);
    server->stop(ADCStopReason::NORMAL);
}

TEST_F(ADCServer, AWrongTriggerLevelIsAConfigurationError) {
    CStreamSettings s;
    fileSettings(s, 0);
    s.setADCTriggerMode(CStreamSettings::State::ON);
    s.setADCTriggerLevelLow(10);
    s.setADCTriggerLevelHigh(-10);
    EXPECT_FALSE(server->start(s, options));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->reasons[0], ADCStopReason::CONFIG_ERROR);
}

namespace {

auto connectTo(asio::io_context& io, uint16_t port) -> std::unique_ptr<asio::ip::tcp::socket> {
    auto sock = std::make_unique<asio::ip::tcp::socket>(io);
    for (int i = 0; i < 100; i++) {
        asio::error_code ec;
        sock->connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), port), ec);
        if (!ec)
            return sock;
        sock->close(ec);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return nullptr;
}

auto netSettings(CStreamSettings& s) -> CStreamSettings& {
    s.setADCPassMode(CStreamSettings::PassMode::NET);
    s.setADCChannels(1, CStreamSettings::State::ON);
    return s;
}

}  // namespace

TEST_F(ADCServer, ANetworkClientThatLeavesStopsTheStreaming) {
    CStreamSettings s;
    ASSERT_TRUE(server->start(netSettings(s)));
    server->startADC();
    asio::io_context io;
    auto client = connectTo(io, NET_ADC_STREAMING_PORT);
    ASSERT_TRUE(client);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    {
        std::lock_guard lock(recorder->mtx);
        EXPECT_TRUE(recorder->reasons.empty()) << "streaming while the client is there";
    }
    client->close();
    ASSERT_TRUE(recorder->waitStopped());
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::lock_guard lock(recorder->mtx);
    EXPECT_EQ(recorder->reasons, std::vector<ADCStopReason>{ADCStopReason::DISCONNECTED});
}

TEST_F(ADCServer, AStopWithTheClientConnectedIsReportedOnce) {
    CStreamSettings s;
    ASSERT_TRUE(server->start(netSettings(s)));
    server->startADC();
    asio::io_context io;
    auto client = connectTo(io, NET_ADC_STREAMING_PORT);
    ASSERT_TRUE(client);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    server->stop(ADCStopReason::NORMAL);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::lock_guard lock(recorder->mtx);
    EXPECT_EQ(recorder->reasons, std::vector<ADCStopReason>{ADCStopReason::NORMAL}) << "the disconnect of our own stop is not a second stop";
}

// A start that throws (here the data port is taken) says why, as the other failed starts do: the
// page and the clients got no answer at all (streaming_improvements 1.6)
TEST_F(ADCServer, AStartWhoseDataPortIsTakenIsAFailedStart) {
    asio::io_context io;
    asio::ip::tcp::acceptor taken(io);
    const asio::ip::tcp::endpoint at(asio::ip::make_address("127.0.0.3"), NET_ADC_STREAMING_PORT);
    taken.open(at.protocol());
    taken.bind(at);
    taken.listen();
    CStreamSettings s;
    ADCStreamOptions o;
    o.host = "127.0.0.3";
    EXPECT_FALSE(server->start(netSettings(s), o));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->reasons, std::vector<ADCStopReason>{ADCStopReason::CONFIG_ERROR});
}

TEST_F(ADCServer, TheDataPortListensOnTheAddressGiven) {
    CStreamSettings s;
    ADCStreamOptions o;
    o.host = "127.0.0.2";
    ASSERT_TRUE(server->start(netSettings(s), o));
    asio::io_context io;
    asio::ip::tcp::socket a(io), b(io);
    asio::error_code ec1, ec2;
    a.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.2"), NET_ADC_STREAMING_PORT), ec1);
    b.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), NET_ADC_STREAMING_PORT), ec2);
    EXPECT_FALSE(ec1) << ec1.message();
    EXPECT_TRUE(ec2) << "not on another address";
    a.close();
    server->stop(ADCStopReason::NORMAL);
}
