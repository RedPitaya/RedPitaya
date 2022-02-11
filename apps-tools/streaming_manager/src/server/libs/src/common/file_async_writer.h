#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <list>
#include <asio.hpp>
#include <fstream>
#include <iostream>
#include "thread_cout.h"

#define USING_FREE_SPACE 1024 * 1024 * 30 // Left free on disk 30 Mb

enum Stream_FileType{
    TDMS_TYPE,
    WAV_TYPE,
    CSV_TYPE
};

struct BinHeader{
    char dataFormatSize;
    uint32_t sizeCh1;
    uint32_t sizeCh2;
    uint32_t lostCount;
    uint32_t sigmentLength;
};

struct BinInfo{
    char dataFormatSize;
    uint32_t size_ch1;
    uint32_t size_ch2;
    uint32_t segSamplesCount;
    uint32_t segLastSamplesCount;
    uint32_t segCount;
    bool     lastSegState;
    uint64_t lostCount;
    BinInfo(){
        dataFormatSize = 0;
        size_ch1 = 0;
        size_ch2 = 0;
        segSamplesCount = 0;
        segCount = 0;
        lastSegState = false;
        lostCount = 0;
        segLastSamplesCount = 0;
    }
};

class Queue
{
    public:
        auto queueSize() -> long;

    protected:
        Queue();
        ~Queue();

        auto pushQueue(std::iostream* buffer) -> void;
        auto popQueue() -> std::iostream*;

        uint64_t m_useMemory;

    private:
        std::list<std::iostream*> m_queue;
        std::mutex m_mutex;
};

class FileQueueManager:public Queue{
    public:
        FileQueueManager(bool testMode = false);
        ~FileQueueManager();

        auto AddBufferToWrite(std::iostream *buffer) -> bool;
        auto BuildTDMSStream(uint8_t* buffer_ch1,size_t size_ch1,uint8_t* buffer_ch2,size_t size_ch2,unsigned short resolution) -> std::iostream *;
        auto BuildBINStream (uint8_t* buffer_ch1,size_t size_ch1,uint8_t* buffer_ch2,size_t size_ch2, unsigned short resolution,uint32_t _lostSize) -> std::iostream *;
        auto CloseFile() -> void;
        auto IsWork() -> bool { return  m_threadWork && !m_hasErrorWrite;}
        auto IsOutOfSpace() -> bool {return m_IsOutOfSpace; }
        auto OpenFile(std::string FileName,bool append) -> void;
        auto StartWrite(Stream_FileType _fileType) -> void;
        auto StopWrite(bool waitAllWrite) -> void;
        auto UpdateWavFile(int _size) -> void;
        auto WriteToFile() -> int;
        auto deleteFile() -> void;

        static auto AvailableSpace(std::string dst, unsigned long long* availableSize) -> int;
        static auto GetFreeSpaceDisk(std::string _filePath) -> unsigned long long;

        static auto ReadBinInfo(std::iostream *buffer) -> BinInfo;
        static auto ReadCSV(std::iostream *buffer,int64_t *_position,int *_channels,bool skipData = false) -> std::iostream *;

    private:

        auto Task() -> void;

        char endOfSegment[12];
        std::fstream fs;
        std::thread *th;
        std::atomic_flag m_ThreadRun = ATOMIC_FLAG_INIT;
        bool m_threadWork;
        int  m_waitAllWrite;
        std::mutex       m_waitLock;
        std::mutex       m_threadControl;
        bool m_hasErrorWrite;
        Stream_FileType  m_fileType; // FLAG for file type TDMS/Wav
        bool m_firstSectionWrite; // Need for detect first section of wav file
        bool m_IsOutOfSpace;
        unsigned long long m_freeSize;
        unsigned long long m_hasWriteSize;
        uint64_t m_aviablePhyMemory;
        bool m_testMode;
        std::string m_fileName;
};
