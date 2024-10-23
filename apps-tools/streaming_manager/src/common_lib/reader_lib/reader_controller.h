#ifndef READER_CONTROLLER_H
#define READER_CONTROLLER_H

#include "settings_lib/stream_settings.h"
#include "tdms_lib/file.h"
#include "wav_lib/wav_reader.h"
#include <string>
#include <vector>

/**
	 * Error of reading TMDS file if Metadata block has empty data.
	 * Data is dublicated
	 */

using namespace std;

class CReaderController
{
public:
	struct DataIn
	{
		uint8_t *ch[2] = {nullptr, nullptr};
		size_t size[2] = {0, 0};
		uint8_t bits = {0};
		size_t readPosition = 0;
	};

	struct Data
	{
		uint8_t *ch[2] = {nullptr,nullptr};
		size_t size[2] = {0,0};
		size_t real_size[2] = {0,0};
		uint8_t bits = {0};
	};

	enum OpenResult {
		OR_OK = 0,
		OR_MISSING_CHANNELS = 1,
		OR_WRONG_DATA_TYPE = 2,
		OR_DATA_NOT_EQUAL = 3,
		OR_CLOSE = 4
	};

	enum BufferResult {
		BR_OK = 0,
		BR_ENDED = 1,
		BR_BROKEN = 2,
		BR_EMPTY = 3
	};

	using Ptr = shared_ptr<CReaderController>;
	static Ptr Create(CStreamSettings::DataFormat _fileType,
					  string _filePath,
					  CStreamSettings::DACRepeat _repeat,
					  uint32_t _rep_count,
					  uint32_t blockSize);

	static Ptr Create(DataIn *dataIn, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count, uint32_t blockSize);

	CReaderController(CStreamSettings::DataFormat _fileType,
					  string _filePath,
					  CStreamSettings::DACRepeat _repeat,
					  uint32_t _rep_count,
					  uint32_t blockSize);

	CReaderController(DataIn *dataIn, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count, uint32_t blockSize);

	~CReaderController();

	auto isOpen() -> CReaderController::OpenResult;
	auto checkFile() -> OpenResult;
	auto getBufferPrepared(Data &data) -> BufferResult;
	auto getChannels(bool *ch1Active, bool *ch2Active) -> void;
	auto getChannelsSize(size_t *ch1Size, size_t *ch2Size) -> void;
	auto disableRepeatMode() -> void;

private:
	struct TemperaryBuffer
	{
		uint8_t *buffer = nullptr;
		size_t size = 0;
		size_t current_pos = 0;
		uint8_t bits = 0;
		bool memoryMode = false;
		void deleteBuffer();
		bool isEnded() { return size == current_pos; }
		TemperaryBuffer(){};
		TemperaryBuffer(const TemperaryBuffer &) = delete;
		TemperaryBuffer(TemperaryBuffer &&) = delete;
		TemperaryBuffer &operator=(const TemperaryBuffer &) = delete;
		TemperaryBuffer &operator=(TemperaryBuffer &&) = delete;
	};

	CReaderController(CReaderController const &) = delete;
	CReaderController &operator=(CReaderController const &) = delete;

	auto checkTDMSFile() -> OpenResult;
	auto checkWavFile() -> OpenResult;
	auto checkMemory() -> OpenResult;
	auto getBuffer(Data &data) -> bool;
	auto getBufferWav(Data &data) -> bool;
	auto getBufferTdms(Data &data) -> bool;
	auto openWav() -> bool;
	auto openTDMS() -> bool;
	auto moveNextMetadata() -> bool;
	auto resetReadFromBuffer() -> bool;
	auto writeFromTemp(uint8_t **buff, size_t max_size, size_t *write_pos, CReaderController::TemperaryBuffer *temp_buf, size_t *realSize)
		-> void;

	CStreamSettings::DataFormat m_fileType;
	string m_filePath;
	CStreamSettings::DACRepeat m_repeat;
	uint32_t m_rep_count;
	CWaveReader *m_wavReader;
	TDMS::File *m_tdmsFile;
	vector<shared_ptr<TDMS::Segment>> m_tdmsSegments;
	uint32_t m_currentSegment;
	uint32_t m_currentMetadata;
	vector<shared_ptr<TDMS::Metadata>> m_currentVecMetadataPtr;
	shared_ptr<TDMS::Metadata> m_currentMetadataPtr;
	OpenResult m_result;
	bool m_channel1Present;
	bool m_channel2Present;
	TemperaryBuffer m_tempBuffer[2];
	bool m_checkEmptyFile;
	uint64_t m_memoryCacheSize;
	size_t m_channel1Size;
	size_t m_channel2Size;
	uint32_t m_blockSize;
	uint8_t *m_dataBuffers[2];
	DataIn *m_genData;
};

#endif
