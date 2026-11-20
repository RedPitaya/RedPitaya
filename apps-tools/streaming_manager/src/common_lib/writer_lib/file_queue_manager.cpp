#include "file_queue_manager.h"
#include <algorithm>
#include <ctime>
#include "file_helper.h"
#include "logger_lib/file_logger.h"
#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#endif

namespace {
// How long the file thread sleeps on an empty queue before it looks at the stop flag again
constexpr std::chrono::milliseconds kWaitForChunk(100);
// The window of limitDirtyPages: at most two of them are dirty at a time
constexpr uint64_t kSyncWindow = 8 * 1024 * 1024;
// The queue takes what the file writes in this time (systemd gives a stop 30 s: the rest is for
// the tail of the file, its report and the sockets)
constexpr double kMaxFlushSeconds = 20;
// before the first measurement, and the least the queue may take
constexpr uint64_t kStartQueueLimit = 32 * 1024 * 1024;
constexpr uint64_t kMinQueueLimit = 8 * 1024 * 1024;
constexpr uint64_t kRateSample = 32 * 1024 * 1024;
}  // namespace

FileQueueManager::FileQueueManager(bool testMode) : Queue() {
    m_threadWork = false;
    m_waitAllWrite = false;
    m_hasErrorWrite = false;
    m_IsOutOfSpace = false;
    th = nullptr;
    m_freeSize = 0;
    m_testMode = testMode;
    m_fileName = "";
    m_hasWriteSize = 0;
}

FileQueueManager::~FileQueueManager() {
    this->stopWrite(false);
    joinSpaceThread();
    closeSyncFd();
}

auto FileQueueManager::closeSyncFd() -> void {
#ifdef __linux__
    if (m_syncFd >= 0)
        ::close(m_syncFd);
#endif
    m_syncFd = -1;
    m_syncStarted = m_syncDone = 0;
}

auto FileQueueManager::limitDirtyPages(bool all) -> void {
#ifdef __linux__
    if (m_syncFd < 0 || m_testMode)
        return;
    auto pos = fs.tellp();
    if (pos < 0)
        return;
    const uint64_t end = static_cast<uint64_t>(pos);
    if (end < m_syncStarted) {  // a seek back (the WAV header): only the tail counts
        return;
    }
    if (!all && end - m_syncStarted < kSyncWindow)
        return;
    // start the write-back of the new window, without waiting
    ::sync_file_range(m_syncFd, static_cast<off64_t>(m_syncStarted), static_cast<off64_t>(end - m_syncStarted), SYNC_FILE_RANGE_WRITE);
    // the window before it: wait until it is on the card, then drop it from the cache
    const uint64_t upTo = all ? end : m_syncStarted;
    if (upTo > m_syncDone) {
        ::sync_file_range(m_syncFd, static_cast<off64_t>(m_syncDone), static_cast<off64_t>(upTo - m_syncDone),
                          SYNC_FILE_RANGE_WAIT_BEFORE | SYNC_FILE_RANGE_WRITE | SYNC_FILE_RANGE_WAIT_AFTER);
        ::posix_fadvise(m_syncFd, static_cast<off_t>(m_syncDone), static_cast<off_t>(upTo - m_syncDone), POSIX_FADV_DONTNEED);
        m_syncDone = upTo;
    }
    m_syncStarted = end;
#else
    (void)all;
#endif
}

auto FileQueueManager::joinSpaceThread() -> void {
    if (!m_spaceThread.joinable())
        return;
    if (m_spaceThread.get_id() == std::this_thread::get_id())
        m_spaceThread.detach();  // the owner is destroyed by its own handler: nothing to wait for
    else
        m_spaceThread.join();
}

auto FileQueueManager::deleteFile() -> void {
    try {
        std::remove(m_fileName.c_str());
    } catch (std::exception& e) {
        aprintf(stderr, "Error delete file: %s err: %s \n", m_fileName.c_str(), e.what());
    }
}

auto FileQueueManager::queueFill() -> double {
    const uint64_t limit = m_queueLimit;
    return limit ? std::min(1.0, static_cast<double>(queueMemory()) / static_cast<double>(limit)) : 0;
}

auto FileQueueManager::addChunkToWrite(FileChunk::Ptr chunk) -> bool {
    if (!chunk || !m_threadWork)
        return false;
    if (m_refuse > 0) {
        m_refuse--;
        return false;
    }
    return pushQueue(std::move(chunk), m_queueLimit);
}

auto FileQueueManager::addFileSwitch(const std::string& name) -> bool {
    if (!m_threadWork)
        return false;
    // a marker of no size: never refused for the memory of the queue
    return pushQueue(FileChunk::nextFile(name), std::numeric_limits<uint64_t>::max());
}

auto FileQueueManager::fileName() -> std::string {
    std::lock_guard lock(m_nameMtx);
    return m_fileName;
}

auto FileQueueManager::openStream(const std::string& FileName, bool Append) -> bool {
    fs.clear();
    fs.open(FileName, std::ios::binary | std::ofstream::out | std::ofstream::in | (Append ? std::ofstream::binary : std::ofstream::trunc));
    if (fs.fail()) {
        fs.clear();
        fs.open(FileName, std::ios::binary | std::ofstream::out | std::ofstream::in | std::ofstream::trunc);
        if (fs.fail()) {
            aprintf(stderr, "File: %s  not exist\n", FileName.c_str());
            return false;
        }
    }
    {
        std::lock_guard lock(m_nameMtx);
        m_fileName = FileName;
    }
    closeSyncFd();
#ifdef __linux__
    m_syncFd = ::open(FileName.c_str(), O_WRONLY | O_CLOEXEC);
    // an appended file: what is there already is not ours to sync
    m_syncStarted = m_syncDone = Append ? static_cast<uint64_t>(std::max<std::streamoff>(0, std::streamoff(fs.seekp(0, std::ios::end).tellp()))) : 0;
#endif
    auto dirName = dirNameOf(FileName);
    if (dirName == "") {
        dirName = ".";
    }
    m_freeSize = getFreeSpaceDisk(dirName);
    if (m_testMode) {
        m_freeSize = std::numeric_limits<unsigned long long>::max();
    }
    m_hasWriteSize = 0;
    m_files++;
    return true;
}

auto FileQueueManager::switchFile(const std::string& name) -> void {
    if (fs.is_open()) {
        fs.flush();
        limitDirtyPages(true);
        fs.close();
    }
    closeSyncFd();
    // the speed measured and the queue limit go on: the same card
    if (!openStream(name, false)) {
        // as a full disk: the recording stops
        m_hasErrorWrite = true;
        m_IsOutOfSpace = true;
        outSpaceNotifyThread();
        return;
    }
    m_firstSectionWrite = false;
}

auto FileQueueManager::openFile(std::string FileName, bool Append) -> void {
    m_files = 0;
    m_allWritten = 0;
    {
        std::lock_guard lock(m_nameMtx);
        m_fileName.clear();
    }
    if (!openStream(FileName, Append))
        return;
    m_memoryLimit = std::max<uint64_t>(kMinQueueLimit, getTotalSystemMemory() / 4);
    m_queueLimit = std::min(kStartQueueLimit, m_memoryLimit);
    m_rate = 0;
    m_rateBytes = 0;
    m_rateTime = {};
    TRACE_SHORT("File queue: up to %d MB of the RAM", (int)(m_memoryLimit / (1024 * 1024)))
}

auto FileQueueManager::closeFile() -> void {
    if (fs.is_open()) {
        fs.flush();
        limitDirtyPages(true);
        fs.close();
    }
    closeSyncFd();
}

auto FileQueueManager::startWrite(CStreamSettings::DataFormat _fileType) -> void {
    m_ThreadRun = true;
    m_threadWork = true;
    m_fileType = _fileType;
    m_firstSectionWrite = false;
    m_waitAllWrite = true;
    m_hasErrorWrite = false;
    m_IsOutOfSpace = false;
    joinSpaceThread();
    m_spaceNotified = false;
    th = new std::thread(&FileQueueManager::task, this);
}

auto FileQueueManager::stopWrite(bool waitAllWrite) -> void {
    if (m_threadWork) {
        m_waitLock.lock();
        m_waitAllWrite = waitAllWrite;
        m_waitLock.unlock();
        m_ThreadRun = false;
    }
    m_threadControl.lock();
    if (th) {
        if (th->joinable()) {
            th->join();
        }
        delete th;
        th = nullptr;
    }
    m_threadControl.unlock();
    stopNotify();
}

auto FileQueueManager::getWritedSize() -> uint64_t {
    return m_allWritten;
}

auto FileQueueManager::task() -> void {
    while (m_ThreadRun) {
        writeToFile(kWaitForChunk);
    }
    m_waitLock.lock();
    if (this->m_waitAllWrite) {
        while (writeToFile() == 0)
            ;
    } else {
        while (popQueue())
            ;
    }
    m_threadWork = false;
    m_waitLock.unlock();
}

auto FileQueueManager::writeToFile(std::chrono::milliseconds wait) -> int {
    auto chunk = popQueue(wait);
    if (chunk == nullptr)
        return -1;

    if (m_hasErrorWrite)
        return 1;

    if (!chunk->nextFileName().empty()) {
        switchFile(chunk->nextFileName());
        return m_hasErrorWrite ? 1 : 0;
    }

    const uint64_t Length = chunk->size();

    if (fs.good() && ((m_hasWriteSize + Length) < m_freeSize)) {
        if (m_testMode) {
            fs.seekp(0);
        }
        const auto t0 = std::chrono::steady_clock::now();
        chunk->writeTo(fs);
        fs.flush();
        m_hasWriteSize += Length;
        m_allWritten += Length;
        limitDirtyPages(false);
        measureWrite(Length, std::chrono::steady_clock::now() - t0);

        if (m_fileType.value == CStreamSettings::DataFormat::WAV) {
            if (m_firstSectionWrite) {
                updateWavFile(Length);
            }

            if (m_firstSectionWrite == false) {
                m_firstSectionWrite = true;
            }
        }

    } else {
        m_IsOutOfSpace = true;
        m_hasErrorWrite = true;
        m_hasWriteSize += Length;
        if (!(m_hasWriteSize < m_freeSize)) {
            aprintf(stdout, "The disc has reached the write limit\n");
        } else {
            aprintf(stdout, "Disk is full or error state\n");
        }
        outSpaceNotifyThread();
        return 1;
    }
    return 0;
}

auto FileQueueManager::measureWrite(uint64_t bytes, std::chrono::steady_clock::duration took) -> void {
    // only the time spent writing counts: the file thread also waits for chunks. limitDirtyPages
    // makes a write wait for the card, so this is the speed of the card, not of the page cache.
    m_rateBytes += bytes;
    m_rateTime += took;
    // a sample each 32 MB or each second of writing
    if (m_rateBytes < kRateSample && m_rateTime < std::chrono::seconds(1))
        return;
    const double rate = static_cast<double>(m_rateBytes) / std::chrono::duration<double>(m_rateTime).count();
    m_rateBytes = 0;
    m_rateTime = {};
    const double smooth = m_rate > 0 ? 0.7 * m_rate + 0.3 * rate : rate;
    m_rate = smooth;
    const double want = smooth * kMaxFlushSeconds;
    m_queueLimit = static_cast<uint64_t>(std::clamp(want, static_cast<double>(kMinQueueLimit), static_cast<double>(m_memoryLimit)));
}

auto FileQueueManager::outSpaceNotifyThread() -> void {
    // from the writer thread, once per write: the thread of the previous write is waited for by
    // startWrite
    if (m_spaceNotified.exchange(true))
        return;
    try {
        m_spaceThread = std::thread([this]() { outSpaceNotify(); });
    } catch (std::exception& e) {}
}

// The two sizes of the WAV header (RIFF and data) grow by what was written. They are 32 bit: past
// 4 GB they stay at 0xFFFFFFFF, which readers take as "to the end of the file" (a recording is
// split before that, CStreamingFile)
auto FileQueueManager::updateWavFile(uint32_t _size) -> void {
    const int offsets[2] = {4, 40};
    auto cur_p = fs.tellp();
    auto cur_g = fs.tellg();
    for (int offset : offsets) {
        uint32_t size = 0;
        fs.seekg(offset, fs.beg);
        fs.read(reinterpret_cast<char*>(&size), sizeof(size));
        const uint64_t grown = static_cast<uint64_t>(size) + _size;
        size = size == 0xFFFFFFFFu || grown > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<uint32_t>(grown);
        fs.seekp(offset, fs.beg);
        fs.write(reinterpret_cast<const char*>(&size), sizeof(size));
    }
    fs.seekp(cur_p);
    fs.seekg(cur_g);
}
