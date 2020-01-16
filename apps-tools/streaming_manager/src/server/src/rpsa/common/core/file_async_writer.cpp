//
// Created by user on 22.11.18.
//

//
// Created by user on 11.10.18.
//

#include "rpsa/common/core/file_async_writer.h"
#include "rpsa/common/core/File.h"
#include <ctime>

#ifndef _WIN32
#include <sys/statvfs.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

std::string DirNameOf(const std::string& fname)
{
    size_t pos = fname.find_last_of("\\/");
    return (std::string::npos == pos)
           ? "."
           : fname.substr(0, pos);
}

FileQueueManager::FileQueueManager():Queue(){
    m_threadWork = false;
    m_waitAllWrite = false;    
    m_hasErrorWrite = false;
}

FileQueueManager::~FileQueueManager(){
    this->StopWrite(false);
}

unsigned long long getTotalSystemMemory()
{
#ifndef _WIN32
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
#else
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
#endif
}

int FileQueueManager::AvailableSpace(std::string dst, ulong* availableSize) {
#ifdef _WIN32
	*availableSize = UINT32_MAX;
	return 0;
#else
    int result = -1;
    try {
        struct statvfs devData;
        memset(&devData, 0, sizeof (struct statvfs));
        if ((statvfs(dst.c_str(), &devData)) >= 0) {
            if (availableSize != NULL) {
                //I don't know if it's right, but I'll set availableSize only if the available size doesn't pass the ulong limit.

                if (devData.f_bavail  > (std::numeric_limits<ulong>::max() / devData.f_bsize)) {
                    *availableSize = std::numeric_limits<ulong>::max();
                } else {
                    *availableSize = devData.f_bavail * devData.f_bsize;
                }
            }
            result = 0;
        }
    }
    catch (std::exception& e)
    {
        std::cout <<  "Error in AvailableSpace(): " << e.what() << '\n';
    }
    return result;
#endif
}

bool FileQueueManager::AddBufferToWrite(std::iostream *buffer){

 //   acout() << m_useMemory  << "\n";
    if (m_threadWork && (m_useMemory < m_aviablePhyMemory)){
        pushQueue(buffer);
        return true;
    }
    else{
        delete buffer;
        return false;
    }
}

ulong FileQueueManager::GetFreeSpaceDisk(std::string _filePath){

    ulong m_freeSize = 0;
    if (FileQueueManager::AvailableSpace(_filePath, &m_freeSize) == 0){

        std::time_t result = std::time(nullptr);	
        std::fstream fs;
        fs.open ("/tmp/debug_streaming.log", std::fstream::in | std::fstream::out | std::fstream::app);
        char* date = std::asctime(std::localtime(&result));
        date[strlen(date) - 1] = '\0';
        fs << date << " : Free space on drive: " << m_freeSize << "\n";
        fs.close();

        if (m_freeSize < USING_FREE_SPACE){
            m_freeSize = 0;
        }else{
            m_freeSize = m_freeSize - USING_FREE_SPACE;
        }

    }else{
        m_freeSize = 0;
    }
    return  m_freeSize;
}

void FileQueueManager::OpenFile(std::string FileName,bool Append){
    fs.open(FileName, std::ios::binary | std::ofstream::out| std::ofstream::in | (Append? std::ofstream::binary  : std::ofstream::trunc));
    if (fs.fail()) {
        fs.open(FileName, std::ios::binary | std::ofstream::out| std::ofstream::in |  std::ofstream::trunc);
        if (fs.fail()) {
            std::cout << "File " << FileName << " not exist" << std::endl;
            return;
        }
    }

    auto dirName = DirNameOf(FileName);
    if (dirName == "") {
        dirName = ".";
    }

    m_freeSize = GetFreeSpaceDisk(dirName);
    m_aviablePhyMemory = getTotalSystemMemory();
    std::cout << "Available physical memory: " << m_aviablePhyMemory / (1024 * 1024) << "Mb\n";
    m_aviablePhyMemory /= 2;
    std::cout << "Used physical memory: " << m_aviablePhyMemory / (1024 * 1024) << "Mb\n";
    m_hasWriteSize = 0;
}

void FileQueueManager::CloseFile(){
    if (fs.is_open())
        fs.close();
}

void FileQueueManager::StartWrite(Stream_FileType _fileType){
    m_ThreadRun.test_and_set();
    m_threadWork = true;
    m_fileType = _fileType;
    m_firstSectionWrite = false;
    m_waitAllWrite = true;
    m_hasErrorWrite = false;
    th = new std::thread(&FileQueueManager::Task,this);
}

void FileQueueManager::StopWrite(bool waitAllWrite){
    if (m_threadWork) {
        m_waitLock.lock();
        m_waitAllWrite = waitAllWrite;
        m_waitLock.unlock();
        m_ThreadRun.clear();
    }
    if (th != nullptr) {
        if (th->joinable())
            th->join();
        delete th;
        th = nullptr;
    }
}

void FileQueueManager::Task(){

    while (m_ThreadRun.test_and_set()){
        WriteToFile();        
    }
    m_waitLock.lock();
    if (this->m_waitAllWrite) {
        while (WriteToFile() == 0);
    }else{
        auto bstream = popQueue();
        while(bstream){
            delete bstream;
            bstream = popQueue();
        }
    }
    m_threadWork = false;
    m_waitLock.unlock();
}


int FileQueueManager::WriteToFile(){
    auto bstream = popQueue();
        
    if (bstream == nullptr)
        return -1;

    if (m_hasErrorWrite) {
        delete bstream;
        return 1;
    }

    if (fs.good() && m_hasWriteSize < m_freeSize) {
        
        fs << bstream->rdbuf();
        fs.flush();
        bstream->seekg(0, std::ios::end);
        auto Length = bstream->tellg();
        m_hasWriteSize += Length;

        if (m_fileType == Stream_FileType::WAV_TYPE){
            if (m_firstSectionWrite){
                updateWavFile(Length);
            }

            if (m_firstSectionWrite == false){
                m_firstSectionWrite = true;
            }
        }

    } else{

        m_hasErrorWrite = true;
        if (!(m_hasWriteSize < m_freeSize)){
            acout() << "The disc has reached the write limit\n";
        }else {
            acout() << "Disk is full or error state\n";
        }
        return 1;
    }
    delete bstream;

    return 0;    
}

void FileQueueManager::updateWavFile(int _size){
    int offset1 = 4;
    int offset2 = 40;
   
    auto cur_p = fs.tellp();
    auto cur_g = fs.tellg();

    int32_t size1 = 0;
    int32_t size2 = 0;
    fs.seekg(offset1, fs.beg);
    fs.read ((char*)&size1, sizeof(size1));
    size1 += _size;
    fs.seekp(offset1, fs.beg);
    fs.write((char*)&size1, sizeof(size1));
    fs.seekg(offset2, fs.beg);
    fs.read ((char*)&size2, sizeof(size2));
    size2 += _size;
    fs.seekp(offset2, fs.beg);
    fs.write((char*)&size2, sizeof(size2));

 
    fs.seekp(cur_p);
    fs.seekg(cur_g);
}

std::iostream *FileQueueManager::BuildTDMSStream(uint8_t* buffer_ch1,size_t size_ch1,uint8_t* buffer_ch2,size_t size_ch2, unsigned short resolution){
    TDMS::File outFile;
    TDMS::WriterSegment segment;
    vector<shared_ptr<TDMS::Metadata>> data;
//    std::time_t tim_sec = std::time(0);
 //   std::time_t local = std::mktime(std::localtime(&tim_sec));
 //   std::time_t gmt = std::mktime(std::gmtime(&tim_sec));
 //   auto timezone = static_cast<long> (local - gmt);

    auto root = segment.GenerateRoot();
    root->TableOfContents.HasMetaData = true;
    root->TableOfContents.HasRawData = true;
    data.push_back(root);
    auto group = segment.GenerateGroup("Group");
    data.push_back(group);

//    auto *time = TDMS::DataType::GetRawTimeValue(tim_sec + timezone);
//    TDMS::DataType dataprop;
//    dataprop.InitDataType(TDMS::DataType::TimeStamp,time);
//    segment.AddProperties(root,"time_stamp_now",dataprop);


    if (size_ch1 != 0)
    {       
        if (resolution == 16)
            size_ch1 /= 2;
        auto channel = segment.GenerateChannel("Group", "ch1");
        data.push_back(channel);
        segment.AddRaw(channel, (resolution == 8 ? TDMS::DataType::Integer8 : TDMS::DataType::Integer16), size_ch1 , buffer_ch1);
    }

    if (size_ch2 != 0)
    {       
        if (resolution == 16)
            size_ch2 /= 2;
        auto channel = segment.GenerateChannel("Group", "ch2");
        data.push_back(channel);
        segment.AddRaw(channel, (resolution == 8 ? TDMS::DataType::Integer8 : TDMS::DataType::Integer16), size_ch2 , buffer_ch2);
    }

    segment.LoadMetadata(data);
    stringstream *memory = new stringstream(ios_base::in | ios_base::out | ios_base::binary);
    outFile.WriteMemory(*memory,segment);
    return memory;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Queue::Queue():
m_useMemory(0)
{

}

Queue::~Queue(){

}




void Queue::pushQueue(std::iostream* buffer){
    mutex_.lock();
    m_queue.push_back(buffer);
    buffer->seekg(0, std::ios::end);
    auto Length = buffer->tellg();
    m_useMemory += Length;
    mutex_.unlock();
}



std::iostream* Queue::popQueue(){
    mutex_.lock();
    std::iostream* buffer = m_queue.front();
    if (buffer != nullptr){
        m_queue.pop_front();
        buffer->seekg(0, std::ios::end);
        auto Length = buffer->tellg();
        m_useMemory -= Length;
        buffer->seekg(0, std::ios::beg);
    }
    mutex_.unlock();

    return buffer;
}

long Queue::queueSize(){
    mutex_.lock();
    long size = m_queue.size();
    mutex_.unlock();
    return size;
}



