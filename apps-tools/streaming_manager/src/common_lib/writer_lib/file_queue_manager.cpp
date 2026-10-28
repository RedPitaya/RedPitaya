#include "file_queue_manager.h"
#include <ctime>
#include "file_helper.h"
#include "logger_lib/file_logger.h"

namespace {
// How long the file thread sleeps on an empty queue before it looks at the stop flag again
constexpr std::chrono::milliseconds kWaitForChunk(100);
}  // namespace

FileQueueManager::FileQueueManager(bool testMode) : Queue() {
    m_threadWork = false;
    m_waitAllWrite = false;
    m_hasErrorWrite = false;
    m_IsOutOfSpace = false;
    th = nullptr;
    m_freeSize = 0;
    m_aviablePhyMemory = 0;
    m_testMode = testMode;
    m_fileName = "";
    m_hasWriteSize = 0;
}

FileQueueManager::~FileQueueManager() {
    this->stopWrite(false);
}

auto FileQueueManager::deleteFile() -> void {
    try {
        std::remove(m_fileName.c_str());
    } catch (std::exception& e) {
        aprintf(stderr, "Error delete file: %s err: %s \n", m_fileName.c_str(), e.what());
    }
}

auto FileQueueManager::addChunkToWrite(FileChunk::Ptr chunk) -> bool {
    if (!chunk || !m_threadWork)
        return false;
    return pushQueue(std::move(chunk), m_aviablePhyMemory);
}

auto FileQueueManager::openFile(std::string FileName, bool Append) -> void {
    fs.open(FileName, std::ios::binary | std::ofstream::out | std::ofstream::in | (Append ? std::ofstream::binary : std::ofstream::trunc));
    if (fs.fail()) {
        fs.open(FileName, std::ios::binary | std::ofstream::out | std::ofstream::in | std::ofstream::trunc);
        if (fs.fail()) {
            aprintf(stderr, "File: %s  not exist\n", FileName.c_str());
            return;
        }
    }
    m_fileName = FileName;
    auto dirName = dirNameOf(FileName);
    if (dirName == "") {
        dirName = ".";
    }

    m_freeSize = getFreeSpaceDisk(dirName);
    if (m_testMode) {
        m_freeSize = std::numeric_limits<unsigned long long>::max();
    }
    m_aviablePhyMemory = getTotalSystemMemory();
    aprintf(stdout, "Available physical memory: %d Mb\n", m_aviablePhyMemory / (1024 * 1024));
    m_aviablePhyMemory /= 2;
    aprintf(stdout, "Used physical memory: %d Mb\n", m_aviablePhyMemory / (1024 * 1024));
    m_hasWriteSize = 0;
}

auto FileQueueManager::closeFile() -> void {
    if (fs.is_open())
        fs.close();
}

auto FileQueueManager::startWrite(CStreamSettings::DataFormat _fileType) -> void {
    m_ThreadRun = true;
    m_threadWork = true;
    m_fileType = _fileType;
    m_firstSectionWrite = false;
    m_waitAllWrite = true;
    m_hasErrorWrite = false;
    m_IsOutOfSpace = false;
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
    return m_hasWriteSize;
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

    const uint64_t Length = chunk->size();

    if (fs.good() && ((m_hasWriteSize + Length) < m_freeSize)) {
        if (m_testMode) {
            fs.seekp(0);
        }
        chunk->writeTo(fs);
        fs.flush();
        m_hasWriteSize += Length;

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

auto FileQueueManager::outSpaceNotifyThread() -> void {
    try {
        std::thread th([this]() { outSpaceNotify(); });
        th.detach();
    } catch (std::exception& e) {}
}

auto FileQueueManager::updateWavFile(uint32_t _size) -> void
{
	int offset1 = 4;
    int offset2 = 40;

    auto cur_p = fs.tellp();
    auto cur_g = fs.tellg();

	uint32_t size1 = 0;
	uint32_t size2 = 0;
	fs.seekg(offset1, fs.beg);
	fs.read((char*)&size1, sizeof(size1));
    size1 += _size;
    fs.seekp(offset1, fs.beg);
    fs.write((char*)&size1, sizeof(size1));
    fs.seekg(offset2, fs.beg);
    fs.read((char*)&size2, sizeof(size2));
    size2 += _size;
    fs.seekp(offset2, fs.beg);
    fs.write((char*)&size2, sizeof(size2));

    fs.seekp(cur_p);
    fs.seekg(cur_g);
}
