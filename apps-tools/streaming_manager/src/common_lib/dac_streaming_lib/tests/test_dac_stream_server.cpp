/**
 * DACStreamServer, the DAC streaming of a board over CHardware: the server of the host build plays
 * WAV files into the dummy generator, which takes a buffer into one of its two slots; what it
 * reports, when and why it stops, and that a pack stays out of the ring while the FPGA holds it.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#include "board_lib/board.h"
#include "dac_streaming_lib/dac_stream_server.h"
#include "dac_streaming_lib/dac_streaming_application.h"
#include "dac_streaming_lib/dac_streaming_manager.h"
#include "data_lib/network_header.h"
#include "uio_lib/generator_dummy.h"
#include "uio_lib/memory_manager.h"

using namespace dac_streaming_lib;
using namespace std::chrono_literals;

namespace {

constexpr uint32_t kBlock = 4096;  // bytes of a channel in a pack
constexpr uint32_t kBlockSamples = kBlock / 2;
constexpr uint32_t kPacks = 12;  // the ring, one channel

class Recorder : public DACServerCallback {
   public:
    void startedTCP(DACStreamServer*) override { add("tcp"); }
    void startedSD(DACStreamServer*) override { add("sd"); }
    void stopped(DACStreamServer*, DACStopReason r) override {
        std::lock_guard lock(mtx);
        reasons.push_back(r);
        events.push_back("stopped");
        cv.notify_all();
    }

    auto waitStopped(int seconds = 10) -> bool {
        std::unique_lock lock(mtx);
        return cv.wait_for(lock, std::chrono::seconds(seconds), [this]() { return !reasons.empty(); });
    }

    auto stops() -> std::vector<DACStopReason> {
        std::lock_guard lock(mtx);
        return reasons;
    }

    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::string> events;
    std::vector<DACStopReason> reasons;

   private:
    void add(const std::string& e) {
        std::lock_guard lock(mtx);
        events.push_back(e);
    }
};

template <typename T>
auto Le(T v) -> std::string {
    return std::string(reinterpret_cast<const char*>(&v), sizeof(T));
}

// A mono 16 bit WAV file, sample i is i
auto writeRamp(const std::filesystem::path& path, uint32_t samples) -> void {
    std::string data;
    for (uint32_t i = 0; i < samples; i++)
        data += Le<int16_t>(static_cast<int16_t>(i));
    std::string fmt = Le<uint16_t>(1) + Le<uint16_t>(1) + Le<uint32_t>(1000) + Le<uint32_t>(2000) + Le<uint16_t>(2) + Le<uint16_t>(16);
    std::string body = "WAVE" + std::string("fmt ") + Le<uint32_t>(static_cast<uint32_t>(fmt.size())) + fmt + "data" + Le<uint32_t>(static_cast<uint32_t>(data.size())) + data;
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    auto all = "RIFF" + Le<uint32_t>(static_cast<uint32_t>(body.size())) + body;
    out.write(all.data(), static_cast<std::streamsize>(all.size()));
}

// The samples of a buffer the generator was given, by its DMA address
auto samplesAt(uint32_t address) -> const int16_t* {
    for (const auto& r : uio_lib::CMemoryManager::instance()->getRegions(uio_lib::MM_DAC)) {
        if (address == r.start + DataLib::sizeHeader())
            return reinterpret_cast<const int16_t*>(r.startMemory + DataLib::sizeHeader());
    }
    return nullptr;
}

class DACServer : public ::testing::Test {
   protected:
    void SetUp() override {
        auto memory = uio_lib::CMemoryManager::instance();
        memory->setMemoryBlockSize(kBlock);
        memory->reallocateBlocks();
        memory->setReserverdMemory(uio_lib::MM_DAC, kPacks * memory->getMemoryBlockSize());
        board = std::make_shared<board_lib::CDummyBoard>();
        hw = uio_lib::CHardware::create(board);
        uio_lib::dummy::resetGenState();
        server = std::make_shared<DACStreamServer>(hw);
        recorder = std::make_shared<Recorder>();
        server->setCallback(recorder);
        dir = std::filesystem::temp_directory_path() / ("dac_server_" + std::to_string(::getpid()));
        std::filesystem::remove_all(dir);
        std::filesystem::create_directories(dir);
        options.dirPath = dir.string();
    }

    void TearDown() override {
        server = nullptr;
        std::filesystem::remove_all(dir);
        uio_lib::dummy::resetGenState();
    }

    static auto fileSettings(CStreamSettings& s, const std::string& name, CStreamSettings::DACRepeat repeat = CStreamSettings::DACRepeat::DAC_REP_OFF) -> CStreamSettings& {
        s.setDACPassMode(CStreamSettings::DACPassMode::DAC_FILE);
        s.setDACFileType(CStreamSettings::DataFormat::WAV);
        s.setDACFile(name);
        s.setDACRepeat(repeat);
        s.setMemoryBlockSize(kBlock);
        return s;
    }

    std::shared_ptr<board_lib::CDummyBoard> board;
    uio_lib::CHardware::Ptr hw;
    DACStreamServer::Ptr server;
    std::shared_ptr<Recorder> recorder;
    std::filesystem::path dir;
    DACStreamOptions options;
};

}  // namespace

TEST_F(DACServer, AFileIsPlayedInOrderAndEndsDone) {
    constexpr uint32_t samples = 7 * kBlockSamples + 1000;
    writeRamp(dir / "ramp.wav", samples);
    std::mutex mtx;
    std::vector<int16_t> firsts;
    uio_lib::dummy::genState().taken = [&](uint8_t, uint32_t ch1, uint32_t, uint32_t) {
        auto s = samplesAt(ch1);
        std::lock_guard lock(mtx);
        firsts.push_back(s ? s[0] : -1);
    };

    CStreamSettings s;

    ASSERT_TRUE(server->start(fileSettings(s, "ramp.wav"), dac_channels_t(), options));
    ASSERT_TRUE(recorder->waitStopped());

    EXPECT_EQ(recorder->events, (std::vector<std::string>{"sd", "stopped"}));
    EXPECT_EQ(recorder->stops(), std::vector<DACStopReason>{DACStopReason::DONE});
    std::lock_guard lock(mtx);
    ASSERT_EQ(firsts.size(), 8u);
    for (size_t i = 0; i < firsts.size(); i++)
        EXPECT_EQ(firsts[i], static_cast<int16_t>(i * kBlockSamples)) << "pack " << i;
    EXPECT_FALSE(uio_lib::dummy::genState().running);
}

// The FPGA reads the buffer of a slot until the slot takes the next but one: the reader of the
// file must not write into the two packs taken last
TEST_F(DACServer, APackStaysOutOfTheRingWhileTheFPGAHoldsIt) {
    writeRamp(dir / "ramp.wav", 7 * kBlockSamples + 1000);
    struct Taken {
        const int16_t* data;
        std::vector<int16_t> copy;
    };
    std::mutex mtx;
    std::vector<Taken> taken;
    int overwritten = 0;
    uio_lib::dummy::genState().taken = [&](uint8_t, uint32_t ch1, uint32_t, uint32_t) {
        std::this_thread::sleep_for(1ms);  // the generator is slower than the reader: the ring is full
        std::lock_guard lock(mtx);
        for (size_t back = 1; back <= 2 && back <= taken.size(); back++) {
            auto& t = taken[taken.size() - back];
            if (std::memcmp(t.data, t.copy.data(), kBlock) != 0)
                overwritten++;
        }
        auto s = samplesAt(ch1);
        if (s)
            taken.push_back({s, std::vector<int16_t>(s, s + kBlockSamples)});
    };

    CStreamSettings s;

    ASSERT_TRUE(server->start(fileSettings(s, "ramp.wav", CStreamSettings::DACRepeat::DAC_REP_INF), dac_channels_t(), options));
    std::this_thread::sleep_for(500ms);
    server->stop(DACStopReason::NORMAL);

    std::lock_guard lock(mtx);
    EXPECT_GT(taken.size(), 3 * kPacks);
    EXPECT_EQ(overwritten, 0);
}

TEST_F(DACServer, AMissingFileIsReportedMissing) {
    CStreamSettings s;
    EXPECT_FALSE(server->start(fileSettings(s, "none.wav"), dac_channels_t(), options));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->events, std::vector<std::string>{"stopped"});
    EXPECT_EQ(recorder->stops(), std::vector<DACStopReason>{DACStopReason::MISSING_FILE});
}

TEST_F(DACServer, AFileThatIsNotASoundIsBroken) {
    std::ofstream(dir / "text.wav") << "not a sound at all";
    CStreamSettings s;
    EXPECT_FALSE(server->start(fileSettings(s, "text.wav"), dac_channels_t(), options));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<DACStopReason>{DACStopReason::BROKEN});
}

TEST_F(DACServer, ANetworkStartWithoutChannelsStopsWithItsReason) {
    CStreamSettings s;
    s.setDACPassMode(CStreamSettings::DACPassMode::DAC_NET);
    s.setMemoryBlockSize(kBlock);
    EXPECT_FALSE(server->start(s, dac_channels_t(), options));
    ASSERT_TRUE(recorder->waitStopped());
    EXPECT_EQ(recorder->stops(), std::vector<DACStopReason>{DACStopReason::NO_CHANNELS});
}

TEST_F(DACServer, AStopIsReportedAlsoWhenNothingRuns) {
    server->stop(DACStopReason::NORMAL);
    EXPECT_EQ(recorder->stops(), std::vector<DACStopReason>{DACStopReason::NORMAL});
}

TEST_F(DACServer, AStopInTheMiddleIsReportedOnce) {
    writeRamp(dir / "ramp.wav", 7 * kBlockSamples + 1000);
    CStreamSettings s;
    ASSERT_TRUE(server->start(fileSettings(s, "ramp.wav", CStreamSettings::DACRepeat::DAC_REP_INF), dac_channels_t(), options));
    std::this_thread::sleep_for(100ms);
    EXPECT_TRUE(server->isRunning());
    server->stop(DACStopReason::NORMAL);
    std::this_thread::sleep_for(300ms);
    EXPECT_EQ(recorder->stops(), std::vector<DACStopReason>{DACStopReason::NORMAL});
    EXPECT_FALSE(server->isRunning());
    EXPECT_FALSE(uio_lib::dummy::genState().running);
}

// The file has been read to the end but the FPGA has not played it yet when the client stops:
// the end found by the stopping generator is not reported after the stop
TEST_F(DACServer, TheEndOfAFileFoundByTheStopIsNotReported) {
    writeRamp(dir / "ramp.wav", 3 * kBlockSamples + 1000);
    std::atomic<int> taken{0};
    uio_lib::dummy::genState().slotFree = [&](uint8_t) { return taken < 2; };  // the FPGA plays the first two for ever
    uio_lib::dummy::genState().taken = [&](uint8_t, uint32_t, uint32_t, uint32_t) { taken++; };

    CStreamSettings s;

    ASSERT_TRUE(server->start(fileSettings(s, "ramp.wav"), dac_channels_t(), options));
    std::this_thread::sleep_for(200ms);
    server->stop(DACStopReason::NORMAL);
    std::this_thread::sleep_for(300ms);
    EXPECT_EQ(recorder->stops(), std::vector<DACStopReason>{DACStopReason::NORMAL});
}

TEST_F(DACServer, ASecondStartPlaysAgain) {
    writeRamp(dir / "ramp.wav", 3 * kBlockSamples + 1000);
    CStreamSettings s;
    ASSERT_TRUE(server->start(fileSettings(s, "ramp.wav", CStreamSettings::DACRepeat::DAC_REP_INF), dac_channels_t(), options));
    std::this_thread::sleep_for(50ms);
    CStreamSettings s2;
    ASSERT_TRUE(server->start(fileSettings(s2, "ramp.wav"), dac_channels_t(), options));
    ASSERT_TRUE(recorder->waitStopped());
    std::this_thread::sleep_for(100ms);
    EXPECT_EQ(recorder->stops(), std::vector<DACStopReason>{DACStopReason::DONE});
}

// A stop that comes before the thread of the generator runs is not lost: it ends the playback
TEST_F(DACServer, AStopRightAfterTheStartOfTheApplicationEndsIt) {
    writeRamp(dir / "ramp.wav", 7 * kBlockSamples + 1000);
    auto memory = uio_lib::CMemoryManager::instance();
    for (int i = 0; i < 20; i++) {
        auto manager = CDACStreamingManager::Create(CDACStreamingManager::WAV_TYPE, (dir / "ramp.wav").string(), CStreamSettings::DACRepeat::DAC_REP_INF, 0, kBlock, false);
        dac_channels_t channels;
        manager->getChannels(channels);
        memory->releaseMemory(uio_lib::MM_DAC);
        ASSERT_GT(memory->reserveMemory(uio_lib::MM_DAC, kPacks, channels.count()), 0u);
        manager->getBufferManager()->generateBuffersEmptyDAC(channels, memory->getRegions(uio_lib::MM_DAC), DataLib::sizeHeader());
        CDACStreamingApplication app(manager, hw);
        auto t0 = std::chrono::steady_clock::now();
        app.runNonBlock();
        app.stop();
        EXPECT_LT(std::chrono::steady_clock::now() - t0, 3s) << "run " << i;
    }
    memory->releaseMemory(uio_lib::MM_DAC);
}
