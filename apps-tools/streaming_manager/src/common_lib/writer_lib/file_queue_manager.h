#ifndef WRITER_LIB_FILEQUEUEMANAGER_H
#define WRITER_LIB_FILEQUEUEMANAGER_H

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>
#include <vector>
#include "data_lib/signal.hpp"
#include "settings_lib/stream_settings.h"
#include "w_queue.h"

class FileQueueManager : public Queue {
   public:
    FileQueueManager(bool testMode = false);
    ~FileQueueManager();

    // Queues a chunk for the file thread; false (the chunk dropped) when the thread does not run
    // or the queue is full (queueLimit)
    auto addChunkToWrite(FileChunk::Ptr chunk) -> bool;
    auto closeFile() -> void;
    auto isWork() -> bool { return m_threadWork && !m_hasErrorWrite; }
    auto isOutOfSpace() -> bool { return m_IsOutOfSpace; }
    auto openFile(std::string FileName, bool append) -> void;
    auto startWrite(CStreamSettings::DataFormat _fileType) -> void;
    auto stopWrite(bool waitAllWrite) -> void;
	auto updateWavFile(uint32_t _size) -> void;
    // Writes the next chunk, waiting up to `wait` for one: 0 written, -1 none, 1 not written (error)
    auto writeToFile(std::chrono::milliseconds wait = std::chrono::milliseconds(0)) -> int;
    auto deleteFile() -> void;
    auto getWritedSize() -> uint64_t;
    // what the queue holds of the memory it may take, 0..1
    auto queueFill() -> double;
    // The memory the queue may take: what the file takes to write in kMaxFlushSeconds at the
    // speed measured so far (a stop writes the queue out before the server exits, and systemd
    // waits 30 s for that), at most a quarter of the RAM
    auto queueLimit() -> uint64_t { return m_queueLimit; }
    // the measured write speed of the file, bytes per second; 0 before the first second of writing
    auto writeRate() -> double { return m_rate; }

    sigslot::signal<> outSpaceNotify;
    sigslot::signal<> stopNotify;

   private:
    auto task() -> void;
    auto outSpaceNotifyThread() -> void;
    // Waits for the thread of the out-of-space notify, unless called on it.
    auto joinSpaceThread() -> void;
    // Keeps the page cache of the file small (Linux): starts the write-back of each new window of
    // the file, waits for the window before it and drops it from the cache. Without it the kernel
    // collects hundreds of MB of dirty pages at a high rate; they squeeze the cache of every other
    // program out (systemd misses its watchdog) and make the close of the file take tens of
    // seconds on an SD card.
    auto limitDirtyPages(bool all) -> void;
    // Adds a write to the speed measurement and sets queueLimit from it
    auto measureWrite(uint64_t bytes, std::chrono::steady_clock::duration took) -> void;
    auto closeSyncFd() -> void;

    std::fstream fs;
    std::thread* th;
    // The out-of-space notify runs on a thread of its own (its handler stops the writer, which
    // cannot join itself): once per write, waited for by the next start and the destructor, so
    // it never runs on a freed object.
    std::thread m_spaceThread;
    std::atomic<bool> m_spaceNotified{false};
    std::atomic_bool m_ThreadRun;
    std::atomic_bool m_threadWork;
    int m_waitAllWrite;
    std::mutex m_waitLock;
    std::mutex m_threadControl;
    std::atomic_bool m_hasErrorWrite;
    CStreamSettings::DataFormat m_fileType = CStreamSettings::DataFormat::BIN;
    bool m_firstSectionWrite;  // Need for detect first section of wav file
    std::atomic_bool m_IsOutOfSpace;
    uint64_t m_freeSize;
    std::atomic<uint64_t> m_hasWriteSize;
    uint64_t m_memoryLimit = 0;  // a quarter of the RAM
    std::atomic<uint64_t> m_queueLimit{0};
    std::atomic<double> m_rate{0};
    uint64_t m_rateBytes = 0;
    std::chrono::steady_clock::duration m_rateTime{};
    bool m_testMode;
    std::string m_fileName;
    int m_syncFd = -1;           // the file once more, for sync_file_range / posix_fadvise
    uint64_t m_syncStarted = 0;  // the write-back runs up to here
    uint64_t m_syncDone = 0;     // written and dropped from the cache up to here
};

#endif
