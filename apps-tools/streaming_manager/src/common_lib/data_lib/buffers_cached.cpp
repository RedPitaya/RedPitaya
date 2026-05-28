#include "buffers_cached.h"
#include <stdint.h>
#include "logger_lib/file_logger.h"
#include "logger_lib/profiler.h"
#include "network_header.h"

using namespace DataLib;

auto CBuffersCached::create() -> CBuffersCached::Ptr {
    return std::make_shared<CBuffersCached>();
}

CBuffersCached::CBuffersCached() : m_buffers(), m_needDestroy(false), m_dataSize(0) {}

CBuffersCached::~CBuffersCached() {
    std::lock_guard lock(m_mtx);
    notifyToDestory();
    m_buffers.clear();
    TRACE("Exit")
}

auto CBuffersCached::addChannel(DataLib::EDataBuffersPackChannel ch, uint8_t bits, DataLib::CDataBufferDMA::ADC_MODE _adc_mode) -> void {
    m_channels[ch] = bits;
    m_channelsMode[ch] = _adc_mode;
}

auto CBuffersCached::generateBuffers(std::vector<uio_lib::MemoryRegionT> blocks, size_t headerSize, bool testMode) -> void {
    if (m_channels.size() == 0) {
        ERROR_LOG("No active channels")
        return;
    }
    m_ringStart = 0;
    m_ringEnd = 0;
    m_ringSize = blocks.size() / m_channels.size();
    profiler::setTimePoint("initBuffer");
    size_t currentBlock = 0;
    for (auto i = 0u; i < m_ringSize; i++) {
        auto pack = DataLib::CDataBuffersPackDMA::Create();
        for (auto s : m_channels) {
            auto bits = s.second;
            auto block = blocks[currentBlock++];
            auto buff = DataLib::CDataBufferDMA::Create(block.start, block.size, block.startMemory, bits);
            buff->setADCMode(m_channelsMode[s.first]);
            if (headerSize) {
                buff->initHeaderAddress(headerSize);
                memset(buff->getMappedMemory(), 0, headerSize);
            }
            if (testMode) {
                uint8_t z = 0;
                uint8_t* data = static_cast<uint8_t*>(buff->getMappedDataMemory());
                for (uint32_t k = 0; k < buff->getDataLenght(); k++, z++) {
                    data[k] = z;
                }
            }
            pack->addBuffer(s.first, buff);
            m_dataSize = m_dataSize < buff->getDataLenght() ? buff->getDataLenght() : m_dataSize;
        }
        m_buffers.push_back(pack);
    }
    profiler::printuS("initBuffer", "Init buffer. Test mode %d", testMode);
    sem_init(&m_countsem, 0, 0);
    sem_init(&m_spacesem, 0, m_ringSize);
}

constexpr auto toPackChannel = [](auto ch) -> EDataBuffersPackChannel {
	using T = std::decay_t<decltype(ch)>;

	if constexpr (std::is_same_v<T, DACChannels>) {
		switch (ch) {
			case DACChannels::DAC_CH1:
				return EDataBuffersPackChannel::CH1;
			case DACChannels::DAC_CH2:
				return EDataBuffersPackChannel::CH2;
			default:
				FATAL("Unsupported DAC channel")
		}
	} else if constexpr (std::is_same_v<T, ADCChannels>) {
		switch (ch) {
			case ADCChannels::ADC_CH1:
				return EDataBuffersPackChannel::CH1;
			case ADCChannels::ADC_CH2:
				return EDataBuffersPackChannel::CH2;
			case ADCChannels::ADC_CH3:
				return EDataBuffersPackChannel::CH3;
			case ADCChannels::ADC_CH4:
				return EDataBuffersPackChannel::CH4;
			default:
				FATAL("Unsupported ADC channel")
		}
	} else if constexpr (std::is_same_v<T, EDataBuffersPackChannel>) {
		return ch;
	} else {
		static_assert(sizeof(T) == 0, "Unsupported channel type");
		return EDataBuffersPackChannel::CH1;
	}
};

template<typename ChannelsType>
auto CBuffersCached::generateBuffersEmpty(ChannelsType channels, std::vector<uio_lib::MemoryRegionT> blocks, size_t headerSize) -> void
{
	using EnumType = typename ChannelsType::EnumType;

	if (channels.count() == 0) {
		ERROR_LOG("No channels")
		return;
	}

	m_ringStart = 0;
	m_ringEnd = 0;
	m_ringSize = blocks.size() / channels.count();

	profiler::setTimePoint("initBuffer");
	size_t currentBlock = 0;

	for (auto i = 0u; i < m_ringSize; i++) {
		auto pack = DataLib::CDataBuffersPackDMA::Create();

		for (auto ch : channels) {
			auto block = blocks[currentBlock++];
			auto buff = DataLib::CDataBufferDMA::Create(block.start, block.size, block.startMemory, 8);

			if (headerSize) {
				buff->initHeaderAddress(headerSize);
				memset(buff->getMappedMemory(), 0, headerSize);
			}
			auto pack_ch = toPackChannel(static_cast<EnumType>(ch));
			pack->addBuffer(pack_ch, buff);
			m_dataSize = std::max<size_t>(m_dataSize, buff->getDataLenght());
		}

		m_buffers.push_back(pack);
	}

	profiler::printuS("initBuffer", "Init buffer.");
	sem_init(&m_countsem, 0, 0);
	sem_init(&m_spacesem, 0, m_ringSize);
}

auto CBuffersCached::generateBuffersEmptyDAC(dac_channels_t channels, std::vector<uio_lib::MemoryRegionT> blocks, size_t headerSize) -> void
{
	generateBuffersEmpty(channels, blocks, headerSize);
}

auto CBuffersCached::generateBuffersEmptyADC(adc_channels_t channels, std::vector<uio_lib::MemoryRegionT> blocks, size_t headerSize) -> void
{
	generateBuffersEmpty(channels, blocks, headerSize);
}

auto CBuffersCached::initHeadersADC() -> bool {
    std::lock_guard lock(m_mtx);
    for (size_t i = 0; i < m_buffers.size(); i++) {
        auto pack = m_buffers[i];
        for (auto ch = (int)DataLib::EDataBuffersPackChannel::CH1; ch <= (int)DataLib::EDataBuffersPackChannel::CH4; ch++) {
            auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)ch);
            if (buff != NULL) {
                DataLib::initHeaderADC(buff, buff->getADCBaseRate(), buff->getADCBaseBits(), pack->getLenghtDataBuffers(), ch);
            }
        }
    }
    return true;
}

auto CBuffersCached::initHeadersDAC(dac_channels_t channels) -> bool
{
	std::lock_guard lock(m_mtx);
	for (size_t i = 0; i < m_buffers.size(); i++) {
		auto pack = m_buffers[i];
		for (auto ch : channels) {
			auto pack_ch = toPackChannel(static_cast<DACChannels>(ch));
			auto buff = pack->getBuffer(pack_ch);
			if (buff != NULL) {
				DataLib::initHeaderDAC(buff, pack->getLenghtDataBuffers(), channels.count());
			}
		}
	}
	return true;
}

auto CBuffersCached::notifyToDestory() -> bool {
    m_needDestroy = true;
    return true;
}

auto CBuffersCached::isWaitToDestory() -> bool {
    return m_needDestroy;
}

auto CBuffersCached::isEmpty() -> bool {
    int sval;
    sem_getvalue(&m_countsem, &sval);
    return sval == 0;
}

inline auto CBuffersCached::getFreeSize() -> uint32_t {
    return m_ringSize - (m_ringEnd < m_ringStart ? ((m_ringEnd + m_ringSize) - m_ringStart) : (m_ringEnd - m_ringSize));
}

auto CBuffersCached::fullPercent() -> float {
    auto usedBuff = (m_ringEnd < m_ringStart ? ((m_ringEnd + m_ringSize) - m_ringStart) : (m_ringEnd - m_ringSize));
    return (float)usedBuff / (float)m_ringSize;
}

auto CBuffersCached::writeBuffer(bool timeout) -> DataLib::CDataBuffersPackDMA::Ptr {
	TRACE_FUNC()
	if (m_ringSize == 0) {
		return nullptr;
	}

	if (timeout) {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
            return nullptr;
        }

        ts.tv_sec += 1;
        auto res = sem_timedwait(&m_spacesem, &ts);
        if (res != 0) {
			return nullptr;
		}
	} else {
        if (sem_wait(&m_spacesem) != 0) {
            return nullptr;
        }
    }

    m_mtx.lock();
    m_ringEnd = (m_ringEnd + 1) % m_ringSize;
    auto pack = m_buffers[m_ringEnd];
    m_mtx.unlock();
    return pack;
}

auto CBuffersCached::unlockBufferWrite() -> void {
    std::lock_guard lock(m_mtx);
    sem_post(&m_countsem);
}

auto CBuffersCached::unlockBufferRead() -> void {
    std::lock_guard lock(m_mtx);
    sem_post(&m_spacesem);
}

auto CBuffersCached::readBuffer() -> DataLib::CDataBuffersPackDMA::Ptr {
	TRACE_FUNC()
	if (m_ringSize == 0) {
		return nullptr;
	}

	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        return nullptr;
    }

    ts.tv_sec += 1;
    if (sem_timedwait(&m_countsem, &ts) != 0) {
        return nullptr;
    }

    m_mtx.lock();
    m_ringStart = (m_ringStart + 1) % m_ringSize;
    auto pack = m_buffers[m_ringStart];
    m_mtx.unlock();
    return pack;
}

auto CBuffersCached::getDataSize() -> uint32_t {
    return m_dataSize;
}

auto CBuffersCached::setOSCRate(uint64_t rate) -> void {
    std::lock_guard lock(m_mtx);
    for (size_t i = 0; i < m_buffers.size(); i++) {
        m_buffers[i]->setOSCRate(rate);
    }
}

auto CBuffersCached::setADCBits(uint8_t bits) -> void {
    std::lock_guard lock(m_mtx);
    for (size_t i = 0; i < m_buffers.size(); i++) {
        m_buffers[i]->setADCBits(bits);
    }
}
