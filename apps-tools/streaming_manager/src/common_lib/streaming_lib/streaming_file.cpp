#include <time.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "data_lib/neon_asm.h"
#include "logger_lib/file_logger.h"
#include "streaming_file.h"
#include "tdms_lib/tdms.h"

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif

using namespace streaming_lib;

auto createDir(const std::string& dir) -> bool {
#ifdef _WIN32
    mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0777);
#endif
    return true;
}

auto createDirTree(const std::string& full_path) -> bool {
    char ch = '/';
#ifdef _WIN32
    ch = '\\';
#endif

    size_t pos = 0;
    bool ret_val = true;
    while (ret_val == true && pos != std::string::npos) {
        pos = full_path.find(ch, pos + 1);
        ret_val = createDir(full_path.substr(0, pos));
    }
    return ret_val;
}

auto getNewFileName(CStreamSettings::DataFormat _fileType, std::string& _filePath, std::string& _prefix) -> std::string {
    createDirTree(_filePath);

    //    char time_str[40];
    //    struct tm *timenow;
    //    time_t now = time(nullptr);
    //    timenow = gmtime(&now);
    //    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);

    std::string filename = _filePath + "/" + std::string("data_file_") + _prefix + ".";
    if (_fileType.value == CStreamSettings::DataFormat::TDMS)
        filename += "tdms";
    if (_fileType.value == CStreamSettings::DataFormat::WAV)
        filename += "wav";
    if (_fileType.value == CStreamSettings::DataFormat::BIN)
        filename += "bin";

    return filename;
}

auto CStreamingFile::makeEmptyDir(const std::string& _filePath) -> void {
    createDirTree(_filePath);
}

auto CStreamingFile::create(CStreamSettings::DataFormat _fileType, std::string& _filePath, uint64_t _samples, bool _v_mode, bool testMode, bool _rp_mode) -> CStreamingFile::Ptr {
    return std::make_shared<CStreamingFile>(_fileType, _filePath, _samples, _v_mode, testMode, _rp_mode);
}

CStreamingFile::CStreamingFile(CStreamSettings::DataFormat _fileType, std::string& _filePath, uint64_t _samples, bool _v_mode, bool testMode, bool _rp_mode)
    : m_fileLogger(nullptr),
      m_ReadyToPass(0),
      m_SendData(0),
      m_file_manager(nullptr),
      m_filePath(_filePath),
      m_file_out(""),
      m_samples(_samples),
      m_passSizeSamples(),
      m_testMode(testMode),
      m_volt_mode(_v_mode),
      m_disableNotify(false),
      m_rp_mode(_rp_mode),
      m_fileType(_fileType) {
    m_file_manager = new FileQueueManager(testMode);
    m_waveWriter = new CWaveWriter();

    m_file_manager->outSpaceNotify.connect([&]() { stop(CStreamingFile::OUT_SPACE, false); });
}

CStreamingFile::~CStreamingFile() {
    stopImmediately();  // joins the watch thread
    if (m_file_manager) {
        delete m_file_manager;
        m_file_manager = nullptr;
    }

    if (m_waveWriter) {
        delete m_waveWriter;
        m_waveWriter = nullptr;
    }
    TRACE("Exit")
}

auto CStreamingFile::disableNotify() -> void {
    m_disableNotify = true;
}

auto CStreamingFile::isFileThreadWork() -> bool {
    if (m_file_manager) {
        return m_file_manager->isWork();
    }
    return false;
}

auto CStreamingFile::isOutOfSpace() -> bool {
    if (m_file_manager) {
        return m_file_manager->isOutOfSpace();
    }
    return false;
}

auto CStreamingFile::setLimits(double seconds, uint64_t bytes, double bufferSeconds) -> void {
    m_limitTime = seconds > 0 ? seconds : 0;
    m_limitSize = bytes;
    m_bufferHint = bufferSeconds > 0 ? bufferSeconds : 0;
}

auto CStreamingFile::startWatch() -> void {
    stopWatch();
    if (m_limitTime <= 0)
        return;
    m_clock.start(m_limitTime, m_bufferHint);
    {
        std::lock_guard lock(m_watchMtx);
        m_watchStop = false;
    }
    std::lock_guard threadLock(m_threadMtx);
    m_watch = std::thread([this]() {
        std::unique_lock lock(m_watchMtx);
        while (!m_watchStop) {
            if (m_clock.due()) {
                lock.unlock();
                aprintf(stdout, "The time limit of %.3f s passed on the clock with no buffer to end it (a trigger not come)\n", m_limitTime);
                stop(CStreamingFile::REACH_LIMIT, true);
                return;  // nothing of the object after the stop: it may go now
            }
            m_watchCv.wait_for(lock, std::chrono::milliseconds(100));
        }
    });
}

// Not under m_stopMtx: the thread may be in stop() itself
auto CStreamingFile::stopWatch() -> void {
    m_clock.off();
    {
        std::lock_guard lock(m_watchMtx);
        m_watchStop = true;
    }
    m_watchCv.notify_all();
    // joined out of the lock (it may be in stop() itself); from it, it stays for a later join
    std::thread done;
    {
        std::lock_guard lock(m_threadMtx);
        if (m_watch.joinable() && m_watch.get_id() != std::this_thread::get_id())
            done = std::move(m_watch);
    }
    if (done.joinable())
        done.join();
}

auto CStreamingFile::setSplit(uint64_t bytes, double seconds) -> void {
    m_splitSize = bytes;
    m_splitTime = seconds > 0 ? seconds : 0;
}

auto CStreamingFile::getFileNames() -> std::vector<std::string> {
    std::lock_guard lock(m_logMtx);
    return m_files;
}

auto CStreamingFile::getPartSize() -> uint64_t {
    return m_file_manager ? m_file_manager->fileSize() : 0;
}

auto CStreamingFile::logger() -> CFileLogger::Ptr {
    std::lock_guard lock(m_logMtx);
    return m_fileLogger;
}

// <name>_0003.<ext>: the number before the extension
auto CStreamingFile::partName(int part) -> std::string {
    return partFileName(m_base, part);
}

// The header of each format comes again with the next chunk
auto CStreamingFile::resetHeaders() -> void {
    m_tdmsStarted = false;
    m_bin = BinFileState();
    m_waveWriter->resetHeaderInit();
    m_partFresh = true;
}

// The file thread ends the part after the buffers queued before and goes on in the next one; the
// report of the part is written now. Not after a stop (another thread may stop: the disk full)
auto CStreamingFile::nextPart() -> bool {
    std::lock_guard stopLock(m_stopMtx);
    if (m_stopped)
        return false;
    const int part = ++m_part;
    const std::string name = partName(part);
    CFileLogger::Ptr old;
    {
        std::lock_guard lock(m_logMtx);
        old = m_fileLogger;
        if (old)
            m_fileLostDone += old->getFileLost();
        m_fileLogger = CFileLogger::create(name + ".log", m_testMode);
        m_file_out = name;
        m_files.push_back(name);
    }
    if (old) {
        old->setFileWrite(m_file_manager->writeRate(), m_file_manager->queueLimit());
        if (!m_testMode)
            old->dumpToFile();
    }
    resetHeaders();
    m_file_manager->addFileSwitch(name);
    m_partBytes = 0;
    m_partLine = 0;
    aprintf(stdout, "Run write to: %s\n", name.c_str());
    return true;
}

auto CStreamingFile::put(FileChunk::Ptr chunk, DataLib::CDataBuffersPackDMA::Ptr pack, const CFileLogger::Ptr& log) -> bool {
    const uint64_t size = chunk ? chunk->size() : 0;
    // the size limit: the chunk that would go over it is not written
    if (m_limitSize && m_allBytes + size > m_limitSize) {
        stop(CStreamingFile::REACH_LIMIT, true);
        return false;
    }
    if (m_file_manager->addChunkToWrite(std::move(chunk))) {
        m_partBytes += size;
        m_allBytes += size;
        m_partFresh = false;
    } else {
        log->addMetric(CFileLogger::EMetric::FILESYSTEM_RATE, 1);
        dropped(pack);
        // the header was in it: the next chunk carries it
        if (m_partFresh)
            resetHeaders();
    }
    return true;
}

auto CStreamingFile::run(std::string _prefix, std::string _fileName) -> void {
    m_passSizeSamples.clear();
    m_passSizeSamples[DataLib::CH1] = 0;
    m_passSizeSamples[DataLib::CH2] = 0;
    m_passSizeSamples[DataLib::CH3] = 0;
    m_passSizeSamples[DataLib::CH4] = 0;

    resetHeaders();
    m_stopped = false;
    m_partBytes = m_allBytes = m_partLine = m_line = 0;
    m_base = _fileName == "" ? getNewFileName(m_fileType, m_filePath, _prefix) : _fileName;
    m_part = 1;
    // parts asked for: numbered from the first one (not in the test mode: one file there)
    const bool parts = !m_testMode && (m_splitSize || m_splitTime > 0);
    {
        std::lock_guard lock(m_logMtx);
        m_file_out = parts ? partName(1) : m_base;
        m_fileLogger = CFileLogger::create(m_file_out + ".log", m_testMode);
        m_files = {m_file_out};
        m_fileLostDone = 0;
    }
    aprintf(stdout, "Run write to: %s\n", m_file_out.c_str());
    m_file_manager->openFile(m_file_out, false);
    m_file_manager->startWrite(m_fileType);
    startWatch();
}

auto CStreamingFile::stop(CStreamingFile::EStopReason reason, bool _flush) -> void {
    stopWatch();
    std::lock_guard<std::mutex> lock(m_stopMtx);
    m_stopped = true;
    // a part may have swapped it: the one of now, under its lock
    auto log = logger();
    if (m_file_manager) {
        m_file_manager->stopWrite(_flush);
        // the writer thread is done: the tail goes to the card, the file closes
        m_file_manager->closeFile();
        if (log)
            log->setFileWrite(m_file_manager->writeRate(), m_file_manager->queueLimit());
        if (m_testMode) {
            m_file_manager->deleteFile();
        }
    }
    if (log && !m_testMode)
        log->dumpToFile();
    // once: the teardown may disable it from another thread at the same time
    if (!m_disableNotify.exchange(true))
        stopNotify(reason);
}

auto CStreamingFile::stopImmediately() -> void {
    stop(CStreamingFile::NORMAL, false);
}

auto CStreamingFile::stopAndFlush() -> void {
    stop(CStreamingFile::NORMAL, true);
}

auto CStreamingFile::convertBuffers(DataLib::CDataBuffersPackDMA::Ptr pack, DataLib::EDataBuffersPackChannel channel, bool lockADCTo1V) -> SBuffPass {
    auto src_buff = pack->getBuffer(channel);
    if (!src_buff) {
        auto pbuff = SBuffPass();
        pbuff.buffer = nullptr;
        pbuff.bufferLen = 0;
        pbuff.samplesCount = 0;
        pbuff.bitsBySample = 0;
        pbuff.adcSpeed = 0;
        return pbuff;
    }

    if (!m_volt_mode) {
        auto lostSamples = src_buff->getLostSamplesAll();
        if (m_rp_mode) {  // We cut off the lost values if we save them to a memory card.
            lostSamples = std::min(lostSamples, (uint64_t)src_buff->getSamplesCount());
        }
        uint64_t lostSamplesInBytes = lostSamples * src_buff->getBitBySample() / 8;
        uint64_t destSize = src_buff->getDataLenght() + lostSamplesInBytes;
        auto dest = net_lib::createBuffer(destSize);
        if (dest.get()) {
            // the samples lost are the ones right before the block (as in BIN): zeros first
            memset(dest.get(), 0, sizeof(uint8_t) * lostSamplesInBytes);
            memcpy_neon(dest.get() + lostSamplesInBytes, src_buff->getMappedDataMemory(), src_buff->getDataLenght());
        } else {
            aprintf(stderr, "There is not enough memory to allocate a buffer Requred size: %lld bytes.\n", destSize);
        }
        //         short *wb2 = ((short*)dest.get());
        //         for(int i = 0 ;i < (destSize) /2 ;i ++)
        //             std::cerr << std::hex <<  (static_cast<int>(wb2[i]) & 0xFFFF)  << " ";
        //             exit(1);
        auto pbuff = SBuffPass();
        pbuff.buffer = dest;
        pbuff.bufferLen = destSize;
        pbuff.samplesCount = src_buff->getSamplesCount() + lostSamples;
        pbuff.bitsBySample = src_buff->getBitBySample();
        pbuff.adcSpeed = src_buff->getADCBaseRate();
        return pbuff;
    } else {
        auto samples = src_buff->getSamplesCount();
        auto lostSamples = src_buff->getLostSamplesAll();
        if (m_rp_mode) {  // We cut off the lost values if we save them to a memory card.
            lostSamples = std::min(lostSamples, (uint64_t)src_buff->getSamplesCount());
        }
        auto bitBySamp = src_buff->getBitBySample();
        auto adcMode = (src_buff->getADCMode() == DataLib::CDataBufferDMA::ATT_1_20 ? 20 : 1);
        if (lockADCTo1V) {
            adcMode = 1;
        }
        uint64_t destSize = (samples + lostSamples) * sizeof(float);
        auto dest = net_lib::createBuffer(destSize);
        if (dest.get()) {
            // the samples lost are the ones right before the block (as in BIN): zeros first
            memset(dest.get(), 0, sizeof(float) * lostSamples);
            auto dest_f = (float*)dest.get() + lostSamples;
            for (uint32_t i = 0; i < samples; i++) {
                if (bitBySamp == 8) {
                    int8_t* src_buff_bytes = reinterpret_cast<int8_t*>(src_buff->getMappedDataMemory());
                    float cnt = src_buff_bytes[i];
                    dest_f[i] = cnt / (1 << (bitBySamp - 1)) * (float)adcMode;
                }

                if (bitBySamp == 16) {
                    auto src_buff_bytes = reinterpret_cast<int16_t*>(src_buff->getMappedDataMemory());
                    float cnt = src_buff_bytes[i];
                    dest_f[i] = cnt / (1 << (bitBySamp - 1)) * (float)adcMode;
                }
            }
        } else {
            aprintf(stderr, "There is not enough memory to allocate a buffer Requred size: %lld bytes.\n", destSize);
        }
        auto pbuff = SBuffPass();
        pbuff.buffer = dest;
        pbuff.bufferLen = destSize;
        pbuff.samplesCount = samples + lostSamples;
        pbuff.bitsBySample = 32;
        pbuff.adcSpeed = src_buff->getADCBaseRate();
        return pbuff;
    }
}

auto CStreamingFile::setTDMSTimeColumn(bool enable) -> void {
    m_tdmsTimeColumn = enable;
}

namespace {

const char* const kTDMSChannels[4] = {"ch1", "ch2", "ch3", "ch4"};

// The sample rate: exact from the ADC clock and the decimation when the header has them, else
// the integer rate (62.5 MHz / 64 gives 976562 there)
auto sampleRate(DataLib::CDataBuffersPackDMA::Ptr pack) -> double {
    for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
        auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)i);
        if (buff && buff->getADCClock() && buff->getDecimation())
            return static_cast<double>(buff->getADCClock()) / buff->getDecimation();
    }
    return static_cast<double>(pack->getOSCRate());
}

// The first channel of a pack, CH4 + 1 when it has none
auto firstChannel(DataLib::CDataBuffersPackDMA::Ptr pack) -> int {
    int i = (int)DataLib::CH1;
    while (i <= (int)DataLib::CH4 && !pack->getBuffer((DataLib::EDataBuffersPackChannel)i))
        i++;
    return i;
}

}  // namespace

// The properties of the recording, in its first segment
auto CStreamingFile::addTDMSProperties(TDMS::SegmentBuilder& segment, DataLib::CDataBuffersPackDMA::Ptr pack, std::map<DataLib::EDataBuffersPackChannel, SBuffPass>& map)
    -> void {
    const int firstCh = firstChannel(pack);
    if (m_tdmsStarted || firstCh > (int)DataLib::CH4)
        return;
    m_tdmsStarted = true;
    auto first = pack->getBuffer((DataLib::EDataBuffersPackChannel)firstCh);
    const int64_t timeCapture = pack->getTimeCapture();
    const double rate = sampleRate(pack);  // after the decimation

    segment.setGroupProperty("Group", "sample_rate", rate);
    segment.setGroupProperty("Group", "data_type", m_volt_mode ? "Volt" : "Raw");
    segment.setGroupProperty("Group", "resolution_bits", first->getBitBySample());
    if (first->getDecimation()) {
        segment.setGroupProperty("Group", "decimation", first->getDecimation());
        segment.setGroupProperty("Group", "adc_rate", first->getADCClock() ? static_cast<double>(first->getADCClock()) : rate * first->getDecimation());
        segment.setGroupProperty("Group", "trigger_mode", (first->getADCFlags() & DataLib::ADC_FLAG_TRIGGER_MODE) != 0);
    }
    for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
        auto ch = (DataLib::EDataBuffersPackChannel)i;
        auto buff = pack->getBuffer(ch);
        if (!buff || !map[ch].bufferLen)
            continue;
        if (rate)
            segment.setChannelProperty("Group", kTDMSChannels[i], TDMS::WaveformProperty::Increment, 1.0 / rate);
        if (timeCapture) {
            segment.setChannelProperty("Group", kTDMSChannels[i], TDMS::WaveformProperty::StartTime, TDMS::TimeStamp::fromUnixNs(timeCapture));
            segment.setChannelProperty("Group", kTDMSChannels[i], TDMS::WaveformProperty::StartOffset, 0.0);
        }
        if (m_volt_mode)
            segment.setChannelProperty("Group", kTDMSChannels[i], TDMS::WaveformProperty::Unit, "V");
        segment.setChannelProperty("Group", kTDMSChannels[i], "attenuator", buff->getADCMode() == DataLib::CDataBufferDMA::ATT_1_20 ? "1:20" : "1:1");
    }
}

// A row per buffer in the group "Buffers": when it starts, the samples written, and what is
// missing right before it (overflow, trigger wait)
auto CStreamingFile::addTDMSBufferRow(TDMS::SegmentBuilder& segment, DataLib::CDataBuffersPackDMA::Ptr pack, std::map<DataLib::EDataBuffersPackChannel, SBuffPass>& map)
    -> void {
    const int firstCh = firstChannel(pack);
    if (firstCh > (int)DataLib::CH4)
        return;
    auto first = pack->getBuffer((DataLib::EDataBuffersPackChannel)firstCh);
    const int64_t start = pack->getTimeCapture();
    const uint64_t samples = map[(DataLib::EDataBuffersPackChannel)firstCh].samplesCount;
    const uint64_t lost = first->getLostSamplesAll();
    const uint64_t wait = first->getWaitSamples();
    segment.copySamples("Buffers", "start_time_ns", &start, 1);
    segment.copySamples("Buffers", "samples", &samples, 1);
    segment.copySamples("Buffers", "lost", &lost, 1);
    segment.copySamples("Buffers", "wait", &wait, 1);
}

auto CStreamingFile::dropped(DataLib::CDataBuffersPackDMA::Ptr pack) -> void {
    uint64_t samples = 0;
    for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
        auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)i);
        if (buff)
            samples = std::max<uint64_t>(samples, buff->getSamplesCount());
    }
    m_droppedSamples += samples;
}

auto CStreamingFile::getFileLost() -> uint64_t {
    std::lock_guard lock(m_logMtx);
    return m_fileLostDone + (m_fileLogger ? m_fileLogger->getFileLost() : 0);
}

// Without m_stopMtx: a stop holds it while the queue goes into the file (seconds), and the page
// and the record status ask every 100 ms. m_file_manager lives as long as this object.
auto CStreamingFile::getQueueFill() -> double {
    return m_file_manager ? m_file_manager->queueFill() : 0;
}

auto CStreamingFile::getCSVFileName() -> std::string {
    if (m_file_manager) {
        auto now = m_file_manager->fileName();
        if (!now.empty())
            return now;
    }
    std::lock_guard lock(m_logMtx);
    return m_file_out;
}

auto CStreamingFile::getPart() -> int {
    const int files = m_file_manager ? m_file_manager->filesWritten() : 0;
    return files > 0 ? files : m_part.load();
}

auto CStreamingFile::getPassSamples() -> uint64_t {
    uint64_t all = 0;
    for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
        DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
        all = std::max(all, m_passSizeSamples[ch]);
    }
    return all;
}

auto CStreamingFile::getWriteRate() -> double {
    return m_file_manager ? m_file_manager->writeRate() : 0;
}

auto CStreamingFile::getWritedSize() -> uint64_t {
    if (m_file_manager) {
        return m_file_manager->getWritedSize();
    }
    return 0;
}

auto CStreamingFile::passBuffers(DataLib::CDataBuffersPackDMA::Ptr pack) -> int {
    if (!pack)
        return 0;
    // stopped (a limit, the disk): nothing more goes to the file
    if (m_stopped)
        return 1;
    // what the pack adds: samples of the time line, of the data, about how many bytes
    uint64_t packLine = 0, packData = 0, packBytes = 0;
    for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
        auto buff = pack->getBuffer((DataLib::EDataBuffersPackChannel)i);
        if (!buff)
            continue;
        packLine = std::max<uint64_t>(packLine, buff->getSamplesWithGaps());
        packData = std::max<uint64_t>(packData, buff->getSamplesCount());
        uint64_t lost = m_fileType.value == CStreamSettings::DataFormat::BIN ? 0 : buff->getLostSamplesAll();
        // on the board the zeros of the losses are cut at the size of the buffer (convertBuffers)
        if (m_rp_mode)
            lost = std::min<uint64_t>(lost, buff->getSamplesCount());
        const uint64_t bytes = m_volt_mode && m_fileType.value != CStreamSettings::DataFormat::BIN ? 4 : std::max<uint64_t>(1, buff->getBitBySample() / 8);
        packBytes += (buff->getSamplesCount() + lost) * bytes + 64;
    }
    const double packRate = sampleRate(pack);
    // how long a buffer of data lasts (not its wait for a trigger)
    if (packRate > 0)
        m_clock.buffer(static_cast<double>(packData) / packRate);
    // the time limit: a pack whose data starts past it (after a wait for the trigger) stays out
    if (m_limitTime > 0 && packRate > 0 && static_cast<double>(m_line + (packLine - std::min(packLine, packData))) >= m_limitTime * packRate) {
        stop(CStreamingFile::REACH_LIMIT, true);
        return 1;
    }
    // the next part: the size, the time, or a WAV before 4 GB
    if (!m_testMode && m_partBytes > 0) {
        const bool bySize = m_splitSize && m_partBytes + packBytes > m_splitSize;
        const bool byTime = m_splitTime > 0 && packRate > 0 && static_cast<double>(m_partLine) >= m_splitTime * packRate;
        const bool wav = m_fileType.value == CStreamSettings::DataFormat::WAV && m_partBytes + packBytes > m_wavPartMax;
        if (bySize || byTime || wav) {
            // no part begun for a pack the size limit keeps out
            if (m_limitSize && m_allBytes + packBytes > m_limitSize) {
                stop(CStreamingFile::REACH_LIMIT, true);
                return 1;
            }
            if (!nextPart())
                return 1;
        }
    }
    m_partLine += packLine;
    m_line += packLine;
    // the logger of the part (nextPart swaps it)
    auto log = logger();
    if (m_fileType.value == CStreamSettings::DataFormat::TDMS) {
        // _adc_mode = 0 for 1:1 and 1 for 1:20 mode

        auto bufferCh1 = convertBuffers(pack, DataLib::CH1, false);
        auto bufferCh2 = convertBuffers(pack, DataLib::CH2, false);
        auto bufferCh3 = convertBuffers(pack, DataLib::CH3, false);
        auto bufferCh4 = convertBuffers(pack, DataLib::CH4, false);

        auto map = std::map<DataLib::EDataBuffersPackChannel, SBuffPass>();
        map[DataLib::CH1] = bufferCh1;
        map[DataLib::CH2] = bufferCh2;
        map[DataLib::CH3] = bufferCh3;
        map[DataLib::CH4] = bufferCh4;
        bool noMemoryException = false;
        for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
            DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
            if (map[ch].buffer.get() == nullptr && map[ch].bufferLen) {
                noMemoryException = true;
            }
        }
        if (!noMemoryException) {
            if (m_samples != 0) {
                for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
                    DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
                    if (map[ch].samplesCount + m_passSizeSamples[ch] > m_samples) {
                        map[ch].samplesCount = m_samples - m_passSizeSamples[ch];
                        map[ch].bufferLen = map[ch].samplesCount * (map[ch].bitsBySample / 8);
                        m_passSizeSamples[ch] += map[ch].samplesCount;
                    } else {
                        m_passSizeSamples[ch] += map[ch].samplesCount;
                    }
                }
            } else {
                for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
                    DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
                    m_passSizeSamples[ch] += map[ch].samplesCount;
                }
            }

            // the time column has a value per sample written: past the sample limit there are fewer
            uint64_t bufSamples = 0;
            for (auto& kv : map)
                bufSamples = std::max<uint64_t>(bufSamples, kv.second.samplesCount);
            auto time_cap = pack->getTimeCapture();

            // A vector per segment: the queued segment references it until the file thread writes it
            std::shared_ptr<std::vector<int64_t>> time;
            if (m_tdmsTimeColumn && time_cap != 0 && bufSamples != 0) {
                time = std::make_shared<std::vector<int64_t>>();
                time->resize(bufSamples);
                double frequency_hz = sampleRate(pack);
                double period_ns = (frequency_hz > 0) ? (1.0 / frequency_hz) * 1e9 : 0;
                for (auto idx = 0u; idx < bufSamples; idx++) {
                    time->at(idx) = time_cap + (uint64_t)(period_ns * idx);
                }
            }

            auto segment = std::make_shared<TDMS::SegmentBuilder>();
            addTDMSProperties(*segment, pack, map);
            addTDMSSamples(*segment, map, time);
            addTDMSBufferRow(*segment, pack, map);
            if (m_file_manager->isWork() && !put(buildTDMSChunk(segment), pack, log))
                return 1;
        } else {
            log->addMetric(CFileLogger::EMetric::OUT_OF_MEMORY, 1);
            dropped(pack);
        }
    }

    if (m_fileType.value == CStreamSettings::DataFormat::WAV) {
        // WAV type support float full range -1...1. adc_mode always equal 1
        auto bufferCh1 = convertBuffers(pack, DataLib::CH1, true);
        auto bufferCh2 = convertBuffers(pack, DataLib::CH2, true);
        auto bufferCh3 = convertBuffers(pack, DataLib::CH3, true);
        auto bufferCh4 = convertBuffers(pack, DataLib::CH4, true);

        auto map = std::map<DataLib::EDataBuffersPackChannel, SBuffPass>();
        map[DataLib::CH1] = bufferCh1;
        map[DataLib::CH2] = bufferCh2;
        map[DataLib::CH3] = bufferCh3;
        map[DataLib::CH4] = bufferCh4;

        bool noMemoryException = false;
        for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
            DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
            if (map[ch].buffer.get() == nullptr && map[ch].bufferLen) {
                TRACE("noMemoryException")
                noMemoryException = true;
            }
        }

        if (!noMemoryException) {
            if (m_samples != 0) {
                for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
                    DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
                    if ((map[ch].samplesCount + m_passSizeSamples[ch]) > m_samples) {
                        map[ch].samplesCount = m_samples - m_passSizeSamples[ch];
                        map[ch].bufferLen = map[ch].samplesCount * (map[ch].bitsBySample / 8);
                        m_passSizeSamples[ch] += map[ch].samplesCount;
                    } else {
                        m_passSizeSamples[ch] += map[ch].samplesCount;
                    }
                }
            } else {
                for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
                    DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
                    m_passSizeSamples[ch] += map[ch].samplesCount;
                }
            }
            auto chunk = m_waveWriter->BuildWAVChunk(map);
            if (m_file_manager->isWork() && !put(std::move(chunk), pack, log))
                return 1;
        } else {
            log->addMetric(CFileLogger::EMetric::OUT_OF_MEMORY, 1);
            dropped(pack);
        }
    }

    if (m_fileType.value == CStreamSettings::DataFormat::BIN) {
        // This algorithm is faster than copying data to TDMS and WAV. Don't redo it
        auto map = std::map<DataLib::EDataBuffersPackChannel, uint32_t>();
        if (m_samples != 0) {
            for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
                DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
                auto buff = pack->getBuffer(ch);
                if (buff.get()) {
                    if (m_passSizeSamples[ch] > m_samples) {
                        buff->reset();
                        map[ch] = 0;
                    } else if (m_passSizeSamples[ch] + buff->getSamplesCount() > m_samples) {
                        map[ch] = m_samples - m_passSizeSamples[ch];
                        m_passSizeSamples[ch] += map[ch];
                    } else {
                        m_passSizeSamples[ch] += buff->getSamplesWithLost();
                        map[ch] = buff->getSamplesCount();
                    }
                }
            }
        } else {
            for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
                DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
                auto buff = pack->getBuffer(ch);
                if (buff.get()) {
                    m_passSizeSamples[ch] += buff->getSamplesWithLost();
                    map[ch] = buff->getSamplesCount();
                }
            }
        }

        if (m_file_manager->isWork() && !put(buildBINChunk(pack, map, m_bin), pack, log))
            return 1;
    }
    uint64_t rate = 0;
    for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
        DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
        auto buff = pack->getBuffer(ch);
        if (buff) {
            log->addMetric(CFileLogger::EMetric::RECIVE_DATE, buff->getDataLenght());
            log->addMetric(ch, buff->getDataLenght(), buff->getLostSamples(DataLib::FPGA), buff->getWaitSamples(), buff->getSamplesCount());
            rate = std::max(rate, buff->getADCBaseRate());
        }
    }
    log->addMetric(CFileLogger::EMetric::OSC_RATE, rate);
    log->addMetric(pack);

    if (m_samples) {
        bool reachLimits = true;
        for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
            DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
            if (pack->isChannelPresent(ch)) {
                if (m_passSizeSamples[ch] < m_samples) {
                    reachLimits = false;
                    break;
                }
            }
        }
        if (reachLimits) {
            stop(CStreamingFile::REACH_LIMIT, true);
            return 1;
        }
    }
    // the time limit: the time line of the stream (the waits count)
    if (m_limitTime > 0 && packRate > 0 && static_cast<double>(m_line) >= m_limitTime * packRate)
        stop(CStreamingFile::REACH_LIMIT, true);
    return 1;
}
