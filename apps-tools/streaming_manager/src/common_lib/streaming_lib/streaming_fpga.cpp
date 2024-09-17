#include <chrono>
#include <cstdlib>
#include <functional>
#include <unistd.h>

#include "logger_lib/file_logger.h"
#include "logger_lib/profiler.h"
#include "streaming_fpga.h"

using namespace streaming_lib;

CStreamingFPGA::CStreamingFPGA(uio_lib::COscilloscope::Ptr _osc, uint8_t _adc_bits)
	: m_Osc_ch(_osc)
	, m_OscThread()
	, mtx()
	, m_isRun(false)
	, m_adc_bits(_adc_bits)
	, m_BytesCount(0)
	, m_testMode(false)
	, m_verbMode(false)
	, m_printDebugBuffer(false)
	, m_mappedBuffers{nullptr, nullptr}
{
	m_passRate = 0;
	m_OscThreadRun = false;
	getBuffF = nullptr;
	unlockBuffF = nullptr;
	if (!m_Osc_ch) {
		FATAL("Register controller is not initialized")
	}
}

CStreamingFPGA::~CStreamingFPGA()
{
	stop();
	TRACE("Exit")
}

auto CStreamingFPGA::isRun() -> bool
{
	return m_isRun;
}

auto CStreamingFPGA::setPrintDebugBuffer(bool mode) -> void
{
	m_printDebugBuffer = mode;
}

auto CStreamingFPGA::runNonBlock() -> void
{
	std::lock_guard lock(mtx);
	try {
		m_OscThreadRun = true;
		setIsRun(true);
		m_OscThread = std::thread(&CStreamingFPGA::oscWorker, this);

#ifdef RP_PLATFORM
		pthread_attr_t thAttr;
		int policy = 0;
		int max_prio_for_policy = 0;
		pthread_attr_init(&thAttr);
		pthread_attr_getschedpolicy(&thAttr, &policy);
		max_prio_for_policy = sched_get_priority_max(policy);
		pthread_setschedprio(m_OscThread.native_handle(), max_prio_for_policy);
		pthread_attr_destroy(&thAttr);
#endif

	} catch (const std::system_error &e) {
		FATAL("%s", e.what())
	}
}

auto CStreamingFPGA::stop() -> bool
{
	std::lock_guard lock(mtx);
	m_OscThreadRun = false;
	if (m_OscThread.joinable()) {
		m_OscThread.join();
	}
	m_Osc_ch->stop();
	m_mappedBuffers[0] = m_mappedBuffers[1] = nullptr;
	TRACE("Stop")
	return true;
}

auto CStreamingFPGA::setIsRun(bool state) -> void
{
	if (m_isRun != state) {
		m_isRun = state;
		isRunNotify(m_isRun);
	}
}

void CStreamingFPGA::oscWorker()
{
	auto timeNow = std::chrono::system_clock::now();
	auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
	auto value = curTime.time_since_epoch();
	long long int timeBegin = value.count();

	m_passRate = 0;
	m_currentBuffer = 0;

	auto pack = getBuffF();
	if (pack == nullptr) {
		FATAL("Can't get first buffer")
	}
	auto pack2 = getBuffF();
	if (pack2 == nullptr) {
		FATAL("Can't get second buffer")
	}

	if (!m_testMode) {
		m_Osc_ch->setDataAddress(0,
								 pack->getBufferDataAddress(DataLib::CH1),
								 pack->getBufferDataAddress(DataLib::CH2),
								 pack->getBufferDataAddress(DataLib::CH3),
								 pack->getBufferDataAddress(DataLib::CH4));
		m_Osc_ch->setDataAddress(1,
								 pack2->getBufferDataAddress(DataLib::CH1),
								 pack2->getBufferDataAddress(DataLib::CH2),
								 pack2->getBufferDataAddress(DataLib::CH3),
								 pack2->getBufferDataAddress(DataLib::CH4));
	}
	m_mappedBuffers[0] = pack;
	m_mappedBuffers[1] = pack2;
	m_Osc_ch->prepare();

	try {
		uint64_t dataSize = 0;
		uint64_t lostSize = 0;
		uint64_t totalPassRate = 0;
		m_overFlowSumm = 0;
		m_overFlowSummCount = 0;
		while (m_OscThreadRun) {
			bool state = true;
			profiler::setTimePoint("FPGA");
			state = m_Osc_ch->wait();
			profiler::printuS("FPGA", "Time write buffer %d", m_currentBuffer);
			if (state) {
				pack = this->passCh();
				m_passRate++;
				totalPassRate++;
			}
			if (state) {
#ifndef RP_PLATFORM
				usleep(10000); // 10ms
#endif
				oscNotify(pack);
				if (pack) {
					dataSize += pack->getLenghtDataBuffers();
					lostSize += pack->getLostAll();
				}
				if (m_verbMode) {
					timeNow = std::chrono::system_clock::now();
					curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
					value = curTime.time_since_epoch();

					if ((value.count() - timeBegin) >= 5000) {
						aprintf(stdout, "Pass buffers: %d\n", m_passRate);
						m_passRate = 0;
						timeBegin = value.count();
					}
				}
			}
		}
		auto timeNowEnd = std::chrono::system_clock::now();
		auto p1 = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow).time_since_epoch();
		auto p2 = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNowEnd).time_since_epoch();
		aprintf(stderr,
				"Loop %lld ms FPGA size %lld FPGA lost: %lld totalPassRate %lld\n",
				p2.count() - p1.count(),
				dataSize,
				lostSize,
				totalPassRate);
	} catch (std::exception &e) {
		aprintf(stderr, "Error: CStreamingApplication::oscWorker() %s\n", e.what());
	}
	setIsRun(false);
	unlockBuffF();
}

auto CStreamingFPGA::passCh() -> DataLib::CDataBuffersPackDMA::Ptr
{
	bool success = false;
	uint32_t overFlow = 0;
	success = m_Osc_ch->getFPGALost(m_currentBuffer, overFlow);

	if (!success) {
		return nullptr;
	}

	if (m_printDebugBuffer) {
		// short *wb2_1 = (short*)buffer_ch1;
		// short *wb2_2 = (short*)buffer_ch2;
		// short *wb2_3 = (short*)buffer_ch3;
		// short *wb2_4 = (short*)buffer_ch4;

		// for(int i = 0 ;i < 16 ;i ++){
		//     aprintf(stdout,"%X - %X - %X - %X \n",(wb2_1 ? (static_cast<int>(wb2_1[i]/ 4)) : 0) , (wb2_2 ?  (static_cast<int>(wb2_2[i]/ 4)) : 0), (wb2_3 ?  (static_cast<int>(wb2_3[i]/ 4)) : 0), (wb2_4 ?  (static_cast<int>(wb2_4[i]/ 4)) : 0));
		// }
		exit(1);
	}

	if (!getBuffF || !unlockBuffF) {
		FATAL("Callback functions is not defined")
	}

	auto packNew = getBuffF();
	DataLib::CDataBuffersPackDMA::Ptr pack = nullptr;
	if (packNew) {
		overFlow += m_overFlowSumm;
		pack = m_mappedBuffers[m_currentBuffer];
		auto bCh1 = pack->getBuffer(DataLib::CH1);
		if (bCh1) {
			bCh1->setLostSamples(DataLib::FPGA, overFlow + bCh1->getSamplesCount() * m_overFlowSummCount);
		}

		auto bCh2 = pack->getBuffer(DataLib::CH2);
		if (bCh2) {
			bCh2->setLostSamples(DataLib::FPGA, overFlow + bCh2->getSamplesCount() * m_overFlowSummCount);
		}

		auto bCh3 = pack->getBuffer(DataLib::CH3);
		if (bCh3) {
			bCh3->setLostSamples(DataLib::FPGA, overFlow + bCh3->getSamplesCount() * m_overFlowSummCount);
		}

		auto bCh4 = pack->getBuffer(DataLib::CH4);
		if (bCh4) {
			bCh4->setLostSamples(DataLib::FPGA, overFlow + bCh4->getSamplesCount() * m_overFlowSummCount);
		}

		m_overFlowSumm = 0;
		m_overFlowSummCount = 0;
		unlockBuffF();
		m_mappedBuffers[m_currentBuffer] = packNew;
		if (!m_testMode) {
			m_Osc_ch->setDataAddress(m_currentBuffer,
									 packNew->getBufferDataAddress(DataLib::CH1),
									 packNew->getBufferDataAddress(DataLib::CH2),
									 packNew->getBufferDataAddress(DataLib::CH3),
									 packNew->getBufferDataAddress(DataLib::CH4));
		}
	} else {
		m_overFlowSumm += overFlow;
		m_overFlowSummCount++;
	}
	m_Osc_ch->clearBuffer(m_currentBuffer);
	m_currentBuffer = m_currentBuffer ? 0 : 1;
	return pack;
}

auto CStreamingFPGA::setTestMode(bool mode) -> void
{
	m_testMode = mode;
}

auto CStreamingFPGA::setVerbousMode(bool mode) -> void
{
	m_verbMode = mode;
}
