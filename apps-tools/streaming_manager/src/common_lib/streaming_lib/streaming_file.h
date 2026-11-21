#ifndef STREAMING_LIB_STREAMING_FILE_H
#define STREAMING_LIB_STREAMING_FILE_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>

#include "data_lib/signal.hpp"
#include "logger_lib/file_logger.h"
#include "settings_lib/stream_settings.h"
#include "wav_lib/wav_writer.h"
#include "writer_lib/file_helper.h"
#include "writer_lib/file_queue_manager.h"
#include "writer_lib/time_limit_clock.h"

#define FILE_PATH "/home/redpitaya/streaming_files/adc"
#define ZERO_BUFFER_SIZE 1048576

namespace streaming_lib {

class CStreamingFile {
   public:
    enum EStopReason { NORMAL = 0, OUT_SPACE = 1, REACH_LIMIT = 2 };

    static auto makeEmptyDir(const std::string& _filePath) -> void;

    using Ptr = std::shared_ptr<CStreamingFile>;

    static auto create(CStreamSettings::DataFormat _fileType, std::string& _filePath, uint64_t _samples, bool _v_mode, bool testMode,
                       bool _rp_mode = false) -> Ptr;

    CStreamingFile(CStreamSettings::DataFormat _fileType, std::string& _filePath, uint64_t _samples, bool _v_mode, bool testMode, bool _rp_mode);
    ~CStreamingFile();

    auto run(std::string _prefix, std::string fileName = "") -> void;
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
    // the file queue: what it holds of its memory, 0..1
    auto getQueueFill() -> double;
    // the measured write speed of the file, bytes per second (0 before it is known)
    auto getWriteRate() -> double;
    // samples per channel that did not go into the file: the write queue was full or a buffer
    // could not be converted (memory). Their buffers are missing from the file.
    auto getDroppedSamples() -> uint64_t { return m_droppedSamples; }

    // TDMS: a column with the time of every sample (off by default; the time is always in the
    // wf_* properties of the channels and in the row of every buffer in the group "Buffers")
    auto setTDMSTimeColumn(bool enable) -> void;

    // Besides the sample limit of create(): stop after `seconds` of the time line of the stream
    // (the trigger waits count) or before the data would go over `bytes`; 0 - none. Taken at a
    // buffer: the one that crosses the time limit is written, one whose data starts past it is
    // not. While a trigger is waited for no buffer comes: the time limit ends the recording on the
    // clock too, once the limit, a buffer and a margin passed (TimeLimitClock); `bufferSeconds`:
    // about how long a buffer lasts (0 - unknown: learnt from the buffers). Before run().
    auto setLimits(double seconds, uint64_t bytes, double bufferSeconds = 0) -> void;
    // the tests: the margin of the clock of the time limit
    auto setTimeMargin(double seconds) -> void { m_clock.setMargin(seconds); }
    // the time limit ended the recording on the clock, no buffer came (a REACH_LIMIT stop)
    auto endedOnTheClock() -> bool { return m_byClock; }
    // The recording in parts of about `bytes` of data or `seconds` of the time line (0 - none):
    // <name>_0001.<ext>, _0002 ..., each a whole file with its header and its report, its time
    // line from 0 and the time of its first sample in its header; the stream goes on between
    // them. A WAV goes on in a new part before 4 GB in any case (32 bit sizes in its header):
    // without parts asked for, the first file keeps its name and the next ones are _0002 ...
    // Before run().
    auto setSplit(uint64_t bytes, double seconds) -> void;
    // the part the file thread writes now, from 1 (0 before run); the file of it is
    // getCSVFileName(), its bytes getPartSize(). The stream may be in the next one already: the
    // three go together on the card
    auto getPart() -> int;
    // every file of the recording so far, the first one first
    auto getFileNames() -> std::vector<std::string>;
    // bytes in the part written now
    auto getPartSize() -> uint64_t;
    // the most a WAV part takes (its header has 32 bit sizes): 4 GB less a margin for a buffer
    static constexpr uint64_t kWavPartMax = 0xFFFFFFFFull - 64ull * 1024 * 1024;
    // the tests: a smaller one
    auto setWavPartMax(uint64_t bytes) -> void { m_wavPartMax = bytes; }
    // the tests: the next `n` chunks refused as by a full queue
    auto refuseChunks(int n) -> void { m_file_manager->refuseChunks(n); }

   private:
    CStreamingFile(const CStreamingFile&) = delete;
    CStreamingFile(CStreamingFile&&) = delete;
    CStreamingFile& operator=(const CStreamingFile&) = delete;
    CStreamingFile& operator=(const CStreamingFile&&) = delete;

    CFileLogger::Ptr m_fileLogger;

    std::atomic_int m_ReadyToPass;
    std::atomic_int m_SendData;
    std::atomic<uint64_t> m_droppedSamples{0};
    auto dropped(DataLib::CDataBuffersPackDMA::Ptr pack) -> void;
    FileQueueManager* m_file_manager;
    CWaveWriter* m_waveWriter;
    std::string m_host;
    std::string m_port;
    std::string m_filePath;
    std::string m_file_out;  // the part written now (m_logMtx)
    std::string m_base;      // the name given to run(): the parts are named from it
    std::vector<std::string> m_files;  // of the recording (m_logMtx)
    uint64_t m_samples;
    double m_limitTime = 0;
    double m_bufferHint = 0;
    // the time limit on the clock: a thread that stops the recording when it is due
    TimeLimitClock m_clock;
    std::thread m_watch;    // m_threadMtx: any thread may stop
    std::mutex m_threadMtx;
    std::mutex m_watchMtx;  // m_watchStop
    std::condition_variable m_watchCv;
    bool m_watchStop = false;
    std::atomic<bool> m_byClock{false};
    auto startWatch() -> void;
    auto stopWatch() -> void;
    uint64_t m_limitSize = 0;
    uint64_t m_splitSize = 0;
    uint64_t m_wavPartMax = kWavPartMax;
    double m_splitTime = 0;
    std::atomic<int> m_part{0};
    uint64_t m_partBytes = 0;  // the data given to the part, to the whole recording
    uint64_t m_allBytes = 0;
    uint64_t m_partLine = 0;  // samples of the time line (with the gaps) of the part, of all
    uint64_t m_line = 0;
    uint64_t m_fileLostDone = 0;  // the file losses of the parts before (their loggers are gone; m_logMtx)
    bool m_partFresh = true;      // no chunk of the file taken yet: the next one carries its header
    std::atomic<bool> m_stopped{false};
    std::mutex m_logMtx;  // m_fileLogger and m_file_out: a part swaps them, other threads read
    auto logger() -> CFileLogger::Ptr;
    auto partName(int part) -> std::string;
    auto nextPart() -> bool;
    auto resetHeaders() -> void;
    // the chunk to the file: false when the size limit stops the recording before it
    auto put(FileChunk::Ptr chunk, DataLib::CDataBuffersPackDMA::Ptr pack, const CFileLogger::Ptr& log) -> bool;
    std::mutex m_stopMtx;
    std::map<DataLib::EDataBuffersPackChannel, uint64_t> m_passSizeSamples;

    bool m_testMode;
    bool m_volt_mode;
    // set by the stop (FPGA or file thread) and by the teardown of the server: atomic
    std::atomic<bool> m_disableNotify{false};
    bool m_rp_mode;
    bool m_tdmsTimeColumn = false;
    bool m_tdmsStarted = false;  // the first segment, with the properties of the recording, is written
    BinFileState m_bin;          // the BIN file: its header, the channels, the time line

    CStreamSettings::DataFormat m_fileType;

    auto stop(EStopReason reason, bool _flush) -> void;
    auto convertBuffers(DataLib::CDataBuffersPackDMA::Ptr pack, DataLib::EDataBuffersPackChannel channel, bool lockADCTo1V) -> SBuffPass;
    auto addTDMSProperties(TDMS::SegmentBuilder& segment, DataLib::CDataBuffersPackDMA::Ptr pack, std::map<DataLib::EDataBuffersPackChannel, SBuffPass>& map) -> void;
    auto addTDMSBufferRow(TDMS::SegmentBuilder& segment, DataLib::CDataBuffersPackDMA::Ptr pack, std::map<DataLib::EDataBuffersPackChannel, SBuffPass>& map) -> void;
};

}  // namespace streaming_lib

#endif
