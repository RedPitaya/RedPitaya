#include <chrono>
#include <cstring>
#include <iostream>
#include <fstream>
#include <time.h>
#include <functional>
#include <cstdlib>

#include "streaming_file.h"
#include "data_lib/neon_asm.h"
#include "data_lib/thread_cout.h"

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif

using namespace streaming_lib;

auto createDir(const std::string &dir) -> bool {
#ifdef _WIN32
    mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0777);
#endif
    return true;
}

auto createDirTree(const std::string &full_path) -> bool {
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

auto getNewFileName(CStreamSettings::DataFormat _fileType,std::string &_filePath,std::string &_prefix) -> std::string {
    createDirTree(_filePath);

//    char time_str[40];
//    struct tm *timenow;
//    time_t now = time(nullptr);
//    timenow = gmtime(&now);
//    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);

    std::string filename = _filePath  + "/" + std::string("data_file_") + _prefix + ".";
    if (_fileType == CStreamSettings::DataFormat::TDMS) filename += "tdms";
    if (_fileType == CStreamSettings::DataFormat::WAV)  filename += "wav";
    if (_fileType == CStreamSettings::DataFormat::BIN)  filename += "bin";
    
    return filename;
}

auto CStreamingFile::makeEmptyDir(const std::string &_filePath) -> void{
    createDirTree(_filePath);
}

auto CStreamingFile::create(CStreamSettings::DataFormat _fileType,std::string &_filePath, uint64_t _samples, bool _v_mode,bool testMode) -> CStreamingFile::Ptr{

    return std::make_shared<CStreamingFile>(_fileType, _filePath,_samples, _v_mode,testMode);
}

CStreamingFile::CStreamingFile(CStreamSettings::DataFormat _fileType,std::string &_filePath, uint64_t _samples, bool _v_mode,bool testMode) :
    m_fileLogger(nullptr),
    m_ReadyToPass(0),
    m_SendData(0),
    m_file_manager(nullptr),
    m_filePath(_filePath),
    m_file_out(""),
    m_samples(_samples),
    m_passSizeSamples(),
    m_testMode(testMode),
    m_volt_mode(_v_mode),
    m_disableNotify(false),
    m_fileType(_fileType)
{
    m_file_manager = new FileQueueManager(testMode);
    m_waveWriter = new CWaveWriter();

    m_file_manager->outSpaceNotify.connect([&](){
        stop(CStreamingFile::OUT_SPACE);
    });
}


CStreamingFile::~CStreamingFile()
{
    stop();
    if (m_file_manager){
        delete m_file_manager;
        m_file_manager = nullptr;
    }

    if (m_waveWriter){
        delete m_waveWriter;
        m_waveWriter = nullptr;
    }
}

auto CStreamingFile::disableNotify() -> void{
    m_disableNotify = true;
}

auto CStreamingFile::isFileThreadWork() -> bool {
    if (m_file_manager) {
        return m_file_manager->isWork();
    }
    return false;
}

auto CStreamingFile::isOutOfSpace() -> bool {
    if (m_file_manager) {
        return m_file_manager->isOutOfSpace();
    }
    return false;  
}

auto CStreamingFile::run(std::string _prefix) -> void {
    m_passSizeSamples.clear();
    m_passSizeSamples[DataLib::CH1] = 0;
    m_passSizeSamples[DataLib::CH2] = 0;
    m_passSizeSamples[DataLib::CH3] = 0;
    m_passSizeSamples[DataLib::CH4] = 0;

    m_file_out = getNewFileName(m_fileType, m_filePath, _prefix);
    m_fileLogger = CFileLogger::create(m_file_out + ".log",m_testMode);
    aprintf(stdout,"Run write to: %s\n",m_file_out.c_str());
    m_file_manager->openFile(m_file_out, false);
    m_file_manager->startWrite(m_fileType);
}

auto CStreamingFile::stop(CStreamingFile::EStopReason reason) -> void{
    std::lock_guard<std::mutex> lock(m_stopMtx);
    if (m_file_manager) {
        m_file_manager->stopWrite(false);
        if (m_testMode){
            m_file_manager->deleteFile();
        }        
    }
    if (m_fileLogger && !m_testMode)
        m_fileLogger->dumpToFile();
    if (!m_disableNotify){
        stopNotify(reason);
        m_disableNotify = true;
    }
}

auto CStreamingFile::stop() -> void {    
    stop(CStreamingFile::NORMAL);
}

auto CStreamingFile::convertBuffers(DataLib::CDataBuffersPack::Ptr pack, DataLib::EDataBuffersPackChannel channel,bool lockADCTo1V) -> SBuffPass {
    auto src_buff = pack->getBuffer(channel);
    if (!src_buff){
        auto pbuff = SBuffPass();
        pbuff.buffer = nullptr;
        pbuff.bufferLen = 0;
        pbuff.samplesCount = 0;
        pbuff.bitsBySample = 0;
        pbuff.adcSpeed = 0;
        return pbuff;
    }

    if (!m_volt_mode) {
        auto lostSizeBytes = src_buff->getLostSamplesInBytesLenght();
        auto destSize = src_buff->getBufferLenght() + lostSizeBytes;
        auto dest = net_lib::createBuffer(destSize);
        if(dest){
            memcpy_neon(dest.get(), src_buff->getBuffer().get(), src_buff->getBufferLenght());
            memset(dest.get() + src_buff->getBufferLenght(), 0 , sizeof(uint8_t) * lostSizeBytes);
        }
//         short *wb2 = ((short*)dest.get());
//         for(int i = 0 ;i < (destSize) /2 ;i ++)
//             std::cerr << std::hex <<  (static_cast<int>(wb2[i]) & 0xFFFF)  << " ";
//             exit(1);
        auto pbuff = SBuffPass();
        pbuff.buffer = dest;
        pbuff.bufferLen = destSize;
        pbuff.samplesCount = src_buff->getSamplesCount() + src_buff->getLostSamplesAll();
        pbuff.bitsBySample = src_buff->getBitBySample();
        pbuff.adcSpeed = pack->getOSCRate();
        return pbuff;
    }else{
        auto samples = src_buff->getSamplesCount();
        auto lostSamples = src_buff->getLostSamplesAll();
        auto bitBySamp = src_buff->getBitBySample();
        auto adcMode = (src_buff->getADCMode() == DataLib::CDataBuffer::ATT_1_20 ? 20 : 1);
        if (lockADCTo1V){
            adcMode = 1;
        }
        auto destSize = (samples + lostSamples) * sizeof(float);
        auto dest = net_lib::createBuffer(destSize);
        if (dest){
            auto dest_f = (float*)dest.get();

            for(uint32_t i = 0 ; i < samples; i++){
                if (bitBySamp == 8) {
                    int8_t* src_buff_bytes = (int8_t*)src_buff->getBuffer().get();
                    float cnt = src_buff_bytes[i];
                    dest_f[i] = cnt / ( 1 << (bitBySamp - 1)) * (float)adcMode;
                }

                if (bitBySamp == 16) {
                    auto src_buff_bytes = (int16_t*)src_buff->getBuffer().get();
                    float cnt = src_buff_bytes[i];
                    dest_f[i] = cnt / ( 1 << (bitBySamp - 1)) * (float)adcMode;
                }
            }
            memset(dest.get() + (samples * sizeof(float)), 0 , sizeof(float) * lostSamples);
        }
        auto pbuff = SBuffPass();
        pbuff.buffer = dest;
        pbuff.bufferLen = destSize;
        pbuff.samplesCount = samples + lostSamples;
        pbuff.bitsBySample = 32;
        pbuff.adcSpeed = pack->getOSCRate();
        return pbuff;
    }
}

auto CStreamingFile::getNetworkLost() -> uint64_t{
    if (m_fileLogger) {
        return m_fileLogger->getNetworkLost();
    }
    return 0;
}

auto CStreamingFile::getFileLost() -> uint64_t{
    if (m_fileLogger) {
        return m_fileLogger->getFileLost();
    }
    return 0;
}

auto CStreamingFile::getCSVFileName() -> std::string{
    return m_file_out;
}


auto CStreamingFile::passBuffers(DataLib::CDataBuffersPack::Ptr pack) -> int {
    if (!pack) return 0;
    if (m_fileType == CStreamSettings::TDMS) {
        // _adc_mode = 0 for 1:1 and 1 for 1:20 mode

        auto bufferCh1 = convertBuffers(pack,DataLib::CH1,false);
        auto bufferCh2 = convertBuffers(pack,DataLib::CH2,false);
        auto bufferCh3 = convertBuffers(pack,DataLib::CH3,false);
        auto bufferCh4 = convertBuffers(pack,DataLib::CH4,false);

        auto map = std::map<DataLib::EDataBuffersPackChannel,SBuffPass>();
        map[DataLib::CH1] = bufferCh1;
        map[DataLib::CH2] = bufferCh2;
        map[DataLib::CH3] = bufferCh3;
        map[DataLib::CH4] = bufferCh4;
        bool noMemoryException = false;
        for(auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++){
            DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
            if (map[ch].buffer == nullptr && map[ch].bufferLen){
                noMemoryException = true;
            }
        }
        if (!noMemoryException){
            if (m_samples != 0){
                for(auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++){
                    DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
                    if (map[ch].samplesCount + m_passSizeSamples[ch] > m_samples){
                        map[ch].samplesCount = m_samples - m_passSizeSamples[ch];
                        map[ch].bufferLen = map[ch].samplesCount * (map[ch].bitsBySample / 8);
                        m_passSizeSamples[ch] += map[ch].samplesCount;
                    }else{
                        m_passSizeSamples[ch] += map[ch].samplesCount;
                    }
                }
            }
            auto stream_data = buildTDMSStream(map);
            if (m_file_manager->isWork()){
                if (!m_file_manager->addBufferToWrite(stream_data)){
                    m_fileLogger->addMetric(CFileLogger::EMetric::FILESYSTEM_RATE,1);
                }
            }
        }else{
            m_fileLogger->addMetric(CFileLogger::EMetric::OUT_OF_MEMORY,1);
        }
    }


    if (m_fileType == CStreamSettings::WAV){

        // WAV type support float full range -1...1. adc_mode always equal 1
        auto bufferCh1 = convertBuffers(pack,DataLib::CH1,true);
        auto bufferCh2 = convertBuffers(pack,DataLib::CH2,true);
        auto bufferCh3 = convertBuffers(pack,DataLib::CH3,true);
        auto bufferCh4 = convertBuffers(pack,DataLib::CH4,true);

        auto map = std::map<DataLib::EDataBuffersPackChannel,SBuffPass>();
        map[DataLib::CH1] = bufferCh1;
        map[DataLib::CH2] = bufferCh2;
        map[DataLib::CH3] = bufferCh3;
        map[DataLib::CH4] = bufferCh4;

        bool noMemoryException = false;
        for(auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++){
            DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
            if (map[ch].buffer == nullptr && map[ch].bufferLen){
                noMemoryException = true;
            }
        }

        if (!noMemoryException){
            if (m_samples != 0){
                for(auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++){
                    DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
                    if (map[ch].samplesCount + m_passSizeSamples[ch] > m_samples){
                        map[ch].samplesCount = m_samples - m_passSizeSamples[ch];
                        map[ch].bufferLen = map[ch].samplesCount * (map[ch].bitsBySample / 8);
                        m_passSizeSamples[ch] += map[ch].samplesCount;
                    }else{
                        m_passSizeSamples[ch] += map[ch].samplesCount;
                    }
                }
            }
            auto stream_data = m_waveWriter->BuildWAVStream(map);
            if (m_file_manager->isWork()){
                if (!m_file_manager->addBufferToWrite(stream_data))
                {
                    m_fileLogger->addMetric(CFileLogger::EMetric::FILESYSTEM_RATE,1);
                }
            }
        }else{
            m_fileLogger->addMetric(CFileLogger::EMetric::OUT_OF_MEMORY,1);
        }
    }

    if (m_fileType == CStreamSettings::BIN){

        if (m_samples != 0){
            for(auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++){
                DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
                auto buff = pack->getBuffer(ch);
                if (buff){
                    if (m_passSizeSamples[ch] > m_samples){
                        buff->reset();
                    }else{
                        m_passSizeSamples[ch] += buff->getSamplesWithLost();
                    }
                }
            }
        }

        auto stream_data = buildBINStream(pack);
        if ( m_file_manager->isWork()){
            if (!m_file_manager->addBufferToWrite(stream_data))
            {
                m_fileLogger->addMetric(CFileLogger::EMetric::FILESYSTEM_RATE,1);
            }
        }
    }

    for(auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++){
        DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
        auto buff = pack->getBuffer(ch);
        if (buff){
            m_fileLogger->addMetric(CFileLogger::EMetric::RECIVE_DATE, buff->getBufferLenght());
            m_fileLogger->addMetric(ch,buff->getBufferLenght(),buff->getLostSamples(DataLib::FPGA),buff->getLostSamples(DataLib::RP_INTERNAL_BUFFER),buff->getSamplesCount());
        }
    }
    m_fileLogger->addMetric(CFileLogger::EMetric::OSC_RATE,pack->getOSCRate());
    m_fileLogger->addMetric(pack);

    if (m_samples){
        bool reachLimits = true;
        for(auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++){
            DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
            if (pack->isChannelPresent(ch)){
                if (m_passSizeSamples[ch] < m_samples){
                    reachLimits = false;
                    break;
                }
            }
        }
        if(reachLimits){
            stop(CStreamingFile::REACH_LIMIT);
        }
    }
    return 1;
}

auto CStreamingFile::addNetWorkLost(uint64_t count) -> void{
    m_fileLogger->addMetric(CFileLogger::EMetric::UPD_RATE,count);
}

