#include <chrono>
#include <iostream>
#include <fstream>
#include <time.h>
#include <cstdlib>
#include "file_logger.h"


auto CFileLogger::create(std::string _filePath,bool testMode) -> CFileLogger::Ptr{
    return std::make_shared<CFileLogger>(_filePath,testMode);
}

CFileLogger::CFileLogger(std::string _filePath,bool testMode):
m_filePath(_filePath),
m_filePathLost(_filePath + ".lost"),
m_file_open(false),
m_oscRate(0),
m_udpLostRate(0),
m_fileSystemLostRate(0),
m_reciveData(0),
m_out_of_memory(0),
m_testMode(testMode),
m_channels(),
m_current_sample()
{
    resetCounters();
    m_file_open = true;
    if (!m_testMode){
        m_fileLost.open(m_filePathLost , std::ios_base::app | std::ios_base::out);
        if (m_fileLost.is_open()){
        }
    }
}

CFileLogger::~CFileLogger()
{
}

void CFileLogger::resetCounters(){
    m_out_of_memory = 0;
    m_udpLostRate = 0;
    m_fileSystemLostRate = 0;
    m_reciveData = 0;
    m_oscRate = 0;
    for(auto i = (int)DataLib::CH1; i < (int)DataLib::CH4; i++){
        DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
        m_current_sample[ch] = 0;
    }
    m_channels.clear();
}

auto CFileLogger::addMetric(CFileLogger::EMetric _metric, uint64_t _value) -> void{
    switch(_metric){

        case EMetric::OSC_RATE:
            m_oscRate = _value;
        break;

        case EMetric::UPD_RATE:
            m_udpLostRate += _value;
        break;

        case EMetric::OUT_OF_MEMORY:
            m_out_of_memory += _value;
        break;

        case EMetric::FILESYSTEM_RATE:
            m_fileSystemLostRate += _value;
            //LOG_P("FILESYSTEM_RATE: %jd\n",m_fileSystemLostRate);
        break;

        case EMetric::RECIVE_DATE:
            m_reciveData += _value;
        break;
        default:
        break;
    }
}

auto CFileLogger::getNetworkLost() -> uint64_t {
    return m_udpLostRate;
}

auto CFileLogger::getFileLost() -> uint64_t {
    return m_fileSystemLostRate;
}

auto CFileLogger::addMetric(DataLib::CDataBuffersPack::Ptr pack) -> void {
    const std::lock_guard<std::mutex> lock(m_mtx);
    if (m_file_open && m_fileLost.is_open()){
        for(auto i = (int)DataLib::CH1; i < (int)DataLib::CH4; i++){
            DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
            auto buff = pack->getBuffer(ch);
            if (buff){                
                m_fileLost << getName(ch) << ": Pos " << m_current_sample[ch] << " Get: " << buff->getSamplesCount() << " (" << buff->getLostSamplesAll() << ")\t";
                m_current_sample[ch] += buff->getSamplesWithLost();
            }
        }
        m_fileLost << "\n";
    }
}

auto CFileLogger::addMetric(DataLib::EDataBuffersPackChannel channel,uint64_t _value,uint64_t _lostFPGA,uint32_t _lostBUFFER,uint64_t _samples) -> void{
    const std::lock_guard<std::mutex> lock(m_mtx);
    if (m_channels.find(channel) != m_channels.end()){
        m_channels.at(channel).pass += _value;
        m_channels.at(channel).lostFpga += _lostFPGA;
        m_channels.at(channel).lostBuffer += _lostBUFFER;
        m_channels.at(channel).samples += _samples;

    }else{
        m_channels[channel].pass = _value;
        m_channels.at(channel).lostFpga = _lostFPGA;
        m_channels.at(channel).lostBuffer = _lostBUFFER;
        m_channels.at(channel).samples = _samples;
    }
}



void CFileLogger::dumpToFile(){
    const std::lock_guard<std::mutex> lock(m_mtx);
    try{
        if (!m_file_open) return;
        m_file_open = false;
        if (m_fileLost.is_open()){
            m_fileLost.close();
        }

        std::ofstream log(m_filePath , std::ios_base::app | std::ios_base::out);

        char buff[20];
        struct tm *sTm;
        time_t now = time (0);
        sTm = gmtime (&now);
        strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);
        log << "======================================================================\n";
        log << "====   Data transfer report  " << buff <<      " =====================\n";
        log << "======================================================================\n";
        log << "\n\n";
        log << "Current decimation factor:\t" << m_oscRate << "\n";
        log << "\n\n";
        log << "Lost data during transfer by UDP network:\t" << m_udpLostRate << "\n";
        log << "Lost data due to file write buffer overflow:\t" << m_fileSystemLostRate << "\n";
        log << "Loss of data due to lack of memory:\t" << m_out_of_memory << "\n";
        log << "\n";
        log << "Total amount of data transferred:\n";
        log << "\t-" << m_reciveData << "b \n";
        log << "\t-" << m_reciveData / 1024 << "kb \n";
        log << "\t-" << m_reciveData / (1024 * 1024) << "Mb \n";
        log << "\n";
        for (const auto& kv : m_channels) {
            log << "The total amount of data transmitted on: " << getName(kv.first)<< "\n";
            log << "\t-" << kv.second.samples << " Samples\n";
            log << "\t-" << kv.second.pass << "b \n";
            log << "\t-" << kv.second.pass / 1024 << "kb \n";
            log << "\t-" << kv.second.pass / (1024 * 1024) << "Mb \n";
            log << "\tLost data on: " << getName(kv.first) << "\n";
            log << "\t- FPGA:" << kv.second.lostFpga << " Samples\n";
            log << "\t- INTERNAL_BUFFER:" << kv.second.lostBuffer << " Samples\n";
            log << "\n";
        }

    }
    catch (std::exception& e)
	{
		fprintf(stderr, "Error: CFileLogger::DumpToFile() %s\n",e.what());
	}
}

auto CFileLogger::getName(DataLib::EDataBuffersPackChannel channel) -> std::string{
    switch (channel) {
        case DataLib::EDataBuffersPackChannel::CH1: return "Channel 1";
        case DataLib::EDataBuffersPackChannel::CH2: return "Channel 2";
        case DataLib::EDataBuffersPackChannel::CH3: return "Channel 3";
        case DataLib::EDataBuffersPackChannel::CH4: return "Channel 4";
        default:
            return "UNDEFINED";
    }
}
