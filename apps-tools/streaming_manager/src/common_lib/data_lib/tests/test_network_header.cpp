/**
 * The on-the-wire packet header.
 *
 * These bytes go straight out of a socket and are parsed by a peer that was
 * built separately, so the layout is a compatibility contract: the offsets and
 * sizes below are static_asserts on purpose, and they fail at compile time on
 * any target whose padding rules differ rather than producing packets nobody
 * can read.
 *
 * The ADC and DAC views share one union, so exactly one of them is meaningful
 * per packet - which one is decided by ID_PACK. That aliasing is pinned too,
 * because it is invisible at the call site.
 */

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "data_lib/buffers_pack.h"
#include "data_lib/network_header.h"
#include "support.h"

using data_test::Arena;
using DataLib::ADCHeader;
using DataLib::CDataBufferDMA;
using DataLib::DACHeader;
using DataLib::NetworkPackHeader;

namespace {

// A buffer whose header area is initialised the way the library requires:
// initHeaderADC and initHeaderDAC both FATAL (which calls exit) unless
// getHeaderLenght() is exactly sizeHeader().
auto MakeHeaderedBuffer(const Arena& arena, std::size_t index, uint8_t bits) -> CDataBufferDMA::Ptr {
    const auto& region = arena.regions()[index];
    auto buffer = CDataBufferDMA::Create(region.start, region.size, region.startMemory, bits);
    buffer->initHeaderAddress(DataLib::sizeHeader());
    std::memset(buffer->getMappedMemory(), 0, DataLib::sizeHeader());
    return buffer;
}

auto HeaderOf(const CDataBufferDMA::Ptr& buffer) -> const NetworkPackHeader* {
    return reinterpret_cast<const NetworkPackHeader*>(buffer->getMappedMemory());
}

}  // namespace

TEST(NetworkHeader, TheWireLayoutIsFixed) {
    static_assert(sizeof(ADCHeader) == 51, "ADC view: 8+8+4+1+1+1+1+8, then 8+4+4+2+1");
    static_assert(sizeof(DACHeader) == 25, "DAC view: 1+1+4+8+1+1+8+1");
    static_assert(sizeof(DataLib::GPIOHeader) == 1);
    static_assert(sizeof(NetworkPackHeader) == 83, "16 id + 8 size + 8 packId + 51 union");

    static_assert(offsetof(NetworkPackHeader, ID_PACK) == 0);
    static_assert(offsetof(NetworkPackHeader, bufferSize) == 16, "packed: no padding after the 16-byte id");
    static_assert(offsetof(NetworkPackHeader, packId) == 24);

    static_assert(offsetof(ADCHeader, lostFPGA) == 0);
    static_assert(offsetof(ADCHeader, sizeOfAllChannels) == 8);
    static_assert(offsetof(ADCHeader, baseRateOSC) == 16);
    static_assert(offsetof(ADCHeader, baseOSCBits) == 20);
    static_assert(offsetof(ADCHeader, channel) == 21);
    static_assert(offsetof(ADCHeader, bitBySample) == 22);
    static_assert(offsetof(ADCHeader, adcMode) == 23);
    static_assert(offsetof(ADCHeader, timeCapture) == 24);
    static_assert(offsetof(ADCHeader, waitFPGA) == 32, "the new fields follow the old ones");
    static_assert(offsetof(ADCHeader, decimation) == 40);
    static_assert(offsetof(ADCHeader, triggerCount) == 44);
    static_assert(offsetof(ADCHeader, triggerBlocks) == 48);
    static_assert(offsetof(ADCHeader, flags) == 50);
    static_assert(DataLib::ADC_FLAG_TRIGGER_MODE == 0x01 && DataLib::ADC_FLAG_WAIT_COUNTER == 0x02);

    static_assert(offsetof(DACHeader, channels) == 0);
    static_assert(offsetof(DACHeader, channel) == 1);
    static_assert(offsetof(DACHeader, channelSize) == 2, "packed: uint32 at an odd-ish offset");
    static_assert(offsetof(DACHeader, sizeOfAllChannels) == 6);
    static_assert(offsetof(DACHeader, onePackMode) == 14);
    static_assert(offsetof(DACHeader, infMode) == 15);
    static_assert(offsetof(DACHeader, repeatCount) == 16);
    static_assert(offsetof(DACHeader, bits) == 24);
    SUCCEED();
}

TEST(NetworkHeader, TheReservedHeaderAreaIsAWholeNumberOf128ByteBlocks) {
    EXPECT_EQ(DataLib::sizeHeader(), 128);
    EXPECT_GT(DataLib::sizeHeader(), sizeof(NetworkPackHeader)) << "the reserved area must hold the whole struct";
    EXPECT_EQ(DataLib::sizeHeader() % 128, 0);
}

TEST(NetworkHeader, TheTwoPacketIdsDifferAndAreSixteenBytes) {
    const uint8_t adc[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xA0, 0xA0, 0xA0, 0xA0, 0xFF, 0xFF, 0xFF, 0xFF, 0xA0, 0xA0, 0xA0, 0xA0};
    const uint8_t dac[16] = {0xFF, 0xFF, 0xA0, 0xA0, 0xFF, 0xFF, 0xA0, 0xA0, 0xFF, 0xFF, 0xA0, 0xA0, 0xFF, 0xFF, 0xA0, 0xA0};
    EXPECT_EQ(std::memcmp(DataLib::ID_PACK_ADC, adc, sizeof(adc)), 0);
    EXPECT_EQ(std::memcmp(DataLib::ID_PACK_DAC, dac, sizeof(dac)), 0);
    EXPECT_NE(std::memcmp(DataLib::ID_PACK_ADC, DataLib::ID_PACK_DAC, 16), 0) << "a receiver tells the two apart by these bytes alone";
}

TEST(NetworkHeader, TheDefaultConstructedHeaderIsZeroedExceptTheDacWidth) {
    const ADCHeader adc;
    EXPECT_EQ(adc.lostFPGA, 0u);
    EXPECT_EQ(adc.sizeOfAllChannels, 0u);
    EXPECT_EQ(adc.baseRateOSC, 0u);
    EXPECT_EQ(adc.baseOSCBits, 0);
    EXPECT_EQ(adc.channel, 0);
    EXPECT_EQ(adc.bitBySample, 0);
    EXPECT_EQ(adc.adcMode, 0);
    EXPECT_EQ(adc.timeCapture, 0);
    EXPECT_EQ(adc.waitFPGA, 0u);
    EXPECT_EQ(adc.decimation, 0u);
    EXPECT_EQ(adc.triggerCount, 0u);
    EXPECT_EQ(adc.triggerBlocks, 0);
    EXPECT_EQ(adc.flags, 0);

    const DACHeader dac;
    EXPECT_EQ(dac.channels, 0);
    EXPECT_EQ(dac.channelSize, 0u);
    EXPECT_FALSE(dac.onePackMode);
    EXPECT_FALSE(dac.infMode);
    EXPECT_EQ(dac.repeatCount, 0);
    EXPECT_EQ(dac.bits, 16) << "the DAC width defaults to 16, unlike every other field";
}

TEST(NetworkHeader, InitHeaderAdcWritesTheAdcViewIntoTheMappedBlock) {
    Arena arena(1, 4096);
    auto buffer = MakeHeaderedBuffer(arena, 0, 16);
    buffer->setADCMode(CDataBufferDMA::ATT_1_20);

    DataLib::initHeaderADC(buffer, 125000000ull, 12, 8192, 3);

    const NetworkPackHeader* header = HeaderOf(buffer);
    EXPECT_EQ(std::memcmp(header->ID_PACK, DataLib::ID_PACK_ADC, 16), 0);
    EXPECT_EQ(header->bufferSize, 4096u) << "the FULL block, header included";
    EXPECT_EQ(header->packId, 0u) << "the id is stamped later, by setHeaderADC";
    EXPECT_EQ(header->adc.sizeOfAllChannels, 8192u);
    EXPECT_EQ(header->adc.baseOSCBits, 12);
    EXPECT_EQ(header->adc.baseRateOSC, 125000000u);
    EXPECT_EQ(header->adc.channel, 3);
    EXPECT_EQ(header->adc.bitBySample, 16) << "taken from the buffer, not from the argument";
    EXPECT_EQ(header->adc.adcMode, CDataBufferDMA::ATT_1_20);
    EXPECT_EQ(header->adc.lostFPGA, 0u);
    EXPECT_EQ(header->adc.timeCapture, 0);
    EXPECT_EQ(header->adc.waitFPGA, 0u);
    EXPECT_EQ(header->adc.decimation, 0u);
    EXPECT_EQ(header->adc.triggerCount, 0u);
    EXPECT_EQ(header->adc.triggerBlocks, 0);
    EXPECT_EQ(header->adc.flags, 0);

    // Only the header area is touched; the data area is left alone.
    EXPECT_EQ(static_cast<uint8_t*>(buffer->getMappedDataMemory())[0], 0);
}

// baseRateOSC is a uint32_t while initHeaderADC takes the rate as uint64_t, so
// a rate above 4 GHz is truncated silently. It cannot happen with the current
// hardware; pinned so that widening the field stays a conscious decision.
TEST(NetworkHeader, TheOscRateIsTruncatedToThirtyTwoBitsOnTheWire) {
    Arena arena(1, 1024);
    auto buffer = MakeHeaderedBuffer(arena, 0, 16);
    DataLib::initHeaderADC(buffer, 0x1'0000'0001ull, 12, 0, 0);
    EXPECT_EQ(HeaderOf(buffer)->adc.baseRateOSC, 1u);
}

TEST(NetworkHeader, SetHeaderAdcStampsTheIdAndTheRuntimeCounters) {
    Arena arena(1, 4096);
    auto buffer = MakeHeaderedBuffer(arena, 0, 16);
    DataLib::initHeaderADC(buffer, 125000000ull, 16, 4096, 1);

    buffer->setLostSamples(DataLib::EDataLost::FPGA, 42);
    buffer->setLostSamples(DataLib::EDataLost::WAIT, 0x1'2345'6789ull);
    buffer->setTimeCapture(-9876543210ll);
    buffer->setDecimation(65536);
    buffer->setTriggerCount(0xFFFFFFFEu);
    buffer->setTriggerBlocks(0xFFFF);
    buffer->setADCFlags(DataLib::ADC_FLAG_TRIGGER_MODE | DataLib::ADC_FLAG_WAIT_COUNTER);
    DataLib::setHeaderADC(buffer, 0xAABBCCDDEEFF0011ull);

    const NetworkPackHeader* header = HeaderOf(buffer);
    EXPECT_EQ(header->packId, 0xAABBCCDDEEFF0011ull);
    EXPECT_EQ(header->adc.lostFPGA, 42u) << "the loss and the wait are separate fields";
    EXPECT_EQ(header->adc.waitFPGA, 0x1'2345'6789ull) << "beyond 32 bits: a pause of seconds at decimation 1";
    EXPECT_EQ(header->adc.timeCapture, -9876543210ll);
    EXPECT_EQ(header->adc.decimation, 65536u);
    EXPECT_EQ(header->adc.triggerCount, 0xFFFFFFFEu);
    EXPECT_EQ(header->adc.triggerBlocks, 0xFFFF);
    EXPECT_EQ(header->adc.flags, 0x03);
    // The fields initHeaderADC wrote are untouched.
    EXPECT_EQ(header->adc.baseRateOSC, 125000000u);
    EXPECT_EQ(header->adc.channel, 1);
    EXPECT_EQ(std::memcmp(header->ID_PACK, DataLib::ID_PACK_ADC, 16), 0);
}

TEST(NetworkHeader, InitAndSetHeaderDacWriteTheDacView) {
    Arena arena(1, 4096);
    auto buffer = MakeHeaderedBuffer(arena, 0, 16);

    DataLib::initHeaderDAC(buffer, 8192, 2);
    const NetworkPackHeader* header = HeaderOf(buffer);
    EXPECT_EQ(std::memcmp(header->ID_PACK, DataLib::ID_PACK_DAC, 16), 0);
    EXPECT_EQ(header->bufferSize, 4096u);
    EXPECT_EQ(header->dac.channels, 2);
    EXPECT_EQ(header->dac.sizeOfAllChannels, 8192u);
    EXPECT_EQ(header->dac.channel, 0);
    EXPECT_EQ(header->dac.channelSize, 0u);
    EXPECT_FALSE(header->dac.onePackMode);
    EXPECT_EQ(header->dac.repeatCount, 0);
    EXPECT_EQ(header->dac.bits, 0) << "initHeaderDAC overwrites the struct's default of 16 with 0";

    DataLib::setHeaderDAC(buffer, 1, 2048, true, false, -3, 8);
    EXPECT_EQ(header->dac.channel, 1);
    EXPECT_EQ(header->dac.channelSize, 2048u);
    EXPECT_TRUE(header->dac.onePackMode);
    EXPECT_FALSE(header->dac.infMode);
    EXPECT_EQ(header->dac.repeatCount, -3);
    EXPECT_EQ(header->dac.bits, 8);
    EXPECT_EQ(header->dac.channels, 2) << "setHeaderDAC does not disturb the channel count";
}

// adc and dac are members of the same anonymous union: writing one clobbers the
// other. Nothing in the API says so, which is why it is worth a test.
TEST(NetworkHeader, TheAdcAndDacViewsShareTheSameBytes) {
    NetworkPackHeader header;
    EXPECT_EQ(static_cast<const void*>(&header.adc), static_cast<const void*>(&header.dac));

    header.adc.lostFPGA = 0x0102030405060708ull;
    // DACHeader::channels and ::channel are the first two bytes of the union,
    // so they now read the low bytes of lostFPGA on this little-endian target.
    EXPECT_EQ(header.dac.channels, 0x08);
    EXPECT_EQ(header.dac.channel, 0x07);
}

TEST(NetworkHeader, GetInfoFromHeaderAdcCopiesTheWireValuesBackIntoThePack) {
    Arena arena(2, 4096);
    auto pack = DataLib::CDataBuffersPackDMA::Create();
    auto first = MakeHeaderedBuffer(arena, 0, 16);
    auto second = MakeHeaderedBuffer(arena, 1, 16);
    pack->addBuffer(DataLib::CH1, first);
    pack->addBuffer(DataLib::CH2, second);

    for (auto& buffer : {first, second}) {
        DataLib::initHeaderADC(buffer, 62500000ull, 14, 8192, 1);
        buffer->setLostSamples(DataLib::EDataLost::FPGA, 7);
        buffer->setLostSamples(DataLib::EDataLost::WAIT, 1000000007ull * 5);
        buffer->setTimeCapture(555);
        buffer->setDecimation(8);
        buffer->setTriggerCount(3);
        buffer->setTriggerBlocks(4);
        buffer->setADCFlags(DataLib::ADC_FLAG_WAIT_COUNTER);
        DataLib::setHeaderADC(buffer, 99);
    }

    // Wipe the in-memory copies so that only the wire bytes can restore them.
    for (auto& buffer : {first, second}) {
        buffer->setADCBaseBits(0);
        buffer->setADCBaseRate(0);
        buffer->setLostSamples(DataLib::EDataLost::FPGA, 0);
        buffer->setBitBySample(8);
        buffer->setADCPackId(0);
        buffer->setTimeCapture(0);
        buffer->setLostSamples(DataLib::EDataLost::WAIT, 0);
        buffer->setDecimation(0);
        buffer->setTriggerCount(0);
        buffer->setTriggerBlocks(0);
        buffer->setADCFlags(0);
    }

    pack->getInfoFromHeaderADC();

    for (auto& buffer : {first, second}) {
        EXPECT_EQ(buffer->getADCBaseBits(), 14);
        EXPECT_EQ(buffer->getADCBaseRate(), 62500000ull);
        EXPECT_EQ(buffer->getLostSamples(DataLib::EDataLost::FPGA), 7u);
        EXPECT_EQ(buffer->getBitBySample(), 16);
        EXPECT_EQ(buffer->getADCPackId(), 99u);
        EXPECT_EQ(buffer->getTimeCapture(), 555);
        EXPECT_EQ(buffer->getWaitSamples(), 1000000007ull * 5);
        EXPECT_EQ(buffer->getDecimation(), 8u);
        EXPECT_EQ(buffer->getTriggerCount(), 3u);
        EXPECT_EQ(buffer->getTriggerBlocks(), 4);
        EXPECT_EQ(buffer->getADCFlags(), DataLib::ADC_FLAG_WAIT_COUNTER);
    }
}

TEST(NetworkHeader, GetInfoFromHeaderDacCopiesTheWireValuesBackIntoThePack) {
    Arena arena(1, 4096);
    auto pack = DataLib::CDataBuffersPackDMA::Create();
    auto buffer = MakeHeaderedBuffer(arena, 0, 16);
    pack->addBuffer(DataLib::CH1, buffer);

    DataLib::initHeaderDAC(buffer, 4096, 1);
    DataLib::setHeaderDAC(buffer, 0, 1024, true, true, 5, 8);

    pack->getInfoFromHeaderDAC();

    EXPECT_TRUE(buffer->getDACOnePackMode());
    EXPECT_TRUE(buffer->getDACInfMode());
    EXPECT_EQ(buffer->getDACRepeatCount(), 5);
    EXPECT_EQ(buffer->getDACChannelSize(), 1024u);
    EXPECT_EQ(buffer->getDACBits(), 8);
}

TEST(NetworkHeader, VerifyPackAcceptsAPackWhoseHeadersMatchTheirBuffers) {
    Arena arena(2, 4096);
    auto pack = DataLib::CDataBuffersPackDMA::Create();
    for (std::size_t i = 0; i < 2; i++) {
        auto buffer = MakeHeaderedBuffer(arena, i, 16);
        DataLib::initHeaderADC(buffer, 125000000ull, 16, 8192, static_cast<uint8_t>(i));
        pack->addBuffer(static_cast<DataLib::EDataBuffersPackChannel>(i), buffer);
    }
    // A mismatch here calls FATAL, which exits the process, so this test also
    // guards against initHeaderADC writing the wrong bufferSize.
    pack->verifyPack(DataLib::ID_PACK_ADC);
    SUCCEED();
}

namespace {

auto MakeAdcPack(const Arena& arena) -> DataLib::CDataBuffersPackDMA::Ptr {
    auto pack = DataLib::CDataBuffersPackDMA::Create();
    for (std::size_t i = 0; i < 2; i++) {
        auto buffer = MakeHeaderedBuffer(arena, i, 16);
        DataLib::initHeaderADC(buffer, 125000000ull, 16, 8192, static_cast<uint8_t>(i));
        pack->addBuffer(static_cast<DataLib::EDataBuffersPackChannel>(i), buffer);
    }
    return pack;
}

}  // namespace

// A DAC pack arriving where ADC packs are expected (or the reverse) has a header of another
// kind: its fields would be read as ADC values.
TEST(NetworkHeaderDeathTest, VerifyPackRefusesAPackOfTheOtherKind) {
    Arena arena(2, 4096);
    auto pack = MakeAdcPack(arena);
    EXPECT_EXIT(pack->verifyPack(DataLib::ID_PACK_DAC), ::testing::ExitedWithCode(1), "ID_PACK");
}

TEST(NetworkHeaderDeathTest, VerifyPackRefusesAPackWithoutAnId) {
    Arena arena(2, 4096);
    auto pack = MakeAdcPack(arena);
    std::memset(pack->getBuffer(DataLib::CH2)->getMappedMemory(), 0, 16);
    EXPECT_EXIT(pack->verifyPack(DataLib::ID_PACK_ADC), ::testing::ExitedWithCode(1), "ID_PACK");
}

TEST(NetworkHeaderDeathTest, VerifyPackRefusesAWrongSize) {
    Arena arena(2, 4096);
    auto pack = MakeAdcPack(arena);
    reinterpret_cast<NetworkPackHeader*>(pack->getBuffer(DataLib::CH1)->getMappedMemory())->bufferSize = 4095;
    EXPECT_EXIT(pack->verifyPack(DataLib::ID_PACK_ADC), ::testing::ExitedWithCode(1), "not the correct size");
}
