#pragma once
#include <iostream>
#include <memory>

class CFileLogger{
public:
    enum Metric{
        OSC_RATE_LOST,
        OSC_RATE,
        UPD_RATE,
        FILESYSTEM_RATE,
        RECIVE_DATE,
        RECIVE_DATA_CH1,
        RECIVE_DATA_CH2
    };

    using Ptr = std::shared_ptr<CFileLogger>;
    static Ptr Create(std::string _filePath);

    CFileLogger(std::string _filePath);
    ~CFileLogger();

    void ResetCounters();
    void AddMetric(CFileLogger::Metric _metric, uint64_t _value);
    void AddMetricId(uint64_t _id);

    void DumpToFile();

private:
    
    CFileLogger(const CFileLogger &) = delete;
    CFileLogger(CFileLogger &&) = delete;

    std::string m_filePath;
    uint64_t    m_oscRate;
    uint64_t    m_oscLostRate;
    uint64_t    m_udpLostRate;
    uint64_t    m_fileSystemLostRate;
    uint64_t    m_reciveData;
    uint64_t    m_reciveData_ch1;
    uint64_t    m_reciveData_ch2;
    uint64_t    m_old_id;
};