#include <chrono>
#include <iostream>
#include <fstream>
#include <time.h>
#include <cstdlib>
#include "rpsa/server/core/FileLogger.h"


CFileLogger::Ptr CFileLogger::Create(std::string _filePath){
    return std::make_shared<CFileLogger>(_filePath);
}

CFileLogger::CFileLogger(std::string _filePath):  
m_filePath(_filePath),
m_filePathLost(_filePath + ".lost"),
m_oscLostRate(0),
m_oscRate(0),
m_udpLostRate(0),
m_fileSystemLostRate(0),
m_reciveData(0),
m_reciveData_ch1(0),
m_reciveData_ch2(0),
m_current_sample(0),
m_old_id(0)
{
    ResetCounters();
    m_fileLost.open(m_filePathLost , std::ios_base::app | std::ios_base::out);
    if (m_fileLost.is_open()){
        m_fileLost << "Start\tSize\n";
    }
}

CFileLogger::~CFileLogger()
{
    m_fileLost.close();
    DumpToFile();
}

void CFileLogger::ResetCounters(){
    m_oscLostRate = 0;
    m_udpLostRate = 0;
    m_fileSystemLostRate = 0;
    m_reciveData = 0;
    m_reciveData_ch1 = 0;
    m_reciveData_ch2 = 0;
    m_oscRate = 0;
    m_current_sample = 0;
}

void CFileLogger::AddMetric(CFileLogger::Metric _metric, uint64_t _value){
    switch(_metric){

        case Metric::OSC_RATE:
            m_oscRate = _value;
        break;

        case Metric::OSC_RATE_LOST:
            m_oscLostRate += _value;
            LOG_P("OSC_RATE_LOST: %jd\n",m_oscLostRate);
        break;
        
        case Metric::UPD_RATE:
            m_udpLostRate += _value;
        break;

        case Metric::FILESYSTEM_RATE:
            m_fileSystemLostRate += _value;
            LOG_P("FILESYSTEM_RATE: %jd\n",m_fileSystemLostRate);
        break;

        case Metric::RECIVE_DATE:
            m_reciveData += _value;
        break;

        case Metric::RECIVE_DATA_CH1:
            m_reciveData_ch1 += _value;
        break;

        case Metric::RECIVE_DATA_CH2:
            m_reciveData_ch2 += _value;
        break;

        default:
        break;
    }
}

void CFileLogger::AddMetricId(uint64_t _id){
    if (_id != 0 && m_old_id != 0)
        if (_id != (m_old_id + 1))
        {   
            m_udpLostRate++;   
        }
    m_old_id = _id;
}

void CFileLogger::AddMetric(uint64_t _samples_data,uint64_t _lost){
    if (m_fileLost.is_open()){
        if (_samples_data > 0){
            m_fileLost << m_current_sample << "\t" << _samples_data << "\n";
            m_current_sample += _samples_data;
        }

        if (_lost > 0){
            m_fileLost << m_current_sample << "\t" << _lost << "\tlost\n";
            m_current_sample += _lost;
        }
    }
}


void CFileLogger::DumpToFile(){
    
    try{
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
        log << "Missed data samples when reading from ADC:\t" << m_oscLostRate << "\n";
        log << "Lost data during transfer by network:\t" << m_udpLostRate << "\n";
        log << "Lost data due to file write buffer overflow:\t" << m_fileSystemLostRate << "\n";
        log << "\n";
        log << "Total amount of data transferred:\n";
        log << "\t-" << m_reciveData << "b \n";
        log << "\t-" << m_reciveData / 1024 << "kb \n";
        log << "\t-" << m_reciveData / (1024 * 1024) << "Mb \n";
        log << "\n";
        log << "The total amount of data transmitted on the first channel:\n";
        log << "\t-" << m_reciveData_ch1 << "b \n";
        log << "\t-" << m_reciveData_ch1 / 1024 << "kb \n";
        log << "\t-" << m_reciveData_ch1 / (1024 * 1024) << "Mb \n";
        log << "\n";
        log << "The total amount of data transmitted on the second channel:\n";
        log << "\t-" << m_reciveData_ch2 << "b \n";
        log << "\t-" << m_reciveData_ch2 / 1024 << "kb \n";
        log << "\t-" << m_reciveData_ch2 / (1024 * 1024) << "Mb \n";
    }
    catch (std::exception& e)
	{
		fprintf(stderr, "Error: CFileLogger::DumpToFile() %s\n",e.what());
	}
}