#include <time.h>
#include <cstdlib>

#include "dac_streaming_manager.h"
#include "data_lib/neon_asm.h"
#include "logger_lib/file_logger.h"

using namespace dac_streaming_lib;

CDACStreamingManager::Ptr CDACStreamingManager::Create(DACStream_FileType _fileType,
													   std::string _filePath,
													   CStreamSettings::DACRepeat _repeat,
													   int32_t _rep_count,
													   uint32_t blockSize,
													   bool verbose)
{
	return std::make_shared<CDACStreamingManager>(_fileType, _filePath, _repeat, _rep_count, blockSize, verbose);
}

CDACStreamingManager::CDACStreamingManager(DACStream_FileType _fileType,
										   std::string _filePath,
										   CStreamSettings::DACRepeat _repeat,
										   int32_t _rep_count,
										   uint32_t blockSize,
										   bool verbose)
	: m_use_local_file(true)
	, m_fileType(_fileType)
	, m_host("")
	, m_filePath(_filePath)
	, m_asionet(nullptr)
	, m_repeat(_repeat)
	, m_rep_count(_rep_count)
	, m_readerController(nullptr)
	, m_buffer(DataLib::CBuffersCached::create())
	, m_isRun(false)
	, m_verbose(verbose)
	, m_blockSize(blockSize)
{
	auto type = CStreamSettings::DataFormat::TDMS;
	switch (m_fileType) {
		case DACStream_FileType::TDMS_TYPE:
			type = CStreamSettings::DataFormat::TDMS;
			break;
		case DACStream_FileType::WAV_TYPE:
			type = CStreamSettings::DataFormat::WAV;
			break;
		default:
			FATAL("Error type of file")
	}
	m_readerController = new CReaderController(type, m_filePath, m_repeat, m_rep_count, blockSize);
}

CDACStreamingManager::Ptr CDACStreamingManager::Create(std::string _host, bool verbose)
{
	return std::make_shared<CDACStreamingManager>(_host, verbose);
}

CDACStreamingManager::CDACStreamingManager(std::string _host, bool verbose)
	: m_use_local_file(false)
	, m_fileType(TDMS_TYPE)
	, m_host(_host)
	, m_filePath("")
	, m_asionet(nullptr)
	, m_repeat(CStreamSettings::DACRepeat::DAC_REP_OFF)
	, m_rep_count(0)
	, m_readerController(nullptr)
	, m_buffer(DataLib::CBuffersCached::create())
	, m_isRun(false)
	, m_verbose(verbose)
{}

CDACStreamingManager::~CDACStreamingManager()
{
	this->stop();
	this->stopServer();
	if (m_readerController) {
		delete m_readerController;
		m_readerController = nullptr;
	}
}

auto CDACStreamingManager::startServer() -> void
{
	m_asionet = nullptr;
	m_asionet = net_lib::CAsioNet::create(net_lib::M_SERVER, m_host, NET_DAC_STREAMING_PORT, m_buffer);

	m_asionet->serverConnectNotify.connect([=, this](std::string &host) {
		if (m_verbose)
			aprintf(stdout, "Client connected to DAC streaming server %s\n", host.c_str());
	});
	m_asionet->serverDisconnectNotify.connect([=, this](std::string &host) {
		if (m_verbose)
			aprintf(stdout, "Client disconnected from DAC streaming server %s\n", host.c_str());
	});

	m_asionet->start();
}

auto CDACStreamingManager::stopServer() -> void
{
	m_asionet = nullptr;
}

auto CDACStreamingManager::run() -> void
{
	if (!m_use_local_file) {
		this->startServer();
	} else {
		std::lock_guard lock(m_mtx);
		m_isRun = true;
		m_Thread = std::thread(&CDACStreamingManager::threadFunc, this);
	}
}

auto CDACStreamingManager::stop() -> void
{
	if (!m_use_local_file) {
		this->stopServer();
	} else {
		std::lock_guard lock(m_mtx);
		if (m_isRun) {
			m_isThreadRun = false;
			if (m_Thread.joinable()) {
				m_Thread.join();
			}
		}
		m_isRun = false;
	}
}

auto CDACStreamingManager::getChannels(bool *ch1Active, bool *ch2Active) -> bool
{
	if (m_use_local_file) {
		m_readerController->getChannels(ch1Active, ch2Active);
		return true;
	}
	*ch1Active = 0;
	*ch2Active = 0;
	return false;
}

auto CDACStreamingManager::getBuffer() -> const DataLib::CDataBuffersPackDMA::Ptr
{
	auto buffer = m_buffer->readBuffer();
	if (buffer)
		buffer->getInfoFromHeaderDAC();
	return buffer;
}

auto CDACStreamingManager::unlockBuffer() -> void
{
	m_buffer->unlockBufferRead();
}

auto CDACStreamingManager::getBufferManager() -> DataLib::CBuffersCached::Ptr
{
	return m_buffer;
}

auto CDACStreamingManager::threadFunc() -> void
{
	try {
		m_isThreadRun = true;
		bool onePackMode = false;
		int64_t repeatCount = m_repeat.value == CStreamSettings::DACRepeat::DAC_REP_INF ? -1 : 1;
		if (repeatCount != -1) {
			repeatCount = m_rep_count;
		}
		if (m_readerController) {
			size_t chSizes[2] = {0, 0};
			m_readerController->getChannelsSize(&chSizes[0], &chSizes[1]);
			if (m_blockSize >= (chSizes[0] + chSizes[1])) {
				onePackMode = true;
				m_readerController->disableRepeatMode();
			}
		}
		while (m_isThreadRun) {
			if (m_readerController) {
				auto isOpen = m_readerController->isOpen();
				if (isOpen != CReaderController::OR_OK) {
					if (isOpen == CReaderController::OR_CLOSE) {
						notifyStop(CDACStreamingManager::NR_MISSING_FILE);
					}
					notifyStop(CDACStreamingManager::NR_BROKEN);
				}
				uint8_t *ch_buffers[2] = {nullptr, nullptr};
				size_t size_buffer[2] = {0, 0};
				size_t size_actual[2] = {0, 0};

				auto pack = m_buffer->writeBuffer(true);
				if (pack){
					auto res = m_readerController->getBufferPrepared(&ch_buffers[0],
																	 &size_buffer[0],
																	 &size_actual[0],
																	 &ch_buffers[1],
																	 &size_buffer[1],
																	 &size_actual[1]);
					if (size_buffer[0] != 0 || size_buffer[1] != 0) {
						if (size_buffer[0] != 0) {
							auto ch1 = pack->getBuffer(DataLib::CH1);
							if (ch1->getDataLenght() != size_buffer[0]) {
								FATAL("DMA CH1 buffer has diffrent size src: %zu dst: %zu", size_buffer[0], ch1->getDataLenght())
							}
							memcpy_neon(ch1->getMappedDataMemory(), ch_buffers[0], size_buffer[0]);
							// for (int i = 0; i < size1_read / 2; i++) {
							// 	((uint16_t *) ch1->getMappedDataMemory())[i] = ((1 << 15) * i) / (size1_read / 2);
							// }
							DataLib::setHeaderDAC(ch1, 1, size_actual[0], onePackMode, repeatCount);
							// TRACE_SHORT("Copy channel 1 dest size %zu src size %zu", ch1->getDataLenght(), size1_read)
						}
						if (size_buffer[1] != 0) {
							auto ch2 = pack->getBuffer(DataLib::CH2);
							if (ch2->getDataLenght() != size_buffer[1]) {
								FATAL("DMA CH2 buffer has diffrent size src: %zu dst: %zu", size_buffer[1], ch2->getDataLenght())
							}
							memcpy_neon(ch2->getMappedDataMemory(), ch_buffers[1], size_buffer[1]);
							// for (int i = 0; i < size2_read; i++) {
							// 	((uint16_t *) ch2->getMappedDataMemory())[i] = i % 8;
							// }
							DataLib::setHeaderDAC(ch2, 2, size_actual[1], onePackMode, repeatCount);
							// TRACE_SHORT("Copy channel 2 dest size %zu src size %zu", ch2->getDataLenght(), size2_read)
						}
						m_buffer->unlockBufferWrite();
					}
					if (res != CReaderController::BR_OK) {
						auto sendRes = CDACStreamingManager::NR_STOP;
						switch (res) {
							case CReaderController::BR_BROKEN:
								sendRes = CDACStreamingManager::NR_BROKEN;
								break;
							case CReaderController::BR_EMPTY:
								sendRes = CDACStreamingManager::NR_EMPTY;
								break;
							case CReaderController::BR_ENDED:
								sendRes = CDACStreamingManager::NR_ENDED;
								break;
							default:
								break;
						}

						notifyStop(sendRes);
						m_isThreadRun = false;
					}
				}
			}
		}
	} catch (std::exception &e) {
		ERROR_LOG("%s", e.what())
	}
}

auto CDACStreamingManager::isLocalMode() -> bool
{
	return m_use_local_file;
}
