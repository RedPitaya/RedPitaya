/**
 * CBuffersCached - the ring of packs between the DMA writer and the consumer.
 *
 * The ring is guarded by two counting semaphores rather than by the indices:
 * m_spacesem starts at the ring size and m_countsem at zero, so writeBuffer
 * blocks when the ring is full and readBuffer blocks when it is empty. Handing a
 * pack back is a SEPARATE call - unlockBufferWrite after writing, unlockBufferRead
 * after reading - and forgetting one stalls the other side. Every test below
 * therefore always pairs them.
 *
 * The blocking cases really do block: readBuffer waits one second before giving
 * up, and so does writeBuffer(true). Two tests here pay that second on purpose,
 * because a ring buffer whose full/empty behaviour is untested is not tested.
 */

#include <gtest/gtest.h>

#include <pthread.h>
#include <signal.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <future>
#include <thread>
#include <new>
#include <cstdint>
#include <set>
#include <vector>

#include "data_lib/buffers_cached.h"
#include "data_lib/network_header.h"
#include "support.h"

using data_test::Arena;
using DataLib::CBuffersCached;
using DataLib::CDataBufferDMA;
using DataLib::CDataBuffersPackDMA;

namespace {

constexpr std::size_t kBlockSize = 1024;

// A cache with `channels` channels over `ringSize` packs, i.e. an arena of
// channels * ringSize blocks.
struct Fixture {
    Fixture(std::size_t channels, std::size_t ringSize, std::size_t headerSize = 0, bool testMode = false)
        : arena(channels * ringSize, kBlockSize), cache(CBuffersCached::create()) {
        for (std::size_t i = 0; i < channels; i++) {
            cache->addChannel(static_cast<DataLib::EDataBuffersPackChannel>(i), 16, CDataBufferDMA::ATT_1_1);
        }
        cache->generateBuffers(arena.regions(), headerSize, testMode);
    }

    Arena arena;
    CBuffersCached::Ptr cache;
};

auto Elapsed(const std::chrono::steady_clock::time_point& from) -> double {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - from).count();
}

}  // namespace

TEST(BuffersCached, GenerateBuffersCarvesTheBlocksIntoPacks) {
    Fixture fixture(2, 3);

    // 6 blocks / 2 channels = a ring of 3 packs, each holding both channels.
    EXPECT_EQ(fixture.cache->getDataSize(), kBlockSize) << "no header, so the data area is the whole block";
    EXPECT_TRUE(fixture.cache->isEmpty());

    std::set<CDataBuffersPackDMA*> seen;
    for (int i = 0; i < 3; i++) {
        auto pack = fixture.cache->writeBuffer(true);
        ASSERT_NE(pack, nullptr) << "pack " << i;
        EXPECT_TRUE(pack->isChannelPresent(DataLib::CH1));
        EXPECT_TRUE(pack->isChannelPresent(DataLib::CH2));
        EXPECT_FALSE(pack->isChannelPresent(DataLib::CH3));
        EXPECT_EQ(pack->getBuffer(DataLib::CH1)->getBufferFullLenght(), kBlockSize);
        EXPECT_EQ(pack->getBuffer(DataLib::CH1)->getBitBySample(), 16);
        EXPECT_EQ(pack->getBuffer(DataLib::CH1)->getADCMode(), CDataBufferDMA::ATT_1_1);
        seen.insert(pack.get());
        fixture.cache->unlockBufferWrite();
    }
    EXPECT_EQ(seen.size(), 3u) << "the ring must hand out three distinct packs";
}

TEST(BuffersCached, AHeaderSizeShrinksTheDataAreaAndIsZeroed) {
    Fixture fixture(1, 2, DataLib::sizeHeader());

    EXPECT_EQ(fixture.cache->getDataSize(), kBlockSize - DataLib::sizeHeader());

    auto pack = fixture.cache->writeBuffer(true);
    ASSERT_NE(pack, nullptr);
    auto buffer = pack->getBuffer(DataLib::CH1);
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->getHeaderLenght(), DataLib::sizeHeader());
    EXPECT_EQ(buffer->getDataLenght(), kBlockSize - DataLib::sizeHeader());
    EXPECT_EQ(static_cast<uint8_t*>(buffer->getMappedDataMemory()), static_cast<uint8_t*>(buffer->getMappedMemory()) + DataLib::sizeHeader());
    for (std::size_t i = 0; i < DataLib::sizeHeader(); i++) {
        EXPECT_EQ(static_cast<uint8_t*>(buffer->getMappedMemory())[i], 0) << "header byte " << i;
    }
    fixture.cache->unlockBufferWrite();
}

TEST(BuffersCached, TestModeFillsTheDataAreaWithARampAndLeavesTheHeaderAlone) {
    Fixture fixture(1, 1, DataLib::sizeHeader(), true);

    auto pack = fixture.cache->writeBuffer(true);
    ASSERT_NE(pack, nullptr);
    auto buffer = pack->getBuffer(DataLib::CH1);
    const uint8_t* data = static_cast<const uint8_t*>(buffer->getMappedDataMemory());
    for (std::size_t i = 0; i < buffer->getDataLenght(); i++) {
        EXPECT_EQ(data[i], static_cast<uint8_t>(i & 0xFF)) << "data byte " << i;
    }
    for (std::size_t i = 0; i < DataLib::sizeHeader(); i++) {
        EXPECT_EQ(static_cast<const uint8_t*>(buffer->getMappedMemory())[i], 0) << "header byte " << i;
    }
    fixture.cache->unlockBufferWrite();
}

TEST(BuffersCached, WritingThenReadingHandsBackTheSamePackInOrder) {
    Fixture fixture(1, 3);

    std::vector<CDataBuffersPackDMA*> written;
    for (int i = 0; i < 3; i++) {
        auto pack = fixture.cache->writeBuffer(true);
        ASSERT_NE(pack, nullptr) << i;
        written.push_back(pack.get());
        fixture.cache->unlockBufferWrite();
    }
    EXPECT_FALSE(fixture.cache->isEmpty());

    for (int i = 0; i < 3; i++) {
        auto pack = fixture.cache->readBuffer();
        ASSERT_NE(pack, nullptr) << i;
        EXPECT_EQ(pack.get(), written[static_cast<std::size_t>(i)]) << "the ring is FIFO; index " << i;
        fixture.cache->unlockBufferRead();
    }
    EXPECT_TRUE(fixture.cache->isEmpty());
}

TEST(BuffersCached, TheRingWrapsAroundAndKeepsPairingWritesWithReads) {
    Fixture fixture(1, 2);

    // Ten passes over a ring of two: every read must see what the matching
    // write produced.
    for (int i = 0; i < 10; i++) {
        auto written = fixture.cache->writeBuffer(true);
        ASSERT_NE(written, nullptr) << i;
        auto buffer = written->getBuffer(DataLib::CH1);
        ASSERT_NE(buffer, nullptr);
        static_cast<uint8_t*>(buffer->getMappedDataMemory())[0] = static_cast<uint8_t>(i);
        fixture.cache->unlockBufferWrite();

        auto read = fixture.cache->readBuffer();
        ASSERT_NE(read, nullptr) << i;
        EXPECT_EQ(read.get(), written.get()) << "pass " << i;
        EXPECT_EQ(static_cast<uint8_t*>(read->getBuffer(DataLib::CH1)->getMappedDataMemory())[0], static_cast<uint8_t>(i));
        fixture.cache->unlockBufferRead();
    }
    EXPECT_TRUE(fixture.cache->isEmpty());
}

TEST(BuffersCached, IsEmptyTracksTheUnreadCount) {
    Fixture fixture(1, 3);
    EXPECT_TRUE(fixture.cache->isEmpty());

    ASSERT_NE(fixture.cache->writeBuffer(true), nullptr);
    EXPECT_TRUE(fixture.cache->isEmpty()) << "taking a pack to fill does not make it readable";
    fixture.cache->unlockBufferWrite();
    EXPECT_FALSE(fixture.cache->isEmpty()) << "unlockBufferWrite is what publishes it";

    ASSERT_NE(fixture.cache->readBuffer(), nullptr);
    EXPECT_TRUE(fixture.cache->isEmpty());
    fixture.cache->unlockBufferRead();
    EXPECT_TRUE(fixture.cache->isEmpty());
}

// fullPercent returns a FRACTION in [0, 1), not a percentage, and it is derived
// from the two indices rather than from the semaphores - so it counts packs
// handed out for writing, not packs waiting to be read.
TEST(BuffersCached, FullPercentIsAFractionOfTheRing) {
    Fixture fixture(1, 4);
    EXPECT_FLOAT_EQ(fixture.cache->fullPercent(), 0.0f);

    ASSERT_NE(fixture.cache->writeBuffer(true), nullptr);
    fixture.cache->unlockBufferWrite();
    EXPECT_FLOAT_EQ(fixture.cache->fullPercent(), 0.25f);

    ASSERT_NE(fixture.cache->writeBuffer(true), nullptr);
    fixture.cache->unlockBufferWrite();
    EXPECT_FLOAT_EQ(fixture.cache->fullPercent(), 0.5f);

    ASSERT_NE(fixture.cache->readBuffer(), nullptr);
    fixture.cache->unlockBufferRead();
    EXPECT_FLOAT_EQ(fixture.cache->fullPercent(), 0.25f);
}

TEST(BuffersCached, AFullRingRefusesAFurtherWriteAfterTheTimeout) {
    Fixture fixture(1, 2);
    for (int i = 0; i < 2; i++) {
        ASSERT_NE(fixture.cache->writeBuffer(true), nullptr) << i;
        fixture.cache->unlockBufferWrite();
    }

    const auto started = std::chrono::steady_clock::now();
    EXPECT_EQ(fixture.cache->writeBuffer(true), nullptr) << "the ring is full";
    const double waited = Elapsed(started);
    EXPECT_GE(waited, 0.9) << "writeBuffer(true) waits about a second before giving up";
    EXPECT_LT(waited, 5.0);

    // Freeing one slot lets the next write through immediately.
    ASSERT_NE(fixture.cache->readBuffer(), nullptr);
    fixture.cache->unlockBufferRead();
    EXPECT_NE(fixture.cache->writeBuffer(true), nullptr);
    fixture.cache->unlockBufferWrite();
}

TEST(BuffersCached, AnEmptyRingRefusesAReadAfterTheTimeout) {
    Fixture fixture(1, 2);

    const auto started = std::chrono::steady_clock::now();
    EXPECT_EQ(fixture.cache->readBuffer(), nullptr) << "nothing has been published yet";
    const double waited = Elapsed(started);
    EXPECT_GE(waited, 0.9) << "readBuffer waits about a second before giving up";
    EXPECT_LT(waited, 5.0);
}

TEST(BuffersCached, AnEmptyBlockListProducesARingThatHandsOutNothing) {
    auto cache = CBuffersCached::create();
    cache->addChannel(DataLib::CH1, 16, CDataBufferDMA::ATT_1_1);
    cache->generateBuffers({}, 0, false);

    EXPECT_EQ(cache->getDataSize(), 0u);
    EXPECT_EQ(cache->writeBuffer(true), nullptr) << "a zero-sized ring short-circuits before the semaphore";
    EXPECT_EQ(cache->readBuffer(), nullptr);
    EXPECT_FLOAT_EQ(cache->fullPercent(), 0.0f);
}

TEST(BuffersCached, TheDestroyFlagIsStickyAndStartsClear) {
    auto cache = CBuffersCached::create();
    EXPECT_FALSE(cache->isWaitToDestory());
    EXPECT_TRUE(cache->notifyToDestory());
    EXPECT_TRUE(cache->isWaitToDestory());
    EXPECT_TRUE(cache->notifyToDestory());
    EXPECT_TRUE(cache->isWaitToDestory());
}

// The board stops the FPGA thread first; it may wait in writeBuffer() for a pack that only the
// network sender frees, and that one may be blocked on a client that stopped reading.
TEST(BuffersCached, DestroyReleasesAWriterBlockedOnAFullRing) {
    Fixture fixture(1, 2);
    for (int i = 0; i < 2; i++) {
        ASSERT_NE(fixture.cache->writeBuffer(), nullptr) << i;
        fixture.cache->unlockBufferWrite();
    }
    auto writer = std::async(std::launch::async, [&] { return fixture.cache->writeBuffer(); });
    ASSERT_EQ(writer.wait_for(std::chrono::milliseconds(200)), std::future_status::timeout) << "the ring is full";
    fixture.cache->notifyToDestory();
    if (writer.wait_for(std::chrono::seconds(2)) != std::future_status::ready) {
        fixture.cache->readBuffer();  // free a pack, else the future waits for the writer forever
        fixture.cache->unlockBufferRead();
        FAIL() << "the destroy did not release the writer";
    }
    EXPECT_EQ(writer.get(), nullptr) << "no pack after the destroy";
    const auto started = std::chrono::steady_clock::now();
    EXPECT_EQ(fixture.cache->writeBuffer(), nullptr) << "and none for a later writer while the ring is full";
    EXPECT_LT(Elapsed(started), 0.5) << "the writer gives up within a wait slice";
    EXPECT_EQ(fixture.cache->writeBuffer(true), nullptr);
}

// A stop right after the start: the FPGA thread takes its first two packs only then, and a
// missing one is fatal there. Free packs are handed out after the destroy as before.
TEST(BuffersCached, AfterDestroyAFreePackIsStillHandedOut) {
    Fixture fixture(1, 3);
    fixture.cache->notifyToDestory();
    EXPECT_NE(fixture.cache->writeBuffer(), nullptr);
    fixture.cache->unlockBufferWrite();
    EXPECT_NE(fixture.cache->writeBuffer(true), nullptr);
    fixture.cache->unlockBufferWrite();
}

TEST(BuffersCached, AfterDestroyTheReaderStillGetsThePacksWrittenBefore) {
    Fixture fixture(1, 3);
    auto written = fixture.cache->writeBuffer();
    ASSERT_NE(written, nullptr);
    fixture.cache->unlockBufferWrite();
    fixture.cache->notifyToDestory();
    EXPECT_EQ(fixture.cache->readBuffer(), written);
    fixture.cache->unlockBufferRead();
    EXPECT_EQ(fixture.cache->readBuffer(), nullptr) << "nothing else was written";
}

namespace {

void IgnoreSignal(int) {}

// Interrupts the waits of `thread` with SIGUSR1 a few times; sem_wait is never restarted.
void Interrupt(pthread_t thread) {
    struct sigaction sa = {};
    sa.sa_handler = IgnoreSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, nullptr);
    for (int i = 0; i < 5; i++) {
        pthread_kill(thread, SIGUSR1);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

}  // namespace

TEST(BuffersCached, ASignalDoesNotEndTheWaitOfAWriter) {
    Fixture fixture(1, 2);
    for (int i = 0; i < 2; i++) {
        ASSERT_NE(fixture.cache->writeBuffer(), nullptr) << i;
        fixture.cache->unlockBufferWrite();
    }
    std::atomic<bool> done{false};
    CDataBuffersPackDMA::Ptr got;
    std::thread writer([&] {
        got = fixture.cache->writeBuffer();
        done = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Interrupt(writer.native_handle());
    EXPECT_FALSE(done) << "EINTR is not a free pack";
    ASSERT_NE(fixture.cache->readBuffer(), nullptr);
    fixture.cache->unlockBufferRead();
    writer.join();
    EXPECT_NE(got, nullptr);
}

TEST(BuffersCached, ASignalDoesNotEndTheWaitOfAReader) {
    Fixture fixture(1, 2);
    std::atomic<bool> done{false};
    CDataBuffersPackDMA::Ptr got;
    std::thread reader([&] {
        got = fixture.cache->readBuffer();
        done = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Interrupt(reader.native_handle());
    EXPECT_FALSE(done) << "the reader waits its second";
    ASSERT_NE(fixture.cache->writeBuffer(), nullptr);
    fixture.cache->unlockBufferWrite();
    reader.join();
    EXPECT_NE(got, nullptr);
}

TEST(BuffersCached, SettingTheRateAndTheAdcWidthReachesEveryPack) {
    Fixture fixture(2, 3);

    fixture.cache->setOSCRate(125000000ull);
    fixture.cache->setADCBits(12);

    for (int i = 0; i < 3; i++) {
        auto pack = fixture.cache->writeBuffer(true);
        ASSERT_NE(pack, nullptr) << i;
        for (auto channel : {DataLib::CH1, DataLib::CH2}) {
            auto buffer = pack->getBuffer(channel);
            ASSERT_NE(buffer, nullptr);
            EXPECT_EQ(buffer->getADCBaseRate(), 125000000ull) << "pack " << i;
            EXPECT_EQ(buffer->getADCBaseBits(), 12) << "pack " << i;
        }
        fixture.cache->unlockBufferWrite();
    }
}

TEST(BuffersCached, InitHeadersAdcStampsEveryBufferInEveryPack) {
    Fixture fixture(2, 2, DataLib::sizeHeader());
    fixture.cache->setOSCRate(125000000ull);
    fixture.cache->setADCBits(16);

    EXPECT_TRUE(fixture.cache->initHeadersADC());

    for (int i = 0; i < 2; i++) {
        auto pack = fixture.cache->writeBuffer(true);
        ASSERT_NE(pack, nullptr) << i;
        for (auto channel : {DataLib::CH1, DataLib::CH2}) {
            auto buffer = pack->getBuffer(channel);
            ASSERT_NE(buffer, nullptr);
            const auto* header = reinterpret_cast<const DataLib::NetworkPackHeader*>(buffer->getMappedMemory());
            EXPECT_EQ(std::memcmp(header->ID_PACK, DataLib::ID_PACK_ADC, 16), 0) << "pack " << i;
            EXPECT_EQ(header->bufferSize, kBlockSize);
            EXPECT_EQ(header->adc.baseRateOSC, 125000000u);
            EXPECT_EQ(header->adc.baseOSCBits, 16);
            EXPECT_EQ(header->adc.sizeOfAllChannels, pack->getLenghtDataBuffers());
        }
        fixture.cache->unlockBufferWrite();
    }
}

TEST(BuffersCached, GenerateBuffersEmptyDacUsesTheChannelSetAndEightBitBuffers) {
    Arena arena(4, kBlockSize);
    auto cache = CBuffersCached::create();

    dac_channels_t channels;
    channels.enable(DACChannels::DAC_CH1);
    channels.enable(DACChannels::DAC_CH2);
    cache->generateBuffersEmptyDAC(channels, arena.regions(), DataLib::sizeHeader());

    EXPECT_EQ(cache->getDataSize(), kBlockSize - DataLib::sizeHeader());
    EXPECT_TRUE(cache->initHeadersDAC(channels));

    for (int i = 0; i < 2; i++) {
        auto pack = cache->writeBuffer(true);
        ASSERT_NE(pack, nullptr) << i;
        EXPECT_TRUE(pack->isChannelPresent(DataLib::CH1));
        EXPECT_TRUE(pack->isChannelPresent(DataLib::CH2));
        EXPECT_FALSE(pack->isChannelPresent(DataLib::CH3));
        EXPECT_EQ(pack->getBuffer(DataLib::CH1)->getBitBySample(), 8) << "generateBuffersEmpty hardcodes 8 bits";
        const auto* header = reinterpret_cast<const DataLib::NetworkPackHeader*>(pack->getBuffer(DataLib::CH1)->getMappedMemory());
        EXPECT_EQ(std::memcmp(header->ID_PACK, DataLib::ID_PACK_DAC, 16), 0);
        EXPECT_EQ(header->dac.channels, 2);
        cache->unlockBufferWrite();
    }
}

TEST(BuffersCached, GenerateBuffersEmptyAdcMapsAllFourChannels) {
    Arena arena(8, kBlockSize);
    auto cache = CBuffersCached::create();

    adc_channels_t channels;
    channels.enableAll();
    cache->generateBuffersEmptyADC(channels, arena.regions(), 0);

    EXPECT_EQ(cache->getDataSize(), kBlockSize);
    for (int i = 0; i < 2; i++) {
        auto pack = cache->writeBuffer(true);
        ASSERT_NE(pack, nullptr) << i;
        for (auto channel : {DataLib::CH1, DataLib::CH2, DataLib::CH3, DataLib::CH4}) {
            EXPECT_TRUE(pack->isChannelPresent(channel)) << "pack " << i;
        }
        cache->unlockBufferWrite();
    }
}

TEST(BuffersCached, AChannelSubsetOnlyProducesThoseChannels) {
    Arena arena(4, kBlockSize);
    auto cache = CBuffersCached::create();

    adc_channels_t channels;
    channels.enable(ADCChannels::ADC_CH2);
    channels.enable(ADCChannels::ADC_CH4);
    cache->generateBuffersEmptyADC(channels, arena.regions(), 0);

    auto pack = cache->writeBuffer(true);
    ASSERT_NE(pack, nullptr);
    EXPECT_FALSE(pack->isChannelPresent(DataLib::CH1));
    EXPECT_TRUE(pack->isChannelPresent(DataLib::CH2));
    EXPECT_FALSE(pack->isChannelPresent(DataLib::CH3));
    EXPECT_TRUE(pack->isChannelPresent(DataLib::CH4));
    cache->unlockBufferWrite();
}

// m_ringStart, m_ringEnd and m_ringSize were missing from the constructor's
// initialiser list and the two semaphores were only sem_init'd inside
// generateBuffers, so a cache that was created but never populated - which is
// exactly what happens when generateBuffers bails out on "No active channels" -
// answered out of indeterminate memory, and isEmpty() called sem_getvalue on an
// uninitialised semaphore.
//
// Placement-new over a poisoned block is what makes the index half of that
// deterministic instead of luck: on a fresh heap allocation the bytes are often
// already zero, so the same object built with make_shared can look healthy. The
// pattern varies per byte on purpose - with one repeated byte the three ring
// fields read the SAME garbage and the arithmetic in fullPercent cancels to 0,
// hiding exactly what this test looks for.
TEST(BuffersCached, TheRingIndicesAreInitialisedByTheConstructor) {
    alignas(CBuffersCached) unsigned char storage[sizeof(CBuffersCached)];
    for (std::size_t i = 0; i < sizeof(storage); i++) {
        storage[i] = static_cast<unsigned char>(i * 31u + 7u);
    }

    auto* cache = new (static_cast<void*>(storage)) CBuffersCached();
    EXPECT_EQ(cache->getDataSize(), 0u);
    EXPECT_FLOAT_EQ(cache->fullPercent(), 0.0f) << "an unpopulated ring is 0% full, not a random fraction";
    EXPECT_FALSE(cache->isWaitToDestory());
    cache->~CBuffersCached();
}

// The same state reached the ordinary way. Every one of these calls used to read
// indeterminate memory, and isEmpty() an uninitialised semaphore; none of them
// may block or fault now.
TEST(BuffersCached, AnUnpopulatedCacheIsSafeToQuery) {
    auto cache = CBuffersCached::create();
    EXPECT_EQ(cache->getDataSize(), 0u);
    EXPECT_FLOAT_EQ(cache->fullPercent(), 0.0f);
    EXPECT_TRUE(cache->isEmpty());
    EXPECT_EQ(cache->writeBuffer(true), nullptr);
    EXPECT_EQ(cache->readBuffer(), nullptr);
}

// generateBuffers logs "No active channels" and returns without building a ring
// when addChannel was never called; the object has to stay in that same safe
// state afterwards.
TEST(BuffersCached, ARejectedGenerateLeavesTheCacheSafeToQuery) {
    Arena arena(2, kBlockSize);
    auto cache = CBuffersCached::create();
    cache->generateBuffers(arena.regions(), 0, false);  // no addChannel

    EXPECT_EQ(cache->getDataSize(), 0u);
    EXPECT_FLOAT_EQ(cache->fullPercent(), 0.0f);
    EXPECT_TRUE(cache->isEmpty());
    EXPECT_EQ(cache->writeBuffer(true), nullptr);
    EXPECT_EQ(cache->readBuffer(), nullptr);
}

// generateBuffers re-arms the semaphores for the ring it just built. The
// constructor has already initialised them, and POSIX leaves sem_init on an
// initialised semaphore undefined, so they are destroyed first - and the ring
// still has to behave afterwards.
TEST(BuffersCached, TheSemaphoresAreRearmedForTheGeneratedRing) {
    Fixture fixture(1, 2);

    EXPECT_TRUE(fixture.cache->isEmpty());
    for (int i = 0; i < 2; i++) {
        ASSERT_NE(fixture.cache->writeBuffer(true), nullptr) << i;
        fixture.cache->unlockBufferWrite();
    }
    EXPECT_FALSE(fixture.cache->isEmpty()) << "two published packs";
    for (int i = 0; i < 2; i++) {
        ASSERT_NE(fixture.cache->readBuffer(), nullptr) << i;
        fixture.cache->unlockBufferRead();
    }
    EXPECT_TRUE(fixture.cache->isEmpty());
}
