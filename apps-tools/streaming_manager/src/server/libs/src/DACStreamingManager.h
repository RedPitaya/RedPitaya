#ifndef STREAMING_ROOT_DACSTREAMING_MANAGER_H
#define STREAMING_ROOT_DACSTREAMING_MANAGER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <asio.hpp>
#include "Generator.h"
#include "DACAsioNetController.h"
#include "neon_asm.h"
#include "thread_cout.h"


class CDACStreamingManager
{
    public:
        enum DACStream_FileType{
            TDMS_TYPE,
            WAV_TYPE
        };
        
        enum DACMode{
            RAW,
            VOLT
        };
    
        using Ptr = std::shared_ptr<CDACStreamingManager>;
    
        typedef std::function<void(int)> Callback;
        typedef std::function<void()> CallbackVoid;
        typedef CDACAsioNetController DACAsio;

        static Ptr Create(DACStream_FileType _fileType, std::string _filePath, DACMode _mode);
        CDACStreamingManager(DACStream_FileType _fileType, std::string _filePath, DACMode _mode);

        static Ptr Create(std::string _host, std::string _port);
        CDACStreamingManager(std::string _host, std::string _port);

        ~CDACStreamingManager();
        CDACStreamingManager(const CDACStreamingManager &) = delete;
        CDACStreamingManager(CDACStreamingManager &&) = delete;
    

        auto run() -> void;
        auto stop() -> void;
        auto isLocalMode() -> bool;
        auto getBuffer() -> const CDACAsioNetController::BufferPack;

        CDACStreamingManager::Callback notifyStop;
        
private:
                  bool m_use_local_file;
    DACStream_FileType m_fileType;
           std::string m_host;
           std::string m_port;
           std::string m_filePath;
              DACAsio *m_asionet;
              DACMode  m_mode;
    
        auto startServer() -> void;
        auto stopServer() -> void;
};

#endif