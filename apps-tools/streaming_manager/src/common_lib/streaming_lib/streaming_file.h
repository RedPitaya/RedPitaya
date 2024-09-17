#ifndef STREAMING_LIB_STREAMING_FILE_H
#define STREAMING_LIB_STREAMING_FILE_H

#include <memory>

#include "settings_lib/stream_settings.h"
#include "logger_lib/file_logger.h"
#include "writer_lib/file_helper.h"
#include "writer_lib/file_queue_manager.h"
#include "wav_lib/wav_writer.h"
#include "data_lib/signal.hpp"

#define FILE_PATH "/home/redpitaya/streaming_files/adc"
#define ZERO_BUFFER_SIZE 1048576

namespace streaming_lib {

class CStreamingFile
{
public:
	enum EStopReason {
		NORMAL = 0,
		OUT_SPACE = 1,
		REACH_LIMIT = 2
	};

	static auto makeEmptyDir(const std::string &_filePath) -> void;

	using Ptr = std::shared_ptr<CStreamingFile>;

	static auto create(CStreamSettings::DataFormat _fileType,
					   std::string &_filePath,
					   uint64_t _samples,
					   bool _v_mode,
					   bool testMode,
					   bool _rp_mode = false) -> Ptr;

	CStreamingFile(
		CStreamSettings::DataFormat _fileType, std::string &_filePath, uint64_t _samples, bool _v_mode, bool testMode, bool _rp_mode);
	~CStreamingFile();

	auto run(std::string _prefix) -> void;
	auto stopAndFlush() -> void;
	auto stopImmediately() -> void;
	auto disableNotify() -> void;

	auto isFileThreadWork() -> bool;
	auto isOutOfSpace() -> bool;
	auto passBuffers(DataLib::CDataBuffersPackDMA::Ptr pack) -> int;

	sigslot::signal<EStopReason> stopNotify;

	auto getFileLost() -> uint64_t;
	auto getCSVFileName() -> std::string;

	auto getPassSamples() -> uint64_t;
	auto getWritedSize() -> uint64_t;

private:
	CStreamingFile(const CStreamingFile &) = delete;
	CStreamingFile(CStreamingFile &&) = delete;
	CStreamingFile &operator=(const CStreamingFile &) = delete;
	CStreamingFile &operator=(const CStreamingFile &&) = delete;

	CFileLogger::Ptr m_fileLogger;

	std::atomic_int m_ReadyToPass;
	std::atomic_int m_SendData;
	FileQueueManager *m_file_manager;
	CWaveWriter *m_waveWriter;
	std::string m_host;
	std::string m_port;
	std::string m_filePath;
	std::string m_file_out;
	uint64_t m_samples;
	std::mutex m_stopMtx;
	std::map<DataLib::EDataBuffersPackChannel, uint64_t> m_passSizeSamples;

	bool m_testMode;
	bool m_volt_mode;
	bool m_disableNotify;
	bool m_rp_mode;

	CStreamSettings::DataFormat m_fileType;

	auto stop(EStopReason reason, bool _flush) -> void;
	auto convertBuffers(DataLib::CDataBuffersPackDMA::Ptr pack, DataLib::EDataBuffersPackChannel channel, bool lockADCTo1V) -> SBuffPass;
};

} // namespace streaming_lib

#endif
