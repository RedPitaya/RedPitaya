/**
 * CDataBufferDMA - one DMA block plus the bookkeeping the streaming path hangs
 * off it.
 *
 * The class carries two address spaces at once and it is easy to mix them up,
 * so both are pinned side by side: getBuffer()/getHeaderAddress()/
 * getDataAddress() are DEVICE addresses the library only ever stores and
 * offsets, while getMappedMemory()/getMappedDataMemory() are the host pointers
 * it writes through. initHeaderAddress() has to move both consistently.
 *
 * Ownership differs per constructor and is not visible in the signature: the
 * (uint32_t, size_t, void*, uint8_t) overload borrows the mapping, the
 * (uint8_t*, size_t, uint8_t) overload ADOPTS the pointer and delete[]s it.
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <new>
#include <type_traits>

#include "support.h"

using data_test::Arena;
using data_test::kDeviceBase;
using DataLib::CDataBufferDMA;

TEST(DataBufferDMA, IsNeitherCopyableNorMovable) {
    static_assert(!std::is_copy_constructible_v<CDataBufferDMA>);
    static_assert(!std::is_copy_assignable_v<CDataBufferDMA>);
    static_assert(!std::is_move_constructible_v<CDataBufferDMA>);
    static_assert(!std::is_move_assignable_v<CDataBufferDMA>);
    SUCCEED();
}

TEST(DataBufferDMA, TheMappedConstructorStoresBothAddressSpaces) {
    Arena arena(1, 4096);
    const auto& region = arena.regions()[0];
    auto buffer = CDataBufferDMA::Create(region.start, region.size, region.startMemory, 16);

    EXPECT_EQ(buffer->getBuffer(), kDeviceBase);
    EXPECT_EQ(buffer->getBufferFullLenght(), 4096u);
    EXPECT_EQ(buffer->getBitBySample(), 16);

    // With no header the data starts where the buffer starts.
    EXPECT_EQ(buffer->getHeaderAddress(), 0u) << "the header address stays 0 until initHeaderAddress runs";
    EXPECT_EQ(buffer->getHeaderLenght(), 0u);
    EXPECT_EQ(buffer->getDataAddress(), kDeviceBase);
    EXPECT_EQ(buffer->getDataLenght(), 4096u);
    EXPECT_EQ(buffer->getMappedMemory(), region.startMemory);
    EXPECT_EQ(buffer->getMappedDataMemory(), region.startMemory);
}

TEST(DataBufferDMA, InitHeaderAddressMovesTheDeviceAddressAndThePointerTogether) {
    Arena arena(1, 4096);
    const auto& region = arena.regions()[0];
    auto buffer = CDataBufferDMA::Create(region.start, region.size, region.startMemory, 16);

    buffer->initHeaderAddress(128);

    EXPECT_EQ(buffer->getHeaderAddress(), kDeviceBase) << "the header sits at the start of the block";
    EXPECT_EQ(buffer->getHeaderLenght(), 128u);
    EXPECT_EQ(buffer->getDataAddress(), kDeviceBase + 128u);
    EXPECT_EQ(buffer->getDataLenght(), 4096u - 128u);
    EXPECT_EQ(buffer->getBufferFullLenght(), 4096u) << "the full length still covers the header";
    EXPECT_EQ(buffer->getMappedMemory(), region.startMemory);
    EXPECT_EQ(buffer->getMappedDataMemory(), region.startMemory + 128);

    // Calling it again re-bases from the buffer start, not from the previous
    // data address, so it is idempotent for a given header size.
    buffer->initHeaderAddress(128);
    EXPECT_EQ(buffer->getDataAddress(), kDeviceBase + 128u);
    EXPECT_EQ(buffer->getMappedDataMemory(), region.startMemory + 128);

    buffer->initHeaderAddress(256);
    EXPECT_EQ(buffer->getDataAddress(), kDeviceBase + 256u);
    EXPECT_EQ(buffer->getDataLenght(), 4096u - 256u);
    EXPECT_EQ(buffer->getMappedDataMemory(), region.startMemory + 256);
}

TEST(DataBufferDMA, TheEmptyConstructorLeavesEverythingAtZeroExceptTheWidth) {
    auto buffer = CDataBufferDMA::CreateEmpty(8);
    EXPECT_EQ(buffer->getBuffer(), 0u);
    EXPECT_EQ(buffer->getBufferFullLenght(), 0u);
    EXPECT_EQ(buffer->getDataAddress(), 0u);
    EXPECT_EQ(buffer->getDataLenght(), 0u);
    EXPECT_EQ(buffer->getHeaderLenght(), 0u);
    EXPECT_EQ(buffer->getBitBySample(), 8);
    EXPECT_EQ(buffer->getMappedMemory(), nullptr);
    EXPECT_EQ(buffer->getMappedDataMemory(), nullptr);
    EXPECT_EQ(buffer->getSamplesCount(), 0u);
    EXPECT_EQ(buffer->getLostSamplesAll(), 0u);
    EXPECT_EQ(buffer->getADCMode(), CDataBufferDMA::ATT_1_1);
    EXPECT_EQ(buffer->getADCPackId(), 0u);
    EXPECT_EQ(buffer->getTimeCapture(), 0);
    EXPECT_FALSE(buffer->getDACOnePackMode());
    EXPECT_FALSE(buffer->getDACInfMode());
    EXPECT_EQ(buffer->getDACRepeatCount(), 0);
    EXPECT_EQ(buffer->getDACChannelSize(), 0u);
}

// The (uint8_t*, size_t, uint8_t) constructor takes ownership: the destructor
// delete[]s the pointer. Under ASan this test fails loudly if that ever becomes
// a double free or if the ownership is dropped and the block leaks.
TEST(DataBufferDMA, TheAdoptingConstructorFreesTheBufferItWasGiven) {
    uint8_t* owned = new uint8_t[512];
    std::memset(owned, 0x5A, 512);
    {
        auto buffer = CDataBufferDMA::Create(owned, 512, 8);
        EXPECT_EQ(buffer->getMappedMemory(), owned);
        EXPECT_EQ(buffer->getMappedDataMemory(), owned);
        EXPECT_EQ(buffer->getBufferFullLenght(), 512u);
        EXPECT_EQ(buffer->getBuffer(), 0u) << "there is no device address for a heap buffer";
        EXPECT_EQ(buffer->getDataAddress(), 0u);
        EXPECT_EQ(static_cast<uint8_t*>(buffer->getMappedDataMemory())[0], 0x5A);
    }
    // `owned` is gone here - nothing may touch it, and nothing may free it again.
    SUCCEED();
}

TEST(DataBufferDMA, TheMappedConstructorDoesNotFreeTheMapping) {
    Arena arena(1, 256);
    {
        auto buffer = CDataBufferDMA::Create(arena.regions()[0].start, 256, arena.memoryOf(0), 8);
        static_cast<uint8_t*>(buffer->getMappedMemory())[0] = 0x11;
    }
    // The arena still owns its storage; reading it after the buffer died must
    // be safe. ASan turns a mistake here into a use-after-free report.
    EXPECT_EQ(arena.memoryOf(0)[0], 0x11);
}

TEST(DataBufferDMA, SamplesCountFollowsTheDataLengthAndTheSampleWidth) {
    Arena arena(1, 4096);
    auto buffer = CDataBufferDMA::Create(arena.regions()[0].start, 4096, arena.memoryOf(0), 16);
    EXPECT_EQ(buffer->getSamplesCount(), 2048u);

    buffer->setBitBySample(8);
    EXPECT_EQ(buffer->getBitBySample(), 8);
    EXPECT_EQ(buffer->getSamplesCount(), 4096u);

    buffer->setBitBySample(32);
    EXPECT_EQ(buffer->getSamplesCount(), 1024u);

    // The header is excluded from the sample count.
    buffer->setBitBySample(16);
    buffer->initHeaderAddress(128);
    EXPECT_EQ(buffer->getSamplesCount(), (4096u - 128u) / 2u);

    // Integer division truncates rather than rounding up.
    Arena odd(1, 5);
    auto ragged = CDataBufferDMA::Create(odd.regions()[0].start, 5, odd.memoryOf(0), 16);
    EXPECT_EQ(ragged->getSamplesCount(), 2u);
}

TEST(DataBufferDMA, LostSamplesAreCountedInSamplesAndConvertedToBytesOnDemand) {
    Arena arena(1, 4096);
    auto buffer = CDataBufferDMA::Create(arena.regions()[0].start, 4096, arena.memoryOf(0), 16);

    EXPECT_EQ(buffer->getLostSamples(DataLib::EDataLost::FPGA), 0u) << "the constructors seed the FPGA counter";
    EXPECT_EQ(buffer->getLostSamplesAll(), 0u);
    EXPECT_EQ(buffer->getLostSamplesInBytesLenght(), 0u);
    EXPECT_EQ(buffer->getSamplesWithLost(), 2048u);

    buffer->setLostSamples(DataLib::EDataLost::FPGA, 10);
    EXPECT_EQ(buffer->getLostSamples(DataLib::EDataLost::FPGA), 10u);
    EXPECT_EQ(buffer->getLostSamplesAll(), 10u);
    EXPECT_EQ(buffer->getLostSamplesInBytesLenght(), 20u) << "10 samples of 16 bits";
    EXPECT_EQ(buffer->getSamplesWithLost(), 2048u + 10u);

    buffer->setBitBySample(8);
    EXPECT_EQ(buffer->getLostSamplesInBytesLenght(), 10u);

    buffer->reset();
    EXPECT_EQ(buffer->getLostSamplesAll(), 0u) << "reset() clears the loss counters and nothing else";
    EXPECT_EQ(buffer->getBitBySample(), 8);
}

// The trigger wait is a pause, not a loss: everything that counts lost samples (the zero fill of
// the files, the loss statistics, the sample limit) leaves it out, getSamplesWithGaps adds it.
TEST(DataBufferDMA, TheWaitIsKeptApartFromTheLoss) {
    Arena arena(1, 4096);
    auto buffer = CDataBufferDMA::Create(arena.regions()[0].start, 4096, arena.memoryOf(0), 16);

    EXPECT_EQ(buffer->getWaitSamples(), 0u) << "the constructors seed the wait counter";
    EXPECT_EQ(buffer->getSamplesWithGaps(), 2048u);

    buffer->setLostSamples(DataLib::EDataLost::FPGA, 10);
    buffer->setLostSamples(DataLib::EDataLost::WAIT, 5'000'000'000ull);
    EXPECT_EQ(buffer->getWaitSamples(), 5'000'000'000ull);
    EXPECT_EQ(buffer->getLostSamples(DataLib::EDataLost::WAIT), 5'000'000'000ull);
    EXPECT_EQ(buffer->getLostSamplesAll(), 10u);
    EXPECT_EQ(buffer->getLostSamplesInBytesLenght(), 20u);
    EXPECT_EQ(buffer->getSamplesWithLost(), 2048u + 10u);
    EXPECT_EQ(buffer->getSamplesWithGaps(), 2048u + 10u + 5'000'000'000ull);

    buffer->reset();
    EXPECT_EQ(buffer->getWaitSamples(), 0u) << "reset() clears the wait too";
}

TEST(DataBufferDMA, TheAcquisitionPropertiesAreStoredAsGiven) {
    auto buffer = CDataBufferDMA::CreateEmpty(16);
    buffer->setDecimation(1);
    buffer->setTriggerCount(0xFFFFFFFFu);
    buffer->setTriggerBlocks(0xFFFF);
    buffer->setADCFlags(0x03);
    EXPECT_EQ(buffer->getDecimation(), 1u);
    EXPECT_EQ(buffer->getTriggerCount(), 0xFFFFFFFFu);
    EXPECT_EQ(buffer->getTriggerBlocks(), 0xFFFF);
    EXPECT_EQ(buffer->getADCFlags(), 0x03);
}

TEST(DataBufferDMA, TheWriteCounterTracksHowMuchOfTheBlockIsFilled) {
    Arena arena(1, 1024);
    auto buffer = CDataBufferDMA::Create(arena.regions()[0].start, 1024, arena.memoryOf(0), 16);

    buffer->resetWriteSize();
    EXPECT_EQ(buffer->getWriteSize(), 0u);
    EXPECT_EQ(buffer->getWriteSizeLeft(), 1024u);

    buffer->addWriteSize(256);
    EXPECT_EQ(buffer->getWriteSize(), 256u);
    EXPECT_EQ(buffer->getWriteSizeLeft(), 768u);

    buffer->addWriteSize(768);
    EXPECT_EQ(buffer->getWriteSize(), 1024u);
    EXPECT_EQ(buffer->getWriteSizeLeft(), 0u) << "exactly full is allowed; only going past it is fatal";

    buffer->resetWriteSize();
    EXPECT_EQ(buffer->getWriteSize(), 0u);
    EXPECT_EQ(buffer->getWriteSizeLeft(), 1024u);

    // The counter measures the FULL block, header included, not the data area.
    buffer->initHeaderAddress(128);
    EXPECT_EQ(buffer->getWriteSizeLeft(), 1024u);
    buffer->addWriteSize(0);
    EXPECT_EQ(buffer->getWriteSize(), 0u);
}

// addWriteSize is the only guard against over-filling a block, and it is a hard
// one: FATAL prints and calls exit(1). Pinned as a death test because it is the
// documented contract - the counter itself only saturates defensively, it never
// gets the chance to wrap while this exit stands.
TEST(DataBufferDMADeathTest, WritingPastTheEndOfTheBlockIsFatal) {
    Arena arena(1, 256);
    auto buffer = CDataBufferDMA::Create(arena.regions()[0].start, 256, arena.memoryOf(0), 16);
    buffer->resetWriteSize();
    buffer->addWriteSize(256);
    ASSERT_EQ(buffer->getWriteSizeLeft(), 0u);

    EXPECT_EXIT(buffer->addWriteSize(1), ::testing::ExitedWithCode(1), "Write size greater than allowed");
}

TEST(DataBufferDMA, TheAdcAndDacPropertiesAreIndependentSlots) {
    auto buffer = CDataBufferDMA::CreateEmpty(16);

    buffer->setADCMode(CDataBufferDMA::ATT_1_20);
    buffer->setADCBaseBits(12);
    buffer->setADCBaseRate(125000000ull);
    buffer->setADCPackId(0xDEADBEEFCAFEull);
    buffer->setTimeCapture(-1234567890123ll);

    buffer->setDACOnePackMode(true);
    buffer->setDACInfMode(true);
    buffer->setDACChannelSize(4096);
    buffer->setDACBits(8);
    buffer->setDACRepeatCount(3);

    EXPECT_EQ(buffer->getADCMode(), CDataBufferDMA::ATT_1_20);
    EXPECT_EQ(buffer->getADCBaseBits(), 12);
    EXPECT_EQ(buffer->getADCBaseRate(), 125000000ull);
    EXPECT_EQ(buffer->getADCPackId(), 0xDEADBEEFCAFEull) << "the pack id is 64-bit";
    EXPECT_EQ(buffer->getTimeCapture(), -1234567890123ll) << "the capture time is signed 64-bit";

    EXPECT_TRUE(buffer->getDACOnePackMode());
    EXPECT_TRUE(buffer->getDACInfMode());
    EXPECT_EQ(buffer->getDACChannelSize(), 4096u);
    EXPECT_EQ(buffer->getDACBits(), 8);
    EXPECT_EQ(buffer->getDACRepeatCount(), 3);

    // The sample width and the DAC width are separate fields.
    EXPECT_EQ(buffer->getBitBySample(), 16);
}

TEST(DataBufferDMA, TheRepeatCounterStopsAtZeroInsteadOfGoingNegative) {
    auto buffer = CDataBufferDMA::CreateEmpty(16);
    buffer->setDACRepeatCount(2);
    buffer->decDACRepeatCount();
    EXPECT_EQ(buffer->getDACRepeatCount(), 1);
    buffer->decDACRepeatCount();
    EXPECT_EQ(buffer->getDACRepeatCount(), 0);
    buffer->decDACRepeatCount();
    buffer->decDACRepeatCount();
    EXPECT_EQ(buffer->getDACRepeatCount(), 0) << "the counter is clamped, not wrapped";

    // A negative count set explicitly is left alone - only the decrement is
    // guarded, so a caller can still park a sentinel there.
    buffer->setDACRepeatCount(-5);
    buffer->decDACRepeatCount();
    EXPECT_EQ(buffer->getDACRepeatCount(), -5);
}

// m_writeSize, m_baseADCBits, m_baseADCRate and m_dacBits appeared in no
// constructor's initialiser list, so a freshly created buffer answered
// getWriteSize(), getADCBaseBits(), getADCBaseRate() and getDACBits() with
// whatever was in the allocation, getWriteSizeLeft() reported nonsense, and
// addWriteSize() accumulated onto garbage until it tripped its own FATAL -
// while CDataBuffersPackDMA::getNextDMABufferForWrite() reads that counter
// directly. Every member now carries a default initialiser in the header.
//
// Placement-new over a poisoned block is what makes this deterministic rather
// than luck: a fresh heap allocation is often already zero.
TEST(DataBufferDMA, EveryScalarFieldIsInitialisedByTheConstructor) {
    alignas(CDataBufferDMA) unsigned char storage[sizeof(CDataBufferDMA)];
    // A varying pattern rather than memset: a single repeated byte makes several
    // fields read the same value, and arithmetic between them can cancel out and
    // hide the very thing this test looks for.
    for (std::size_t i = 0; i < sizeof(storage); i++) {
        storage[i] = static_cast<unsigned char>(i * 31u + 7u);
    }

    auto* buffer = new (static_cast<void*>(storage)) CDataBufferDMA(static_cast<uint8_t>(16));
    EXPECT_EQ(buffer->getWriteSize(), 0u);
    EXPECT_EQ(buffer->getWriteSizeLeft(), 0u) << "an empty buffer has no room, not 4 GiB of it";
    EXPECT_EQ(buffer->getADCBaseBits(), 0);
    EXPECT_EQ(buffer->getADCBaseRate(), 0u);
    EXPECT_EQ(buffer->getDACBits(), 0);
    EXPECT_EQ(buffer->getWaitSamples(), 0u);
    EXPECT_EQ(buffer->getDecimation(), 0u);
    EXPECT_EQ(buffer->getTriggerCount(), 0u);
    EXPECT_EQ(buffer->getTriggerBlocks(), 0);
    EXPECT_EQ(buffer->getADCFlags(), 0);
    buffer->~CDataBufferDMA();
}

// getSamplesCount() divides by (m_bitBySample / 8). Any width below 8 - and 0
// is reachable, since nothing validates the constructor argument - used to make
// that an integer division by zero, which is SIGFPE on both x86-64 and ARM.
TEST(DataBufferDMA, ASampleWidthBelowEightBitsDoesNotDivideByZero) {
    auto buffer = CDataBufferDMA::CreateEmpty(0);
    EXPECT_EQ(buffer->getSamplesCount(), 0u);
    EXPECT_EQ(buffer->getLostSamplesInBytesLenght(), 0u);

    Arena arena(1, 1024);
    auto mapped = CDataBufferDMA::Create(arena.regions()[0].start, 1024, arena.memoryOf(0), 0);
    EXPECT_EQ(mapped->getSamplesCount(), 0u);
}
