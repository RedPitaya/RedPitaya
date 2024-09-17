#ifndef DATA_LIB_BUFFER_PACK_H
#define DATA_LIB_BUFFER_PACK_H

#include <stdint.h>
#include <memory>
#include <map>
#include "buffer.h"

namespace DataLib {

enum EDataBuffersPackChannel {
	CH1 = 0,
	CH2 = 1,
	CH3 = 2,
	CH4 = 3
};

class CDataBuffersPackDMA final
{
public:
	using Ptr = std::shared_ptr<DataLib::CDataBuffersPackDMA>;

	static auto Create() -> CDataBuffersPackDMA::Ptr;

	CDataBuffersPackDMA();
	~CDataBuffersPackDMA();

	auto addBuffer(EDataBuffersPackChannel channel, DataLib::CDataBufferDMA::Ptr buffer) -> void;
	auto getBuffer(EDataBuffersPackChannel channel) const -> DataLib::CDataBufferDMA::Ptr;
	auto getBufferDataAddress(EDataBuffersPackChannel channel) -> uint32_t;

	auto setOSCRate(uint64_t rate) -> void;
	auto setADCBits(uint8_t bits) -> void;

	auto checkDataBuffersEqual() -> bool;
	auto getDataBuffersLenght() -> size_t;
	auto getBuffersSamples() -> size_t;
	auto getLenghtBuffers() -> uint64_t;
	auto getLenghtDataBuffers() -> uint64_t;
	auto getLostAll() -> uint64_t;
	auto isChannelPresent(EDataBuffersPackChannel channel) -> bool;

	auto debugPackADC() -> void;
	auto debugPackDAC() -> void;
	auto verifyPack() -> void;
	auto getInfoFromHeaderADC() -> void;
	auto getInfoFromHeaderDAC() -> void;

	auto resetWriteSizeAll() -> void;
	auto getNextDMABufferForWrite() -> CDataBufferDMA::Ptr;
	auto isAllDataWrite() -> bool;

private:
	CDataBuffersPackDMA(const CDataBuffersPackDMA &) = delete;
	CDataBuffersPackDMA(CDataBuffersPackDMA &&) = delete;
	CDataBuffersPackDMA &operator=(const CDataBuffersPackDMA &) = delete;
	CDataBuffersPackDMA &operator=(const CDataBuffersPackDMA &&) = delete;

	std::map<EDataBuffersPackChannel, CDataBufferDMA::Ptr> m_buffers;
};

} // namespace DataLib

#endif
