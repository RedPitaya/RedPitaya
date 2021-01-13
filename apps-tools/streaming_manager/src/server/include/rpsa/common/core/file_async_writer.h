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
#include "types.h"


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
    long queueSize();
protected:
    Queue();
    ~Queue();
    void pushQueue(std::iostream* buffer);
    std::iostream* popQueue();
    uint64_t m_useMemory;    
private:
    std::list<std::iostream*> m_queue;
    std::mutex mutex_;    
};


class FileQueueManager:public Queue{
    std::fstream fs;
    std::thread *th;
    std::atomic_flag m_ThreadRun = ATOMIC_FLAG_INIT;
    bool m_threadWork;
    int  m_waitAllWrite;
    std::mutex       m_waitLock;
    bool m_hasErrorWrite;
    Stream_FileType  m_fileType; // FLAG for file type TDMS/Wav
    bool m_firstSectionWrite; // Need for detect first section of wav file
    bool m_IsOutOfSpace;
    void Task();
   ulong m_freeSize;
   ulong m_hasWriteSize;   
    uint64_t m_aviablePhyMemory; 
public:
    FileQueueManager();
    ~FileQueueManager();
    static ulong GetFreeSpaceDisk(std::string _filePath);
    void StartWrite(Stream_FileType _fileType);
    void StopWrite(bool waitAllWrite);
    bool IsWork() { return  m_threadWork && !m_hasErrorWrite;}
    bool IsOutOfSpace() {return m_IsOutOfSpace; }
    int  WriteToFile();
    bool AddBufferToWrite(std::iostream *buffer);
    void OpenFile(std::string FileName,bool append);
    void CloseFile();
static int  AvailableSpace(std::string dst, ulong* availableSize);
    std::iostream *BuildTDMSStream(uint8_t* buffer_ch1,size_t size_ch1,uint8_t* buffer_ch2,size_t size_ch2,unsigned short resolution);
    std::iostream *BuildBINStream (uint8_t* buffer_ch1,size_t size_ch1,uint8_t* buffer_ch2,size_t size_ch2, unsigned short resolution,uint32_t _lostSize);
    static std::iostream *ReadCSV(std::iostream *buffer,int64_t *_position,int *_channels,bool skipData=false);
    static BinInfo        ReadBinInfo(std::iostream *buffer);
    void updateWavFile(int _size);
private:
    char endOfSegment[12];
};
