#ifndef LOGGER_LIB_FILE_LOGGER_H
#define LOGGER_LIB_FILE_LOGGER_H

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <mutex>
#include "data_lib/buffers_pack.h"

enum PColor { WHITE, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN };

void aprintf(FILE* stream, const char* format, ...);

void acprintf(FILE* stream, PColor color, const char* format, ...);

#define __SHORT_FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define FATAL(...)                                                                                                                              \
    {                                                                                                                                           \
        char error_msg[1024];                                                                                                                   \
        snprintf(error_msg, 1024, __VA_ARGS__);                                                                                                 \
        acprintf(stderr, PColor::RED, "Fatal error at line %d, file %s:%s %s\n", __LINE__, __SHORT_FILENAME__, __PRETTY_FUNCTION__, error_msg); \
        exit(1);                                                                                                                                \
    }
#define ERROR_LOG(...)                                                                                                       \
    {                                                                                                                        \
        char error_msg[1024];                                                                                                \
        snprintf(error_msg, 1024, __VA_ARGS__);                                                                              \
        acprintf(stderr, PColor::RED, "[E] {%s:%s}(%d) %s\n", __SHORT_FILENAME__, __PRETTY_FUNCTION__, __LINE__, error_msg); \
    }
#define WARNING(...)                                                                                                            \
    {                                                                                                                           \
        char error_msg[1024];                                                                                                   \
        snprintf(error_msg, 1024, __VA_ARGS__);                                                                                 \
        acprintf(stderr, PColor::YELLOW, "[W] {%s:%s}(%d) %s\n", __SHORT_FILENAME__, __PRETTY_FUNCTION__, __LINE__, error_msg); \
    }

#ifdef TRACE_ENABLE
#define TRACE(...)                                                                                                            \
    {                                                                                                                         \
        char error_msg[1024];                                                                                                 \
        snprintf(error_msg, 1024, __VA_ARGS__);                                                                               \
        acprintf(stderr, PColor::CYAN, "[T] {%s:%s}(%d) %s\n", __SHORT_FILENAME__, __PRETTY_FUNCTION__, __LINE__, error_msg); \
    }
#define TRACE_SHORT(...)                                       \
    {                                                          \
        char error_msg[1024];                                  \
        snprintf(error_msg, 1024, __VA_ARGS__);                \
        acprintf(stderr, PColor::CYAN, "[T] %s\n", error_msg); \
    }
#else
#define TRACE(...)
#define TRACE_SHORT(...)
#endif

class CFileLogger {
   public:
    enum EMetric { OSC_RATE, FILESYSTEM_RATE, OUT_OF_MEMORY, RECIVE_DATE };

    using Ptr = std::shared_ptr<CFileLogger>;
    static auto create(std::string _filePath, bool testMode) -> Ptr;

    CFileLogger(std::string _filePath, bool testMode);
    ~CFileLogger();

    auto resetCounters() -> void;
    auto addMetric(CFileLogger::EMetric _metric, uint64_t _value) -> void;
    auto addMetric(DataLib::CDataBuffersPackDMA::Ptr pack) -> void;
    auto addMetric(DataLib::EDataBuffersPackChannel channel, uint64_t _value, uint64_t _lostFPGA, uint64_t _samples) -> void;

    auto dumpToFile() -> void;

    auto getFileLost() -> uint64_t;

   private:
    struct ChStat {
        uint64_t pass;
        uint64_t lostFpga;
        uint64_t samples;
    };

    CFileLogger(const CFileLogger&) = delete;
    CFileLogger(CFileLogger&&) = delete;
    CFileLogger& operator=(const CFileLogger&) = delete;
    CFileLogger& operator=(const CFileLogger&&) = delete;

    auto getName(DataLib::EDataBuffersPackChannel channel) -> std::string;

    std::string m_filePath;
    std::string m_filePathLost;
    std::mutex m_mtx;
    bool m_file_open;
    uint64_t m_oscRate;
    uint64_t m_fileSystemLostRate;
    uint64_t m_reciveData;
    uint64_t m_out_of_memory;
    std::ofstream m_fileLost;
    bool m_testMode;
    std::map<DataLib::EDataBuffersPackChannel, ChStat> m_channels;
    std::map<DataLib::EDataBuffersPackChannel, uint64_t> m_current_sample;
};

#endif
