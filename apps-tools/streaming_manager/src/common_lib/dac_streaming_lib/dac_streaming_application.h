#ifndef STREAMING_ROOT_DACSAPP_H
#define STREAMING_ROOT_DACSAPP_H

#include <atomic>
#include <mutex>
#include <thread>

#include "uio_lib/generator.h"
#include "dac_streaming_manager.h"

namespace dac_streaming_lib {

class CDACStreamingApplication
{
public:
	using Ptr = std::shared_ptr<CDACStreamingApplication>;

	CDACStreamingApplication(CDACStreamingManager::Ptr _streamingManager, uio_lib::CGenerator::Ptr _gen);
	~CDACStreamingApplication();
	auto run() -> void;
	auto runNonBlock() -> void;
	auto stop() -> bool;
	auto isRun() -> bool { return m_isRun; }

private:
	void genWorker();
	void signalHandler(const std::error_code &_error, int _signalNumber);

	int m_PerformanceCounterPeriod = 10;
	uio_lib::CGenerator::Ptr m_gen;
	CDACStreamingManager::Ptr m_streamingManager;
	std::thread m_Thread;
	std::mutex mtx;
	std::atomic_bool m_GenThreadRun;
	std::atomic_int m_ReadyToPass;
	std::atomic_bool m_isRun;
	std::atomic_bool m_isRunNonBloking;
};

} // namespace dac_streaming_lib
#endif
