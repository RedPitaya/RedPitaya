#ifndef STREAMING_ROOT_DACSTREAMING_MANAGER_H
#define STREAMING_ROOT_DACSTREAMING_MANAGER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <functional>

#include "dac_net_controller.h"
#include "uio_lib/generator.h"
#include "settings_lib/dac_settings.h"
#include "reader_lib/reader_controller.h"
#include "data_lib/signal.hpp"

namespace dac_streaming_lib {

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

        sigslot::signal<NotifyResult> notifyStop;
        
private:
                       bool m_use_local_file;
         DACStream_FileType m_fileType;
                std::string m_host;
                std::string m_port;
                std::string m_filePath;
 CDACAsioNetController::Ptr m_asionet;

 CStreamSettings::DACRepeat m_repeat;
                    int32_t m_rep_count;
                    int64_t m_memoryCacheSize;
         CReaderController *m_readerController;

        auto startServer() -> void;
        auto stopServer() -> void;
};

}

#endif
