#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <asio.hpp>
#include <Oscilloscope.h>
#include <file_async_writer.h>
#include <wavWriter.h>
#include "AsioNet.h"
#include "FileLogger.h"
#include "neon_asm.h"
#include "shared_buffer.h"
#include "thread_cout.h"
#include "log.h"

//using rpsa::msg::message_factory;
//using rpsa::msg::message_type;

//#define FILE_PATH "/opt/redpitaya/www/apps/streaming_manager/upload"
#define FILE_PATH "/tmp/stream_files"

#define UDP_BUFFER_LIMIT 512
#define TCP_BUFFER_LIMIT 65536/2
#define ZERO_BUFFER_SIZE 1048576

#define MIN(X,Y) ((X < Y) ? X: Y)
#define MAX(X,Y) ((X > Y) ? X: Y)


class CStreamingManager
{
public:

    static void MakeEmptyDir(std::string _filePath);
  

    using Ptr = std::shared_ptr<CStreamingManager>;
    
    typedef std::function<void(int)> Callback;
    typedef std::function<void()> CallbackVoid;

    static Ptr Create(Stream_FileType _fileType,std::string _filePath, int _samples, bool _v_mode);
    CStreamingManager(Stream_FileType _fileType,std::string _filePath, int _samples, bool _v_mode);

    static Ptr Create(std::string _host, std::string _port, asionet::Protocol _protocol);
    CStreamingManager(std::string _host, std::string _port, asionet::Protocol _protocol);

    ~CStreamingManager();
    CStreamingManager(const COscilloscope &) = delete;
    CStreamingManager(COscilloscope &&) = delete;
    

    void run();
    void stop();
    bool isFileThreadWork();
    bool isOutOfSpace();
    bool convertToCSV();
    bool convertToCSV(std::string _file_name,int32_t start_seg, int32_t end_seg);
    void stopWriteToCSV();
    int passBuffers(uint64_t _lostRate, uint32_t _oscRate, uint32_t _adc_mode,uint32_t _adc_bits,const void *_buffer_ch1, uint32_t _size_ch1,const void *_buffer_ch2, uint32_t _size_ch2, unsigned short _resolution ,uint64_t _id);
    CStreamingManager::Callback notifyPassData;
    CStreamingManager::Callback notifyStop;
    CStreamingManager::CallbackVoid notifyPassDataReset;
    
private:
    CFileLogger::Ptr  m_fileLogger;
    std::atomic_int   m_ReadyToPass;
    std::atomic_int   m_SendData;
    FileQueueManager *m_file_manager;
    CWaveWriter      *m_waveWriter;
    std::string       m_host;
    std::string       m_port;
    std::string       m_filePath;
    asionet::Protocol m_protocol;
    asionet::CAsioNet *m_asionet;
    uint64_t          m_index_of_message;
    std::string       m_file_out;
    int               m_samples;  
    int               m_passSizeSamples;
    uint8_t           m_zeroBuffer[ZERO_BUFFER_SIZE];
    
    bool m_volt_mode;
    bool m_use_local_file;
    bool m_stopWriteCSV;
    Stream_FileType m_fileType;
    void startServer();
    void stopServer();
    uint8_t * convertBuffers(const void *_buffer,uint32_t _buf_size,size_t &_dest_buff_size,uint32_t _lostSize, uint32_t _adc_mode, uint32_t _adc_bits, unsigned short _resolution);
    
};
