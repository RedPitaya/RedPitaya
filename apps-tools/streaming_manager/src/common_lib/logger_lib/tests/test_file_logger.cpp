/**
 * CFileLogger - the files written next to a recording: <name>.lost.txt with one
 * line per buffer and the report <name>.txt written at the stop.
 *
 * The regress (streamtest.py) reads the report with the pattern "FPGA:\s*(\d+)",
 * one match per channel; the tests keep that shape.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <thread>

#include "data_lib/network_header.h"
#include "data_lib/tests/support.h"
#include "logger_lib/file_logger.h"

using data_test::Arena;
using data_test::MakeBuffer;
using DataLib::CDataBuffersPackDMA;

namespace {

constexpr size_t kBlock = 64;  // 32 samples of 16 bit

class FileLoggerTest : public ::testing::Test {
   protected:
    void SetUp() override {
        auto info = ::testing::UnitTest::GetInstance()->current_test_info();
        m_dir = std::filesystem::path(::testing::TempDir()) / (std::string("file_logger_") + info->name());
        std::filesystem::remove_all(m_dir);
        std::filesystem::create_directories(m_dir);
        m_base = (m_dir / "rec.bin.log").string();
    }
    void TearDown() override { std::filesystem::remove_all(m_dir); }

    auto read(const std::string& path) -> std::string {
        std::ifstream f(path);
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }
    auto lostFile() -> std::string { return read(m_base + ".lost.txt"); }
    auto report() -> std::string { return read(m_base + ".txt"); }

    // A pack of two channels whose buffers carry the given gaps and properties.
    auto makePack(uint64_t lost, uint64_t wait, uint32_t decimation, uint8_t flags, uint32_t triggers) -> CDataBuffersPackDMA::Ptr {
        auto pack = CDataBuffersPackDMA::Create();
        for (auto ch : {DataLib::CH1, DataLib::CH2}) {
            auto buf = MakeBuffer(m_arena, m_next++ % 8, 16);
            buf->setLostSamples(DataLib::FPGA, lost);
            buf->setLostSamples(DataLib::WAIT, wait);
            buf->setDecimation(decimation);
            buf->setADCFlags(flags);
            buf->setTriggerCount(triggers);
            buf->setADCBaseRate(15625000);  // the rate after the decimation 8
            pack->addBuffer(ch, buf);
        }
        return pack;
    }

    // What streaming_file does for every pack.
    auto log(CFileLogger& logger, const CDataBuffersPackDMA::Ptr& pack) -> void {
        for (auto ch : {DataLib::CH1, DataLib::CH2}) {
            auto buf = pack->getBuffer(ch);
            logger.addMetric(CFileLogger::EMetric::RECIVE_DATE, buf->getDataLenght());
            logger.addMetric(ch, buf->getDataLenght(), buf->getLostSamples(DataLib::FPGA), buf->getWaitSamples(), buf->getSamplesCount());
        }
        logger.addMetric(CFileLogger::EMetric::OSC_RATE, 15625000);
        logger.addMetric(pack);
    }

    std::filesystem::path m_dir;
    std::string m_base;
    Arena m_arena{8, kBlock};
    size_t m_next = 0;
};

// The lost file pads its columns; the content checks compare with single spaces.
auto squash(const std::string& line) -> std::string {
    return std::regex_replace(line, std::regex(" +"), " ");
}

auto lines(const std::string& text) -> std::vector<std::string> {
    std::vector<std::string> out;
    std::stringstream ss(text);
    for (std::string l; std::getline(ss, l);)
        out.push_back(l);
    return out;
}

}  // namespace

TEST_F(FileLoggerTest, TheLostFileShowsTheLossAndTheWaitOfEveryBuffer) {
    {
        CFileLogger logger(m_base, false);
        log(logger, makePack(0, 0, 8, 0, 0));
        log(logger, makePack(5, 0, 8, 0, 0));
        log(logger, makePack(0, 100, 8, 0, 0));
        log(logger, makePack(0, 0, 8, 0, 0));
        logger.dumpToFile();
    }
    auto l = lines(lostFile());
    ASSERT_EQ(l.size(), 5u);
    EXPECT_EQ(l[0].rfind("# ", 0), 0u) << "a legend first";
    EXPECT_EQ(squash(l[1]), "Channel 1: Pos 0 Get: 32 (0) Wait: 0 Channel 2: Pos 0 Get: 32 (0) Wait: 0");
    EXPECT_EQ(squash(l[2]), "Channel 1: Pos 32 Get: 32 (5) Wait: 0 Channel 2: Pos 32 Get: 32 (5) Wait: 0");
    EXPECT_EQ(squash(l[3]), "Channel 1: Pos 69 Get: 32 (0) Wait: 100 Channel 2: Pos 69 Get: 32 (0) Wait: 100");
    // the position is on the time line: the wait moves it like the loss does
    EXPECT_EQ(squash(l[4]), "Channel 1: Pos 201 Get: 32 (0) Wait: 0 Channel 2: Pos 201 Get: 32 (0) Wait: 0");
}

TEST_F(FileLoggerTest, TheTriggerCounterIsLoggedOnlyInTriggerMode) {
    {
        CFileLogger logger(m_base, false);
        log(logger, makePack(0, 10, 8, DataLib::ADC_FLAG_TRIGGER_MODE, 3));
        log(logger, makePack(0, 0, 8, 0, 4));
        logger.dumpToFile();
    }
    auto l = lines(lostFile());
    ASSERT_EQ(l.size(), 3u);
    EXPECT_NE(squash(l[1]).find("Wait: 10 Trig: 3"), std::string::npos) << l[1];
    EXPECT_EQ(l[2].find("Trig"), std::string::npos) << l[2];
}

TEST_F(FileLoggerTest, TheLegendIsNotRepeatedInAnAppendedFile) {
    for (int run = 0; run < 2; run++) {
        CFileLogger logger(m_base, false);
        log(logger, makePack(0, 0, 8, 0, 0));
        logger.dumpToFile();
    }
    auto l = lines(lostFile());
    ASSERT_EQ(l.size(), 3u);
    EXPECT_EQ(l[0][0], '#');
    EXPECT_NE(l[2][0], '#');
}

TEST_F(FileLoggerTest, TheReportKeepsTheWaitApartFromTheLoss) {
    {
        CFileLogger logger(m_base, false);
        log(logger, makePack(5, 100, 8, DataLib::ADC_FLAG_TRIGGER_MODE | DataLib::ADC_FLAG_WAIT_COUNTER, 7));
        log(logger, makePack(2, 50, 8, DataLib::ADC_FLAG_TRIGGER_MODE | DataLib::ADC_FLAG_WAIT_COUNTER, 9));
        logger.dumpToFile();
    }
    auto r = report();
    const std::regex regressPattern(R"(FPGA:\s*(\d+))");
    std::vector<std::string> fpga;
    for (auto it = std::sregex_iterator(r.begin(), r.end(), regressPattern); it != std::sregex_iterator(); ++it)
        fpga.push_back((*it)[1]);
    EXPECT_EQ(fpga, (std::vector<std::string>{"7", "7"})) << "the regress pattern, one match per channel";
    EXPECT_NE(r.find("Wait for a trigger on: Channel 1\n\t- 150 Samples"), std::string::npos) << r;
    EXPECT_NE(r.find("Wait for a trigger on: Channel 2\n\t- 150 Samples"), std::string::npos) << r;
    EXPECT_NE(r.find("Decimation:\t8\n"), std::string::npos) << r;
    EXPECT_NE(r.find("Sample rate:\t15625000\n"), std::string::npos) << r;
    EXPECT_NE(r.find("ADC rate:\t125000000\n"), std::string::npos) << "the buffers carry the rate after the decimation\n" << r;
    EXPECT_NE(r.find("Trigger mode:\ton\n"), std::string::npos) << r;
    EXPECT_NE(r.find("after the first and the last buffer:\t7 / 9\n"), std::string::npos) << r;
    EXPECT_NE(r.find("apart from the loss:\tyes\n"), std::string::npos) << r;
    EXPECT_NE(r.find("Buffers:\t2\n"), std::string::npos) << r;
}

TEST_F(FileLoggerTest, WithoutTheAcquisitionPropertiesTheReportSaysSo) {
    {
        CFileLogger logger(m_base, false);
        log(logger, makePack(0, 0, 0, 0, 0));
        logger.dumpToFile();
    }
    auto r = report();
    EXPECT_NE(r.find("Decimation, trigger mode:\tnot reported\n"), std::string::npos) << r;
    EXPECT_EQ(r.find("Trigger mode:"), std::string::npos) << r;
}

TEST_F(FileLoggerTest, TheTestModeWritesNoLostFile) {
    {
        CFileLogger logger(m_base, true);
        log(logger, makePack(1, 1, 8, 0, 0));
    }
    EXPECT_FALSE(std::filesystem::exists(m_base + ".lost.txt"));
}

TEST_F(FileLoggerTest, TheFileLossCanBeReadWhileTheWriterCounts) {
    CFileLogger logger(m_base, true);
    std::atomic<bool> done{false};
    std::thread writer([&] {
        for (int i = 0; i < 100000; i++)
            logger.addMetric(CFileLogger::EMetric::FILESYSTEM_RATE, 1);
        done = true;
    });
    uint64_t last = 0;
    while (!done) {
        auto now = logger.getFileLost();
        EXPECT_GE(now, last);
        last = now;
    }
    writer.join();
    EXPECT_EQ(logger.getFileLost(), 100000u);
}

TEST_F(FileLoggerTest, GpioBuffersGetTheirOwnLinesOnTheirOwnTimeLine) {
    {
        CFileLogger logger(m_base, false);
        logger.setGpioProperties({125000000, 1, true, true, true});
        log(logger, makePack(0, 0, 8, 0, 0));
        logger.addGpioMetric(4096, 1024, 70000, 0, 0, 1);
        logger.addGpioMetric(4096, 1024, 50000, 3, 400000, 2);
        logger.addGpioMetric(4096, 1024, 60000, 0, 0, 2);
        logger.dumpToFile();
    }
    auto l = lines(lostFile());
    ASSERT_EQ(l.size(), 5u);
    EXPECT_EQ(squash(l[1]).rfind("Channel 1: Pos 0 ", 0), 0u) << l[1];
    EXPECT_EQ(squash(l[2]), "GPIO: Pos 0 Get: 70000 (0) Wait: 0 Words: 1024 Trig: 1");
    EXPECT_EQ(squash(l[3]), "GPIO: Pos 70000 Get: 50000 (3) Wait: 400000 Words: 1024 Trig: 2");
    EXPECT_EQ(squash(l[4]), "GPIO: Pos 520003 Get: 60000 (0) Wait: 0 Words: 1024 Trig: 2");
}

TEST_F(FileLoggerTest, TheReportHasAGpioPartThatTheRegressPatternSkips) {
    {
        CFileLogger logger(m_base, false);
        logger.setGpioProperties({125000000, 4, true, true, true});
        log(logger, makePack(1, 0, 8, 0, 0));
        logger.addGpioMetric(4096, 1024, 70000, 5, 100, 7);
        logger.addGpioMetric(4096, 1024, 50000, 2, 200, 9);
        logger.dumpToFile();
    }
    auto r = report();
    const std::regex regressPattern(R"(FPGA:\s*(\d+))");
    EXPECT_EQ(std::distance(std::sregex_iterator(r.begin(), r.end(), regressPattern), std::sregex_iterator()), 2) << r;
    EXPECT_NE(r.find("Current ADC speed:"), std::string::npos) << r;
    EXPECT_NE(r.find("GPIO input:\n\tBase rate:\t125000000\n\tDecimation:\t4\n\tSample rate:\t31250000.000\n\tRLE:\ton\n\tTrigger mode:\ton\n"),
              std::string::npos)
        << r;
    EXPECT_NE(r.find("\tFPGA trigger counter after the first and the last buffer:\t7 / 9\n"), std::string::npos) << r;
    EXPECT_NE(r.find("on: GPIO\n\t-120000 Samples\n\t-2048 Words\n\t-8192b"), std::string::npos) << r;
    EXPECT_NE(r.find("\t- FPGA overflow:\t7 Samples\n\tWait for a trigger on: GPIO\n\t- 300 Samples\n"), std::string::npos) << r;
}

TEST_F(FileLoggerTest, AGpioOnlyReportHasNoAdcPart) {
    {
        CFileLogger logger(m_base, false);
        logger.setGpioProperties({125000000, 1, false, false, true});
        logger.addGpioMetric(4096, 1024, 1024, 0, 0, 0);
        logger.dumpToFile();
    }
    auto r = report();
    EXPECT_EQ(r.find("ADC speed"), std::string::npos) << r;
    EXPECT_EQ(r.find("trigger counter"), std::string::npos) << "no trigger counter outside the trigger mode";
    EXPECT_NE(r.find("\tRLE:\toff\n\tTrigger mode:\toff\n"), std::string::npos) << r;
    EXPECT_EQ(squash(lines(lostFile())[1]), "GPIO: Pos 0 Get: 1024 (0) Wait: 0 Words: 1024");
}

TEST_F(FileLoggerTest, TheColumnsOfTheLostFileAreUnderEachOther) {
    {
        CFileLogger logger(m_base, false);
        logger.setGpioProperties({125000000, 1, true, true, true});
        log(logger, makePack(0, 0, 8, DataLib::ADC_FLAG_TRIGGER_MODE, 1));
        log(logger, makePack(123456789, 98765432101ull, 8, DataLib::ADC_FLAG_TRIGGER_MODE, 2));
        log(logger, makePack(0, 0, 8, DataLib::ADC_FLAG_TRIGGER_MODE, 3));
        logger.addGpioMetric(4096, 1024, 12345678, 7, 3100000, 4);
    }
    auto l = lines(lostFile());
    ASSERT_EQ(l.size(), 5u);
    for (auto key : {"Pos ", "Get:", "Wait:"}) {
        auto at = l[1].find(key);
        ASSERT_NE(at, std::string::npos);
        for (size_t i = 2; i < l.size(); i++)
            EXPECT_EQ(l[i].find(key), at) << key << " in line " << i << ": " << l[i];
    }
    // the second channel and the trigger counter too, for the ADC lines
    for (auto key : {"Channel 2", "Trig:"}) {
        auto at = l[1].find(key);
        for (size_t i = 2; i < 4; i++)
            EXPECT_EQ(l[i].find(key), at) << key << " in line " << i << ": " << l[i];
    }
    EXPECT_EQ(l[1].find('\t'), std::string::npos) << "spaces only, tabs depend on the viewer";
}
