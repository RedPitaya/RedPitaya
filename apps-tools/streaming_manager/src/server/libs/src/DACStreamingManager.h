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
#include "stream_settings.h"
#include "neon_asm.h"
#include "thread_cout.h"
#include "ReaderController.h"


class CDACStreamingManager
{
    public:
        enum NotifyResult{
            NR_ENDED,
            NR_BROKEN,
            NR_EMPTY,
            NR_MISSING_FILE,
            NR_STOP
        };

        enum DACStream_FileType{
            TDMS_TYPE,
            WAV_TYPE
        };
           
        using Ptr = std::shared_ptr<CDACStreamingManager>;
    
        typedef std::function<void(NotifyResult)> Callback;
        typedef std::function<void()> CallbackVoid;
        typedef CDACAsioNetController DACAsio;

        static Ptr Create(DACStream_FileType _fileType, std::string _filePath, CStreamSettings::DACRepeat _repeat,int32_t _rep_count,int64_t memoryCacheSize);
        CDACStreamingManager(DACStream_FileType _fileType, std::string _filePath, CStreamSettings::DACRepeat _repeat,int32_t _rep_count,int64_t memoryCacheSize);

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

 CStreamSettings::DACRepeat m_repeat;
                    int32_t m_rep_count;
                    int64_t m_memoryCacheSize;
         CReaderController *m_readerController;

 auto startServer() -> void;
        auto stopServer() -> void;
};

#endif
