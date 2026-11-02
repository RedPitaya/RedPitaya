/**
 * ADCStreamServer, the ADC streaming of a board over CHardware: the server of the host build runs
 * on the dummy scope (a buffer full every 10 ms) and writes files; what it reports, when and why
 * it stops.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>
#include <vector>

#include "board_lib/board.h"
#include "streaming_lib/adc_stream_server.h"
#include "streaming_lib/trigger_commands.h"
#include "uio_lib/memory_manager.h"
#include "uio_lib/oscilloscope_dummy.h"
#include "uio_lib/trigger.h"

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

TEST_F(ADCServer, TheTriggerStatusComesFromTheHardware) {
    auto& st = uio_lib::dummy::oscState();
    st.armed = true;
    st.triggerCount = []() -> uint32_t { return 5; };
    auto status = triggerStatusOf(*hw);
    EXPECT_TRUE(status.adcArmed);
    EXPECT_EQ(status.adcTriggerCount, 5u);
}
