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
    void updateWavFile(int _size);
};
