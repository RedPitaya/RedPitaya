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

#define UNUSED(x) [&x]{}()

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
    std::string filename = _filePath  + "/" + std::string("data_file_") + time_str + ".";
    if (_fileType == Stream_FileType::TDMS_TYPE) filename += "tdms";
    if (_fileType == Stream_FileType::WAV_TYPE)  filename += "wav";
    if (_fileType == Stream_FileType::CSV_TYPE)  filename += "bin";
    
    return filename;
}

void CStreamingManager::MakeEmptyDir(std::string _filePath){
    createDirTree(_filePath);
}



CStreamingManager::Ptr CStreamingManager::Create(Stream_FileType _fileType,std::string _filePath, int _samples, bool _v_mode){

    return std::make_shared<CStreamingManager>(_fileType, _filePath,_samples, _v_mode);
}

CStreamingManager::CStreamingManager(Stream_FileType _fileType,std::string _filePath, int _samples, bool _v_mode) :
    m_volt_mode(_v_mode),
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
        m_volt_mode(false),
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
        m_stopWriteCSV = false;
}

CStreamingManager::~CStreamingManager()
{
    this->stop();
    stopWriteToCSV();
    if (m_file_manager){
        delete m_file_manager;
        m_file_manager = nullptr;
    }

    if (m_waveWriter){
        delete m_waveWriter;
        m_waveWriter = nullptr;
    }

    if (m_asionet){
        delete m_asionet;
        m_asionet = nullptr;
    }

}

void CStreamingManager::stopWriteToCSV(){
    m_stopWriteCSV = true;
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
                                UNUSED(error);
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
        if (m_file_manager) {
            m_file_manager->StopWrite(false);
        }
    } else{
        this->stopServer();
    }
}

uint8_t * CStreamingManager::convertBuffers(const void *_buffer,uint32_t _buf_size,size_t &_dest_buff_size,uint32_t _lostSize, uint32_t _adc_mode, uint32_t _adc_bits, unsigned short _resolution){
    UNUSED(_adc_bits);
    uint8_t *dest = nullptr;

    if (!m_volt_mode) {
        dest = new uint8_t[_buf_size + _lostSize];
        _dest_buff_size = _buf_size + _lostSize;
        memcpy_neon(dest, _buffer, _buf_size);
   
        // short *wb2 = ((short*)dest);
        // for(int i = 0 ;i < (_buf_size) /2 ;i ++)
        //     std::cout << std::hex <<  (static_cast<int>(wb2[i]) & 0xFFFF)  << " ";
        //     exit(1);
    }else{
        uint32_t samples = _buf_size / (_resolution  / 8);
        float *dest_f = new float[samples + _lostSize / 4];
        _dest_buff_size = (samples + _lostSize / 4) * sizeof(float);
        for(uint32_t i = 0 ; i < samples; i++){
            float cnt = (_resolution == 8) ?  ((int8_t*)_buffer)[i]:((int16_t*)_buffer)[i];
//            dest_f[i] = cnt / ( 1 << (_adc_bits - 1));
            dest_f[i] = cnt / ( 1 << (_resolution - 1)) * (float)_adc_mode;
        }

        dest = reinterpret_cast<uint8_t*>(dest_f);
    }
    return dest;
}


int CStreamingManager::passBuffers(uint64_t _lostRate, uint32_t _oscRate, uint32_t _adc_mode, uint32_t _adc_bits, const void *_buffer_ch1, uint32_t _size_ch1,const void *_buffer_ch2, uint32_t _size_ch2, unsigned short _resolution, uint64_t _id){


   // ASIO_ASSERT(!(_size_ch1 != _size_ch2 && _size_ch1 != 0 && _size_ch2 != 0));
    uint8_t *buff_ch1 = nullptr;
    uint8_t *buff_ch2 = nullptr;
    size_t   buff_ch1_size = 0;
    size_t   buff_ch2_size = 0;    
    uint8_t  byte_per_sample = (_resolution == 16 ? 2 : 1);


    if (m_use_local_file){
        bool flag = false;
        if (m_samples != -1) {
            uint8_t devider = (_resolution == 16 ? 2 : 1);
            m_passSizeSamples += (_size_ch1 > 0 ? _size_ch1  : _size_ch2) / devider;
            if (m_passSizeSamples >= m_samples) {
                flag = true;
                _lostRate = 0;
                int diff = (m_passSizeSamples - m_samples) * devider;
                if (_size_ch1 > 0) {
                    if (diff > (int)_size_ch1)
                        _size_ch1 = 0;
                    else
                        _size_ch1 -= diff;
                }
                if (_size_ch2 > 0) {
                     if (diff > (int)_size_ch2)
                        _size_ch2 = 0;
                    else
                        _size_ch2 -= diff;
                }
            }
        }

        uint32_t samples_buff1 = _size_ch1 / byte_per_sample;
        uint32_t samples_buff2 = _size_ch2 / byte_per_sample;
        if (m_volt_mode) byte_per_sample = 4; // FLOAT TYPE
        uint32_t lostSize = _lostRate * byte_per_sample;

        if (_size_ch1 + _size_ch2  + lostSize> 0){
            
            if (m_fileType == TDMS_TYPE){
                // _adc_mode = 0 for 1:1 and 1 for 1:20 mode 
                if (_size_ch1 > 0 || lostSize > 0){ 
                    buff_ch1 = convertBuffers(_buffer_ch1,_size_ch1,buff_ch1_size,lostSize,_adc_mode == 1 ? 1 : 20,_adc_bits,_resolution);
                    assert(buff_ch1 && "wav writer: Buffer 1 is null");                    
                    memset(buff_ch1 + (samples_buff1 * byte_per_sample)  , 0 , sizeof(uint8_t) * lostSize);
                }

                if (_size_ch2 > 0 || lostSize > 0){ 
                    buff_ch2 = convertBuffers(_buffer_ch2,_size_ch2,buff_ch2_size,lostSize,_adc_mode == 1 ? 1 : 20,_adc_bits,_resolution);
                    assert(buff_ch2 && "wav writer: Buffer 2 is null");
                    memset(buff_ch2 + (samples_buff2 * byte_per_sample) , 0 , sizeof(uint8_t) * lostSize);
                }

                auto stream_data = m_file_manager->BuildTDMSStream(buff_ch1, buff_ch1_size, buff_ch2, buff_ch2_size, byte_per_sample * 8);
                if (!m_file_manager->AddBufferToWrite(stream_data))
                {
                    m_fileLogger->AddMetric(CFileLogger::Metric::FILESYSTEM_RATE,1);
                }
            }


            if (m_fileType == WAV_TYPE){

                // WAV type support float full range -1...1. adc_mode always equal 1
                if (_size_ch1>0){ 
                    buff_ch1 = convertBuffers(_buffer_ch1,_size_ch1,buff_ch1_size,0, 1 ,_adc_bits,_resolution);
                    assert(buff_ch1 && "wav writer: Buffer 1 is null");   
                }

                if (_size_ch2>0){ 
                    buff_ch2 = convertBuffers(_buffer_ch2,_size_ch2,buff_ch2_size,0, 1 ,_adc_bits,_resolution);
                    assert(buff_ch2 && "wav writer: Buffer 2 is null");
                }
                if (_size_ch1 + _size_ch2 > 0){
                    auto stream_data = m_waveWriter->BuildWAVStream(buff_ch1, buff_ch1_size, buff_ch2, buff_ch2_size, byte_per_sample * 8);
                    if (!m_file_manager->AddBufferToWrite(stream_data))
                    {
                        m_fileLogger->AddMetric(CFileLogger::Metric::FILESYSTEM_RATE,1);
                    }
                }
                delete [] buff_ch1;
                delete [] buff_ch2;

                uint64_t saveLostSize = 0;
                uint64_t saveSize = (ZERO_BUFFER_SIZE < lostSize ? ZERO_BUFFER_SIZE : lostSize);
                while(((saveLostSize + saveSize) <= lostSize) && (saveSize > 0)){
                    auto stream_data = m_waveWriter->BuildWAVStream(m_zeroBuffer, saveSize, m_zeroBuffer, saveSize, byte_per_sample * 8);
                    if (!m_file_manager->AddBufferToWrite(stream_data))
                    {
                        m_fileLogger->AddMetric(CFileLogger::Metric::FILESYSTEM_RATE,1);
                    }
                    saveLostSize += saveSize;
                    if ((saveLostSize + saveSize) > lostSize){
                        saveSize = lostSize - saveLostSize;
                    }
                }
            }       

            if (m_fileType == CSV_TYPE){
                 if (_size_ch1 > 0){ 
                    buff_ch1 = convertBuffers(_buffer_ch1,_size_ch1,buff_ch1_size, 0 ,_adc_mode == 1 ? 1 : 20,_adc_bits,_resolution);
                    assert(buff_ch1 && "wav writer: Buffer 1 is null");                    
                }

                if (_size_ch2 > 0){ 
                    buff_ch2 = convertBuffers(_buffer_ch2,_size_ch2,buff_ch2_size, 0 ,_adc_mode == 1 ? 1 : 20,_adc_bits,_resolution);
                    assert(buff_ch2 && "wav writer: Buffer 2 is null");
                }

                auto stream_data = m_file_manager->BuildBINStream(buff_ch1, buff_ch1_size, buff_ch2, buff_ch2_size, byte_per_sample * 8, lostSize);
                if (!m_file_manager->AddBufferToWrite(stream_data))
                {
                    m_fileLogger->AddMetric(CFileLogger::Metric::FILESYSTEM_RATE,1);
                }
                delete [] buff_ch1;
                delete [] buff_ch2;
            }
        }

        
        m_fileLogger->AddMetric(CFileLogger::Metric::RECIVE_DATE, _size_ch1 + _size_ch2);      
        m_fileLogger->AddMetric(CFileLogger::Metric::RECIVE_DATA_CH1,_size_ch1);
        m_fileLogger->AddMetric(CFileLogger::Metric::RECIVE_DATA_CH2,_size_ch2);            
        m_fileLogger->AddMetric(CFileLogger::Metric::OSC_RATE_LOST,_lostRate);        
        m_fileLogger->AddMetric(CFileLogger::Metric::OSC_RATE,_oscRate);       
        m_fileLogger->AddMetricId(_id);         
        m_fileLogger->AddMetric(MAX(samples_buff1,samples_buff2),_lostRate);
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

                buff_ch1 = (uint8_t *) _buffer_ch1;
                buff_ch2 = (uint8_t *) _buffer_ch2;
 
                size_t new_buff_size = 0;
                while ((frame_offset + split_size) <= buffer_size) {
                    if (frame_offset + split_size > buffer_size)
                        split_size = buffer_size - frame_offset;

                    
                    auto buffer = asionet::CAsioNet::BuildPack(m_index_of_message++, 0, _oscRate,  _resolution, _adc_mode ,_adc_bits,
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
                }
                 // Send empty pack with lost
                if (_lostRate>0) {
                    auto buffer = asionet::CAsioNet::BuildPack(m_index_of_message++, _lostRate, _oscRate,  _resolution, _adc_mode ,_adc_bits,
                                                                    nullptr,
                                                                    0,
                                                                    nullptr,
                                                                    0,
                                                                    new_buff_size);
                    ++m_ReadyToPass;    
                    if (!m_asionet->SendData(false, buffer, new_buff_size)) {
                        m_ReadyToPass--;
                    }
                    delete buffer;
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
    return 0;
}

bool CStreamingManager::convertToCSV(){
    return convertToCSV(m_file_out,-2,-2);
}

bool CStreamingManager::convertToCSV(std::string _file_name,int32_t start_seg, int32_t end_seg){
    bool ret = true;
    try{
        if (m_stopWriteCSV) return false;
        acout() << "Started converting to CSV\n";
        std::string csv_file = _file_name.substr(0, _file_name.size()-3) + "csv";
        acout() << csv_file << "\n";
        std::fstream fs;
        std::fstream fs_out;
        fs.open(_file_name, std::ios::binary | std::ofstream::in | std::ofstream::out);
        fs_out.open(csv_file, std::ofstream::in |  std::ofstream::trunc | std::ofstream::out);
        if (fs.fail() || fs_out.fail()) {
            acout() << " Error open files\n";
            ret = false;
        }else{
            fs.seekg(0, std::ios::end);
            int64_t Length = fs.tellg();
            int64_t position = 0;
            int32_t curSegment = 0;
            int     channels = 0;
            start_seg = MAX(start_seg,1);
            while(position >= 0){
                auto freeSize = FileQueueManager::GetFreeSpaceDisk(csv_file);
                if (freeSize <= USING_FREE_SPACE){
                    acout() << "\nDisk is full\n";
                    ret = false;
                    break;
                }
                if (m_stopWriteCSV){
                    acout() << "\nAbort writing to CSV file\n";
                    ret = false;
                    break;
                }
                curSegment++;
                bool notSkip = (start_seg <= curSegment) && ((end_seg != -2 && end_seg >= curSegment) || end_seg == -2);
                auto csv_seg = FileQueueManager::ReadCSV(&fs,&position, &channels, !notSkip);
                if (end_seg == -2){
                    if (position >=0) {
                        acout() << "\rPROGRESS: " << (position * 100) / Length  << " %";
                    }else{
                        if (position == -2){
                            acout() << "\rPROGRESS: 100 %";
                        }
                    }
                }else{
                    if (curSegment - start_seg >= 0 && (end_seg-1) > start_seg) {
                        acout() << "\rPROGRESS: " << ((curSegment - start_seg - 1) * 100) / (end_seg - start_seg)  << " %";
                    }
                }
                
                if (notSkip && csv_seg){
                    csv_seg->seekg(0, csv_seg->beg);  
                    fs_out << csv_seg->rdbuf();
                    fs_out.flush();
                    fs_out.sync();
                }
                delete csv_seg;
                
                if (end_seg != -2 && end_seg < curSegment){
                    break;
                }

                if (fs.fail() || fs_out.fail()) {
                    acout() << "\nError write to CSV file\n";
                    if (fs.fail()) acout() << "FS is fail\n";
                    if (fs_out.fail()) acout()  << "FS out is fail\n";
                    
                    ret = false;
                    break;
                }
                
            }
        }
        acout() << "\nEnded converting\n";
    }catch (std::exception& e)
	{
		std::cerr << "Error: convertToCSV() : " << e.what() << std::endl ;
        ret = false;
	}
    return ret;
}



