#include <chrono>
#include <iostream>
#include <fstream>
#include <time.h>
#include <functional>
#include <cstdlib>
#include "rpsa/server/core/StreamingManager.h"

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif

bool createDir(const std::string dir)
{
#ifdef _WIN32
    mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0777);
#endif

    return true;
}

bool createDirTree(const std::string full_path)
{
    char ch = '/';
#ifdef _WIN32
    ch = '\\';
#endif

    size_t pos = 0;
    bool ret_val = true;

    while(ret_val == true && pos != std::string::npos)
    {
        pos = full_path.find(ch, pos + 1);
        ret_val = createDir(full_path.substr(0, pos));
    }

    return ret_val;
}

std::string getNewFileName(Stream_FileType _fileType,string _filePath)
{
    createDirTree(_filePath);

    char time_str[40];
    struct tm *timenow;

    time_t now = time(nullptr);
    timenow = gmtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);
    std::string filename = _filePath  + "/" + std::string("data_file_") + time_str+"." + (_fileType == Stream_FileType::TDMS_TYPE ? "tdms":"wav");
    return filename;
}

void CStreamingManager::MakeEmptyDir(std::string _filePath){
    createDirTree(_filePath);
}



CStreamingManager::Ptr CStreamingManager::Create(Stream_FileType _fileType,std::string _filePath, int _samples){

    return std::make_shared<CStreamingManager>(_fileType, _filePath,_samples);
}

CStreamingManager::CStreamingManager(Stream_FileType _fileType,std::string _filePath, int _samples) :
    m_use_local_file(true),
    notifyPassData(nullptr),
    m_file_manager(nullptr),
    m_fileType(_fileType),
    m_asionet(nullptr),
    m_filePath(_filePath),
    m_index_of_message(0),
    notifyStop(nullptr),
    m_samples(_samples)
{
    
    if (m_use_local_file){
        
        m_file_manager = new FileQueueManager();
        m_waveWriter = new CWaveWriter();
        memset(m_zeroBuffer,0,sizeof(uint8_t) * ZERO_BUFFER_SIZE);
    }
}

CStreamingManager::Ptr CStreamingManager::Create(string _host, string _port, asionet::Protocol _protocol){
    return std::make_shared<CStreamingManager>(_host,_port,_protocol);
}

CStreamingManager::CStreamingManager(string _host, string _port, asionet::Protocol _protocol):
        m_use_local_file(false),
        notifyPassData(nullptr),
        m_file_manager(nullptr),
        m_waveWriter(nullptr),
        m_host(_host),
        m_port(_port),
        m_protocol(_protocol),
        m_asionet(nullptr),
        m_filePath(""),
        m_index_of_message(0),
        notifyStop(nullptr),
        m_samples(0)
{

}

CStreamingManager::~CStreamingManager()
{
    this->stop();
    if (m_file_manager!= nullptr){
        delete m_file_manager;
        m_file_manager = nullptr;
    }

    if (m_waveWriter!=nullptr){
        delete m_waveWriter;
        m_waveWriter = nullptr;
    }

    if (m_asionet){
        delete m_asionet;
        m_asionet = nullptr;
    }

}

void CStreamingManager::startServer(){
    if (m_asionet){
        delete m_asionet;
        m_asionet = nullptr;
    }
    m_index_of_message = 0;
    m_SendData = 0 ;
    m_ReadyToPass = 0;
    m_asionet = new asionet::CAsioNet(asionet::Mode::SERVER, m_protocol, m_host, m_port);
    m_asionet->addCallServer_Connect([](std::string host)
                                     {
                                         std::cout << "Connected " << host << '\n';
                                     });
    m_asionet->addCallServer_Disconnect([this](std::string host)
                                     {
                                         std::cout << "Disconnect " << host << '\n';

                                     });
    m_asionet->addCallSend([this](std::error_code error,size_t size)
                            {
                                m_ReadyToPass--;
                                m_SendData += size;
                                if (m_ReadyToPass == 0) {
                                    if (this->notifyPassData)
                                        this->notifyPassData(static_cast<int>(m_SendData));
                                    m_SendData = 0;
                                }
                            });
    m_asionet->Start();
}

void CStreamingManager::stopServer(){

    if (m_asionet) {
        m_asionet->Stop();
        delete m_asionet;
        m_asionet = nullptr;
    }
}

bool CStreamingManager::isFileThreadWork(){
    if (m_use_local_file) {
        if (m_file_manager != nullptr) {
            return m_file_manager->IsWork();
        }
    }
    return false;
}

bool CStreamingManager::isOutOfSpace(){
     if (m_use_local_file) {
        if (m_file_manager != nullptr) {
            return m_file_manager->IsOutOfSpace();
        }
    }
    return false;  
}

void CStreamingManager::run()
{
    if (m_use_local_file){
        m_passSizeSamples = 0;
        m_file_out = getNewFileName(m_fileType, m_filePath);      
        m_fileLogger = CFileLogger::Create(m_file_out + ".log"); 
        std::cout << m_file_out << "\n"; 
        m_file_manager->OpenFile(m_file_out, false);
        m_file_manager->StartWrite(m_fileType);
    }
    else
        this->startServer();
}

void CStreamingManager::stop(){
    if (m_use_local_file){
        if (m_file_manager != nullptr) {
            m_file_manager->StopWrite(false);
        }
    } else{
        this->stopServer();
    }
}


int CStreamingManager::passBuffers(uint64_t _lostRate, uint32_t _oscRate, const void *_buffer_ch1, uint32_t _size_ch1,const void *_buffer_ch2, uint32_t _size_ch2, unsigned short _resolution, uint64_t _id){

    ASIO_ASSERT(!(_size_ch1 != _size_ch2 && _size_ch1 != 0 && _size_ch2 != 0));
    uint8_t *buff_ch1 = nullptr;
    uint8_t *buff_ch2 = nullptr;

    if (m_use_local_file){
        bool flag = false;
        if (m_samples != -1) {
            int devider = (_resolution == 16 ? 2 : 1);
            m_passSizeSamples += (_size_ch1 > 0 ? _size_ch1  : _size_ch2) / devider;
            if (m_passSizeSamples >= m_samples) {
                flag = true;
                int diff = (m_passSizeSamples - m_samples) * devider;
                if (_size_ch1 > 0) {
                    if (diff > _size_ch1)
                        _size_ch1 = 0;
                    else
                        _size_ch1 -= diff;
                }
                if (_size_ch2 > 0) {
                     if (diff > _size_ch2)
                        _size_ch2 = 0;
                    else
                        _size_ch2 -= diff;
                }
            }
        }


        if (_size_ch1 + _size_ch2 > 0){
            
            if (m_fileType == TDMS_TYPE){
                if (_size_ch1>0){ 
                    buff_ch1 = new uint8_t[_size_ch1];
                    memcpy_neon(buff_ch1, _buffer_ch1, _size_ch1);
                }

                if (_size_ch2>0){ 
                    buff_ch2 = new uint8_t[_size_ch2];
                    memcpy_neon(buff_ch2, _buffer_ch2, _size_ch2);
                }

                auto stream_data = m_file_manager->BuildTDMSStream(buff_ch1, _size_ch1, buff_ch2, _size_ch2,_resolution);
                if (!m_file_manager->AddBufferToWrite(stream_data))
                {
                    m_fileLogger->AddMetric(CFileLogger::Metric::FILESYSTEM_RATE,1);
                }
            }


            if (m_fileType == WAV_TYPE){

                if (_size_ch1>0){ 
                    buff_ch1 = new uint8_t[_size_ch1 + _lostRate];
                    memcpy_neon(buff_ch1, _buffer_ch1, _size_ch1);
                    memset(buff_ch1 + _size_ch1 , 0 , sizeof(uint8_t) * _lostRate);
                }

                if (_size_ch2>0){ 
                    buff_ch2 = new uint8_t[_size_ch2 + _lostRate];
                    memcpy_neon(buff_ch2, _buffer_ch2, _size_ch2);
                    memset(buff_ch2 + _size_ch2 , 0 , sizeof(uint8_t) * _lostRate);
                }

                auto stream_data = m_waveWriter->BuildWAVStream(buff_ch1, _size_ch1, buff_ch2, _size_ch2,_resolution);
                if (!m_file_manager->AddBufferToWrite(stream_data))
                {
                    m_fileLogger->AddMetric(CFileLogger::Metric::FILESYSTEM_RATE,1);
                }
                delete [] buff_ch1;
                delete [] buff_ch2;

                uint64_t saveLostSize = 0;
                uint64_t saveSize = ZERO_BUFFER_SIZE < _lostRate ? ZERO_BUFFER_SIZE : _lostRate;
                while(((saveLostSize + saveSize) <= _lostRate) && (saveSize > 0)){
                    stream_data = m_waveWriter->BuildWAVStream(m_zeroBuffer, saveSize, m_zeroBuffer, saveSize,_resolution);
                    if (!m_file_manager->AddBufferToWrite(stream_data))
                    {
                        m_fileLogger->AddMetric(CFileLogger::Metric::FILESYSTEM_RATE,1);
                    }
                    saveLostSize += saveSize;
                    if ((saveLostSize + saveSize) > _lostRate){
                        saveSize = _lostRate - saveLostSize;
                    }
                }
            }
        }

        
        m_fileLogger->AddMetric(CFileLogger::Metric::RECIVE_DATE, _size_ch1 + _size_ch2);      
        m_fileLogger->AddMetric(CFileLogger::Metric::RECIVE_DATA_CH1,_size_ch1);
        m_fileLogger->AddMetric(CFileLogger::Metric::RECIVE_DATA_CH2,_size_ch2);            
        m_fileLogger->AddMetric(CFileLogger::Metric::OSC_RATE_LOST,_lostRate);        
        m_fileLogger->AddMetric(CFileLogger::Metric::OSC_RATE,_oscRate);       
        m_fileLogger->AddMetricId(_id);         
      
        if (notifyPassData)
            notifyPassData(_size_ch1 + _size_ch2);
        if (flag) 
            m_file_manager->StopWrite(true);
        return 1;
    }else{
        if (m_asionet){
            if (m_asionet->IsConnected()) {
                int m_ReadyToPass = 0;
                uint32_t frame_offset = 0;
                uint32_t buffer_size = MAX(_size_ch1, _size_ch2);
                uint32_t split_size = (m_asionet->GetProtocol() == asionet::Protocol::TCP ? TCP_BUFFER_LIMIT
                                                                                          : UDP_BUFFER_LIMIT);        

                if (split_size > buffer_size) {
                    split_size = buffer_size;
                }                                                                              

                size_t full_send_size = 0;
                buff_ch1 = (uint8_t *) _buffer_ch1;
                buff_ch2 = (uint8_t *) _buffer_ch2;
                uint32_t counter = 0;
                uint64_t sendLostRate = 0;
                while (((frame_offset + split_size) <= buffer_size) && (split_size > 0)) {
                                           
                    if ((frame_offset + split_size)  == buffer_size){
                        sendLostRate = _lostRate;
                    }

                    size_t new_buff_size = 0;
                    auto buffer = asionet::CAsioNet::BuildPack(m_index_of_message++, sendLostRate, _oscRate,  _resolution,
                                                               (&*buff_ch1 + frame_offset),
                                                               (_size_ch1 == 0 ? 0 : split_size),
                                                               (&*buff_ch2 + frame_offset),
                                                               (_size_ch2 == 0 ? 0 : split_size),
                                                               new_buff_size);

                    ++m_ReadyToPass;
                    
                    if (!m_asionet->SendData(false, buffer, new_buff_size)) {
                        m_ReadyToPass--;
                    }
                    delete buffer;
                    frame_offset += split_size;
                    if (frame_offset + split_size > buffer_size)
                        split_size = buffer_size - frame_offset;
                    counter++;
                }

                if (m_ReadyToPass > 0)
                    return 1;
                else
                    return 0;
            }else{
                return 0;
            }

        }
    }

}




