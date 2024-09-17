#ifndef WRITER_LIB_FILEQUEUEMANAGER_H
#define WRITER_LIB_FILEQUEUEMANAGER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include "w_queue.h"
#include "data_lib/signal.hpp"
#include "settings_lib/stream_settings.h"

class FileQueueManager:public Queue{
    public:

        FileQueueManager(bool testMode = false);
        ~FileQueueManager();

        auto addBufferToWrite(std::iostream *buffer) -> bool;
        auto closeFile() -> void;
        auto isWork() -> bool { return  m_threadWork && !m_hasErrorWrite;}
        auto isOutOfSpace() -> bool {return m_IsOutOfSpace; }
        auto openFile(std::string FileName,bool append) -> void;
        auto startWrite(CStreamSettings::DataFormat _fileType) -> void;
        auto stopWrite(bool waitAllWrite) -> void;
        auto updateWavFile(int _size) -> void;
        auto writeToFile() -> int;
        auto deleteFile() -> void;
        auto getWritedSize() -> uint64_t;

        sigslot::signal<> outSpaceNotify;
        sigslot::signal<> stopNotify;

    private:

        auto task() -> void;
        auto outSpaceNotifyThread() -> void;

        std::fstream fs;
        std::thread *th;
        std::atomic_bool m_ThreadRun;
        bool m_threadWork;
        int  m_waitAllWrite;
        std::mutex       m_waitLock;
        std::mutex       m_threadControl;
        bool m_hasErrorWrite;
        CStreamSettings::DataFormat m_fileType = CStreamSettings::DataFormat::BIN;
        bool m_firstSectionWrite; // Need for detect first section of wav file
        bool m_IsOutOfSpace;
        uint64_t m_freeSize;
        uint64_t m_hasWriteSize;
        uint64_t m_aviablePhyMemory;
        bool m_testMode;
        std::string m_fileName;
};

#endif
