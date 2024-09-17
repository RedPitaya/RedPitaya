#include "network_header.h"
#include "logger_lib/file_logger.h"
#include "neon_asm.h"
#include <inttypes.h>

using namespace DataLib;

constexpr uint8_t DataLib::ID_PACK_ADC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xA0, 0xA0, 0xA0, 0xA0, 0xFF, 0xFF, 0xFF, 0xFF, 0xA0, 0xA0, 0xA0, 0xA0};
constexpr uint8_t DataLib::ID_PACK_DAC[] = {0xFF, 0xFF, 0xA0, 0xA0, 0xFF, 0xFF, 0xA0, 0xA0, 0xFF, 0xFF, 0xA0, 0xA0, 0xFF, 0xFF, 0xA0, 0xA0};

auto DataLib::initHeaderADC(DataLib::CDataBufferDMA::Ptr buffer, uint64_t oscRate, uint64_t adcBits, uint64_t buffersSize, uint8_t channel)
	-> void
{
	if (buffer->getHeaderLenght() != sizeHeader())
		FATAL("Header for network not init")

	NetworkPackHeader nph;
	memcpy_neon(nph.ID_PACK, ID_PACK_ADC, sizeof(ID_PACK_ADC));
	nph.bufferSize = buffer->getBufferFullLenght();
	nph.adc.adcMode = buffer->getADCMode();
	nph.adc.sizeOfAllChannels = buffersSize;
	nph.adc.baseOSCBits = adcBits;
	nph.adc.baseRateOSC = oscRate;
	nph.adc.channel = channel;
	nph.adc.bitBySample = buffer->getBitBySample();
	nph.adc.adcMode = buffer->getADCMode();
	nph.adc.lostFPGA = 0;

	memcpy_neon(buffer->getMappedMemory(), &nph, sizeof(NetworkPackHeader));
}

auto DataLib::initHeaderDAC(CDataBufferDMA::Ptr buffer, uint64_t buffersSize, uint8_t channels) -> void
{
	if (buffer->getHeaderLenght() != sizeHeader())
		FATAL("Header for network not init")

	NetworkPackHeader nph;
	memcpy_neon(nph.ID_PACK, ID_PACK_DAC, sizeof(ID_PACK_DAC));
	nph.bufferSize = buffer->getBufferFullLenght();
	nph.dac.channels = channels;
	nph.dac.channel = 0;
	nph.dac.channelSize = 0;
	nph.dac.sizeOfAllChannels = buffersSize;
	nph.dac.onePackMode = false;
	nph.dac.repeatCount = 0;
	memcpy_neon(buffer->getMappedMemory(), &nph, sizeof(NetworkPackHeader));
}

auto DataLib::setHeaderADC(DataLib::CDataBufferDMA::Ptr buffer, uint64_t _id) -> void
{
	auto header = reinterpret_cast<NetworkPackHeader *>(buffer->getMappedMemory());
	header->packId = _id;
	header->adc.lostFPGA = buffer->getLostSamples(DataLib::EDataLost::FPGA);
}

auto DataLib::setHeaderDAC(CDataBufferDMA::Ptr buffer, uint8_t channel, uint32_t channelSize, bool onePackMode, int64_t repeatCount) -> void
{
	auto header = reinterpret_cast<NetworkPackHeader *>(buffer->getMappedMemory());
	header->dac.channel = channel;
	header->dac.channelSize = channelSize;
	header->dac.onePackMode = onePackMode;
	header->dac.repeatCount = repeatCount;
}

auto DataLib::sizeHeader() -> uint16_t
{
	return sizeof(NetworkPackHeader);
}

auto DataLib::printADCHeader(uint8_t *data) -> void
{
	NetworkPackHeader nph;
	memcpy_neon(&nph, data, sizeof(NetworkPackHeader));
	printf("ID_PACK: ");
	for (int i = 0; i < 16; i++) {
		printf("%X", nph.ID_PACK[i]);
	}
	printf("\n");
	printf("bufferSize: %" PRIu64 "\n", nph.bufferSize);
	printf("packId: %" PRIu64 "\n", nph.packId);
	printf("adc.sizeOfAllChannels: %" PRIu64 "\n", nph.adc.sizeOfAllChannels);
	printf("adc.baseOSCBits: %d\n", nph.adc.baseOSCBits);
	printf("adc.baseRateOSC: %d\n", nph.adc.baseRateOSC);
	printf("adc.channel: %d\n", nph.adc.channel);
	printf("adc.bitBySample: %d\n", nph.adc.bitBySample);
	printf("adc.adcMode: %d\n", nph.adc.adcMode);
	printf("adc.lostFPGA: %" PRIu64 "\n", nph.adc.lostFPGA);
}

auto DataLib::printDACHeader(uint8_t *data) -> void
{
	NetworkPackHeader nph;
	memcpy_neon(&nph, data, sizeof(NetworkPackHeader));
	printf("ID_PACK: ");
	for (int i = 0; i < 16; i++) {
		printf("%X", nph.ID_PACK[i]);
	}
	printf("\n");
	printf("bufferSize: %" PRIu64 "\n", nph.bufferSize);
	printf("packId: %" PRIu64 "\n", nph.packId);
	printf("dac.channels: %d\n", nph.dac.channels);
	printf("dac.sizeOfAllChannels: %" PRIu64 "\n", nph.dac.sizeOfAllChannels);
	printf("dac.channel: %d\n", nph.dac.channel);
	printf("dac.channelSize: %d\n", nph.dac.channelSize);
	printf("dac.onePackMode: %d\n", nph.dac.onePackMode);
	printf("dac.repeatCount: %" PRIu64 "\n", nph.dac.repeatCount);
}
