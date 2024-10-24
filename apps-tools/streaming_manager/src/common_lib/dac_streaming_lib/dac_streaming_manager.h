#ifndef STREAMING_ROOT_DACSTREAMING_MANAGER_H
#define STREAMING_ROOT_DACSTREAMING_MANAGER_H

#include <atomic>
#include <mutex>
#include <thread>
#include <string>

#include "data_lib/buffers_cached.h"
#include "data_lib/signal.hpp"
#include "net_lib/asio_net.h"
#include "reader_lib/reader_controller.h"

namespace dac_streaming_lib {

class CDACStreamingManager
{
public:
	enum NotifyResult {
		NR_ENDED,
		NR_BROKEN,
		NR_EMPTY,
		NR_MISSING_FILE,
		NR_STOP,
		NR_MEM_ERROR,
		NR_MEM_MODIFY,
		NR_SETTINGS_ERROR
	};

	enum DACStream_FileType {
		TDMS_TYPE,
		WAV_TYPE
	};

	using Ptr = std::shared_ptr<CDACStreamingManager>;

	static Ptr Create(DACStream_FileType _fileType,
					  std::string _filePath,
					  CStreamSettings::DACRepeat _repeat,
					  int32_t _rep_count,
					  uint32_t blockSize,
					  bool verbose);
	CDACStreamingManager(DACStream_FileType _fileType,
						 std::string _filePath,
						 CStreamSettings::DACRepeat _repeat,
						 int32_t _rep_count,
						 uint32_t blockSize,
						 bool verbose);

	static Ptr Create(std::string _host, bool verbose);
	CDACStreamingManager(std::string _host, bool verbose);

	static Ptr Create(uint8_t *ch[2],
					  uint64_t size[2],
					  uint8_t bytesPerSamp,
					  CStreamSettings::DACRepeat _repeat,
					  int32_t _rep_count,
					  uint32_t blockSize,
					  bool verbose);
	CDACStreamingManager(uint8_t *ch[2],
						 uint64_t size[2],
						 uint8_t bytesPerSamp,
						 CStreamSettings::DACRepeat _repeat,
						 int32_t _rep_count,
						 uint32_t blockSize,
						 bool verbose);

	~CDACStreamingManager();
	CDACStreamingManager(const CDACStreamingManager &) = delete;
	CDACStreamingManager(CDACStreamingManager &&) = delete;

	auto run() -> void;
	auto stop() -> void;
	auto isLocalMode() -> bool;

	auto getBufferManager() -> DataLib::CBuffersCached::Ptr;
	auto getBuffer() -> const DataLib::CDataBuffersPackDMA::Ptr;
	auto unlockBuffer() -> void;
	auto isEmptyBuffer() -> bool;
	auto isRunned() -> bool;

	auto getChannels(bool *ch1Active, bool *ch2Active) -> bool;

	sigslot::signal<NotifyResult> notifyStop;

private:
	auto startServer() -> void;
	auto stopServer() -> void;
	auto threadFunc() -> void;

	bool m_use_local_file;
	DACStream_FileType m_fileType;
	std::string m_host;
	std::string m_filePath;
	net_lib::CAsioNet::Ptr m_asionet;

	CStreamSettings::DACRepeat m_repeat;
	int32_t m_rep_count;
	CReaderController *m_readerController;

	DataLib::CBuffersCached::Ptr m_buffer;
	std::thread m_Thread;
	std::atomic_bool m_isRun;
	std::atomic_bool m_isThreadRun;
	std::mutex m_mtx;
	bool m_verbose;
	uint32_t m_blockSize;
};

} // namespace dac_streaming_lib

#endif
