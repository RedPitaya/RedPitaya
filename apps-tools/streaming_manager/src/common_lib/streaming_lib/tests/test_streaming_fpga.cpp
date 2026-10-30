/**
 * CStreamingFPGA puts into the header of every buffer what the scope latched for it: the loss
 * (overflow) apart from the trigger wait, and the run constants (decimation, ADC clock, trigger
 * mode and blocks, flags). The dummy COscilloscope plays the FPGA: each "full buffer" is a step
 * with the counters the real core would have latched.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "data_lib/network_header.h"
#include "data_lib/tests/support.h"
#include "streaming_lib/streaming_fpga.h"
#include "uio_lib/oscilloscope_dummy.h"

using DataLib::CDataBuffersPackDMA;

namespace {

constexpr std::size_t kHeader = 128;
constexpr std::size_t kSamples = 512;  // 16 bit samples of a channel buffer

// A full DMA buffer as the scope sees it
struct Step {
    uint64_t lost[2];  // getBufferLost() of CH1, CH2: overflow and wait together
    uint64_t wait;     // getBufferWait()
    bool freePack;     // a free pack to map in its place, else the buffer is dropped
};

// What a delivered buffer carries
struct Seen {
    uint64_t lost[2];
    uint64_t wait[2];
    uint32_t decimation;
    uint32_t adcClock;
    uint32_t triggerCount;
    uint16_t triggerBlocks;
    uint8_t flags;
};

class FPGARun {
   public:
    explicit FPGARun(std::vector<Step> steps) : m_steps(std::move(steps)), m_arena(64, kHeader + kSamples * 2) {}

    ~FPGARun() { uio_lib::dummy::resetOscState(); }

    // Runs every step through a CStreamingFPGA; the buffers it delivered, in order
    auto run(bool triggerMode, bool waitCounter, uint16_t blocks) -> std::vector<Seen> {
        auto& s = uio_lib::dummy::oscState();
        s.triggerMode = triggerMode;
        s.waitCounter = waitCounter;
        s.triggerBlocks = blocks;
        s.wait = [this]() {
            std::unique_lock lock(m_mtx);
            if (m_next >= m_steps.size()) {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                return false;
            }
            m_current = m_next++;
            return true;
        };
        s.lost = [this](uint8_t, uint8_t ch) { return ch < 2 ? m_steps[m_current].lost[ch] : 0; };
        s.waitCount = [this](uint8_t) { return m_steps[m_current].wait; };
        s.triggerCount = [this]() { return static_cast<uint32_t>(100 + m_current); };

        auto osc = uio_lib::COscilloscope::create(uio_lib::UioT(), 64, true, 62500000, false, 16, 2);
        auto fpga = std::make_shared<streaming_lib::CStreamingFPGA>(osc, false);
        fpga->setTestMode(true);
        fpga->getBuffF = [this]() -> CDataBuffersPackDMA::Ptr {
            // the first two map the DMA buffers, then one per full buffer
            if (m_packs >= 2 && !m_steps[m_current].freePack)
                return nullptr;
            return makePack();
        };
        fpga->unlockBuffF = []() {};
        fpga->oscNotify.connect([this](CDataBuffersPackDMA::Ptr pack) {
            std::lock_guard lock(m_mtx);
            if (pack)
                m_seen.push_back(seenOf(pack));
            m_done++;
            m_cv.notify_all();
        });
        fpga->runNonBlock();
        {
            std::unique_lock lock(m_mtx);
            m_cv.wait_for(lock, std::chrono::seconds(10), [this]() { return m_done == m_steps.size(); });
        }
        fpga->stop();
        return m_seen;
    }

   private:
    auto makePack() -> CDataBuffersPackDMA::Ptr {
        auto pack = CDataBuffersPackDMA::Create();
        pack->addBuffer(DataLib::CH1, data_test::MakeBuffer(m_arena, 2 * m_packs, 16, kHeader));
        pack->addBuffer(DataLib::CH2, data_test::MakeBuffer(m_arena, 2 * m_packs + 1, 16, kHeader));
        m_packs++;
        return pack;
    }

    static auto seenOf(CDataBuffersPackDMA::Ptr pack) -> Seen {
        Seen r{};
        for (int ch = 0; ch < 2; ch++) {
            auto b = pack->getBuffer((DataLib::EDataBuffersPackChannel)ch);
            r.lost[ch] = b->getLostSamples(DataLib::FPGA);
            r.wait[ch] = b->getWaitSamples();
        }
        auto b = pack->getBuffer(DataLib::CH1);
        r.decimation = b->getDecimation();
        r.adcClock = b->getADCClock();
        r.triggerCount = b->getTriggerCount();
        r.triggerBlocks = b->getTriggerBlocks();
        r.flags = b->getADCFlags();
        return r;
    }

    std::vector<Step> m_steps;
    data_test::Arena m_arena;
    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::size_t m_next = 0;
    std::size_t m_current = 0;
    std::size_t m_packs = 0;
    std::size_t m_done = 0;
    std::vector<Seen> m_seen;
};

}  // namespace

TEST(StreamingFPGA, TheTriggerWaitIsApartFromTheLoss) {
    FPGARun run({
        {{10, 12}, 0, true},       // overflow only
        {{5000, 3000}, 4000, true},  // a series gap of 4000; CH2 latched less than the wait
    });
    auto seen = run.run(true, true, 3);
    ASSERT_EQ(seen.size(), 2u);
    EXPECT_EQ(seen[0].lost[0], 10u);
    EXPECT_EQ(seen[0].lost[1], 12u);
    EXPECT_EQ(seen[0].wait[0], 0u);
    EXPECT_EQ(seen[1].lost[0], 1000u) << "the loss without the wait";
    EXPECT_EQ(seen[1].lost[1], 0u) << "never below 0";
    EXPECT_EQ(seen[1].wait[0], 4000u);
    EXPECT_EQ(seen[1].wait[1], 4000u) << "one wait for all channels";
}

TEST(StreamingFPGA, ADroppedBufferIsLostAsAWholeAndItsWaitStaysAPause) {
    FPGARun run({
        {{0, 0}, 0, true},
        {{0, 0}, 0, true},
        {{307, 309}, 300, false},  // no free pack: this buffer is dropped
        {{52, 54}, 50, true},
    });
    auto seen = run.run(true, true, 0);
    ASSERT_EQ(seen.size(), 3u);
    EXPECT_EQ(seen[2].lost[0], 2u + 7u + kSamples) << "its own loss, the dropped one's and its samples";
    EXPECT_EQ(seen[2].lost[1], 4u + 9u + kSamples);
    EXPECT_EQ(seen[2].wait[0], 50u + 300u);
}

TEST(StreamingFPGA, TheRunConstantsAreInEveryBuffer) {
    FPGARun run({
        {{0, 0}, 0, true},
        {{0, 0}, 0, true},
    });
    auto seen = run.run(true, true, 3);
    ASSERT_EQ(seen.size(), 2u);
    for (std::size_t i = 0; i < seen.size(); i++) {
        EXPECT_EQ(seen[i].decimation, 64u);
        EXPECT_EQ(seen[i].adcClock, 62500000u) << "the exact rate is 62.5 MHz / 64, the integer one 976562";
        EXPECT_EQ(seen[i].triggerBlocks, 3);
        EXPECT_EQ(seen[i].flags, DataLib::ADC_FLAG_TRIGGER_MODE | DataLib::ADC_FLAG_WAIT_COUNTER);
        EXPECT_EQ(seen[i].triggerCount, 100u + i) << "read for each buffer";
    }
}

TEST(StreamingFPGA, WithoutTheTriggerModeAndTheWaitCounterTheFlagsAreClear) {
    FPGARun run({{{3, 3}, 0, true}});
    auto seen = run.run(false, false, 0);
    ASSERT_EQ(seen.size(), 1u);
    EXPECT_EQ(seen[0].flags, 0);
    EXPECT_EQ(seen[0].lost[0], 3u) << "an older bitstream: the whole counter is the loss";
    EXPECT_EQ(seen[0].wait[0], 0u);
}
