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
    m_IsOutOfSpace = false;
    th = nullptr;
    memset(endOfSegment, 0xFF, 12);
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

 //   acout() << "m_threadWork: " << m_threadWork << " m_useMemory: " << m_useMemory << " m_aviablePhyMemory: " << m_aviablePhyMemory << '\n';
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

        // std::time_t result = std::time(nullptr);	
        // std::fstream fs;
        // fs.open ("/tmp/debug_streaming.log", std::fstream::in | std::fstream::out | std::fstream::app);
        // char* date = std::asctime(std::localtime(&result));
        // date[strlen(date) - 1] = '\0';
        // fs << date << " : Free space on drive: " << m_freeSize << "\n";
        // fs.close();

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
    m_IsOutOfSpace = false;
    th = new std::thread(&FileQueueManager::Task,this);
}

void FileQueueManager::StopWrite(bool waitAllWrite){
    if (m_threadWork) {
        m_waitLock.lock();
        m_waitAllWrite = waitAllWrite;
        m_waitLock.unlock();
        m_ThreadRun.clear();
    }
    if (th) {
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
    
    bstream->seekg(0, bstream->end);
    auto Length = bstream->tellg();

    if (fs.good() && ((m_hasWriteSize + Length) < m_freeSize)) {
        bstream->seekg(0, bstream->beg);        
        fs << bstream->rdbuf();
        fs.flush();
//        bstream->seekg(0, std::ios::end);
//        auto Length = bstream->tellg();
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
        m_IsOutOfSpace  = true;
        m_hasErrorWrite = true;
        m_hasWriteSize += Length;
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

    uint32_t data_type = TDMS::DataType::Integer8;
    if (resolution == 16) data_type = TDMS::DataType::Integer16;
    if (resolution == 32) data_type = TDMS::DataType::SingleFloat;

    if (size_ch1 != 0)
    {       
        size_ch1 /= (resolution / 8);
        auto channel = segment.GenerateChannel("Group", "ch1");
        data.push_back(channel);
        segment.AddRaw(channel, data_type, size_ch1 , buffer_ch1);
    }

    if (size_ch2 != 0)
    {       
        size_ch2 /= (resolution / 8);
        auto channel = segment.GenerateChannel("Group", "ch2");
        data.push_back(channel);
        segment.AddRaw(channel, data_type, size_ch2 , buffer_ch2);
    }

    segment.LoadMetadata(data);
    stringstream *memory = new stringstream(ios_base::in | ios_base::out | ios_base::binary);
    outFile.WriteMemory(*memory,segment);
    return memory;
}

std::iostream *FileQueueManager::BuildBINStream(uint8_t* buffer_ch1,size_t size_ch1,uint8_t* buffer_ch2,size_t size_ch2, unsigned short resolution,uint32_t _lostSize){
    stringstream *memory = new stringstream(ios_base::in | ios_base::out | ios_base::binary);
    BinHeader header;
    header.dataFormatSize = resolution / 8;
    header.sizeCh1 = size_ch1 / header.dataFormatSize;
    header.sizeCh2 = size_ch2 / header.dataFormatSize;
    header.lostCount = _lostSize;
    header.sigmentLength = size_ch1 + size_ch2;
    //Write header
    memory->write((void*)&header,sizeof(BinHeader));
    if (size_ch1 > 0) memory->write(buffer_ch1, size_ch1);
    if (size_ch2 > 0) memory->write(buffer_ch2, size_ch2);
    //Write end segment
    memory->write(endOfSegment,12);
    return memory;
}

std::iostream *FileQueueManager::ReadCSV(std::iostream *buffer, int64_t *_position,int *_channels,bool skipData){
    uint32_t endSeg[] = { 0, 0 ,0}; 
    stringstream *memory = nullptr;
     new stringstream(ios_base::in | ios_base::out);
    buffer->seekg(*_position, std::ios::beg);
    BinHeader header;
    buffer->read((void*)&header, sizeof(BinHeader));
    buffer->seekg(*_position + sizeof(BinHeader) + header.sigmentLength, std::ios::beg);
    buffer->read((void*)endSeg , 12);
    if (endSeg[0] == 0xFFFFFFFF && endSeg[1] == 0xFFFFFFFF && endSeg[2] == 0xFFFFFFFF){
        if (!skipData){
            uint32_t size_ch1 = header.sizeCh1;
            uint32_t size_ch2 = header.sizeCh2;
            if (*_channels == 0) {
                *_channels = (size_ch1 > 0 ? 1 : 0) + (size_ch2 > 0 ? 1 :0);
            } 
            uint32_t lost     = header.lostCount;
//            acout() << "* " << size_ch1 << " - " << size_ch2 << " $ " << lost << "\n";
            if (size_ch1 || size_ch2 || lost) {
                memory = new stringstream(ios_base::in | ios_base::out);
            }
            char resolution = header.dataFormatSize * 8;
            char *buffer_ch1 = nullptr;
            char *buffer_ch2 = nullptr;
            buffer->seekg(*_position + sizeof(BinHeader), std::ios::beg);
            if (size_ch1 > 0) {
                buffer_ch1 = new char[size_ch1 * header.dataFormatSize];
                buffer->read(buffer_ch1,size_ch1 * header.dataFormatSize);
            }
            if (size_ch2 > 0) {
                buffer_ch2 = new char[size_ch2 * header.dataFormatSize];
                buffer->read(buffer_ch2,size_ch2 * header.dataFormatSize);
            }

            if (size_ch1 != 0 && size_ch2 == 0)
            {       
                if (resolution == 8) { 
                    for(auto ix = 0u ; ix < size_ch1 ; ix++){
                        *memory << (int)((int8_t*)buffer_ch1)[ix] << "\n";
                    }
                }

                if (resolution == 16) { 
                    for(auto ix = 0u ; ix < size_ch1 ; ix++){
                        *memory << ((int16_t*)buffer_ch1)[ix] << "\n";
                    }
                }

                if (resolution == 32) { 
                    for(auto ix = 0u ; ix < size_ch1 ; ix++){
                        *memory << ((float*)buffer_ch1)[ix] << "\n";
                    }
                }
                
            }

            if (size_ch1 == 0 && size_ch2 != 0)
            {       
                if (resolution == 8) { 
                    for(auto ix = 0u ; ix < size_ch2 ; ix++){
                        *memory << (int)((int8_t*)buffer_ch2)[ix] << "\n";
                    }
                }

                if (resolution == 16) { 
                    for(auto ix = 0u ; ix < size_ch2 ; ix++){
                        *memory << ((int16_t*)buffer_ch2)[ix] << "\n";
                    }
                }

                if (resolution == 32) { 
                    for(auto ix = 0u ; ix < size_ch2 ; ix++){
                        *memory << ((float*)buffer_ch2)[ix] << "\n";
                    }
                }
            }

            if (size_ch1 != 0 && size_ch2 != 0)
            {       
                if (resolution == 8) { 
                    for(auto ix = 0u ; ix < size_ch1 ; ix++){
                        *memory << (int)((int8_t*)buffer_ch1)[ix] << "," << (int)((int8_t*)buffer_ch2)[ix] << "\n";
                    }
                }

                if (resolution == 16) { 
                    for(auto ix = 0u ; ix < size_ch1 ; ix++){
                        *memory << ((int16_t*)buffer_ch1)[ix] << "," << ((int16_t*)buffer_ch2)[ix] << "\n";
                    }
                }

                if (resolution == 32) { 
                    for(auto ix = 0u ; ix < size_ch1 ; ix++){
                        *memory << ((float*)buffer_ch1)[ix] << "," << ((float*)buffer_ch2)[ix] << "\n";
                    }
                }
            }
            if (buffer_ch1 != nullptr) delete[] buffer_ch1;
            if (buffer_ch2 != nullptr) delete[] buffer_ch2;
            if (lost > 0) {
                std::string s = "";
                if (*_channels == 2) s = "0,0\n";
                if (*_channels == 1) s = "0\n";
                for(int i = 0 ; i < lost ; ++i){
                    *memory << s;
                }
            }
        }
        buffer->seekg(0, std::ios::end);
        auto Length = buffer->tellg();
//        acout() << "\nLen " << Length <<  " Pos " << *_position <<"\n";
        *_position = *_position + sizeof(BinHeader) + header.sigmentLength + 12;
        if (*_position >= Length) {
            *_position = -2;
        }
    }else{
        *_position = -1;
    }
    return memory;
}

BinInfo FileQueueManager::ReadBinInfo(std::iostream *buffer){
    int64_t position = 0;
    buffer->seekg(0, std::ios::end);
    auto Length = buffer->tellg();
    BinInfo bi;
    while(position >= 0){
        uint32_t endSeg[] = { 0, 0 ,0}; 
        buffer->seekg(position, std::ios::beg);
        BinHeader header;
        buffer->read((void*)&header, sizeof(BinHeader));
        buffer->seekg(position + sizeof(BinHeader) + header.sigmentLength, std::ios::beg);
        buffer->read((void*)endSeg , 12);
        bi.dataFormatSize = header.dataFormatSize;
        bi.size_ch1  += header.sizeCh1;
        bi.size_ch2  += header.sizeCh2;
        bi.lostCount += header.lostCount;
        if (bi.segSamplesCount == 0) bi.segSamplesCount = header.sizeCh1 > header.sizeCh2 ? header.sizeCh1 : header.sizeCh2;
        bi.segLastSamplesCount = header.sizeCh1 > header.sizeCh2 ? header.sizeCh1 : header.sizeCh2;
        bi.segCount++;
        if (endSeg[0] == 0xFFFFFFFF && endSeg[1] == 0xFFFFFFFF && endSeg[2] == 0xFFFFFFFF){
            position =  position + sizeof(BinHeader) + header.sigmentLength + 12;
            if (position >= Length) {
                position = -2;
            }
        }else{
            position = -1;
        }
    }
    bi.lastSegState = position == -2;
    return bi;
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



