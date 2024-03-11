#include <chrono>
#include <iostream>
#include <fstream>
#include <time.h>
#include <functional>
#include <cstdlib>

#include "data_lib/thread_cout.h"
#include "dac_streaming_manager.h"

using namespace dac_streaming_lib;

CDACStreamingManager::Ptr CDACStreamingManager::Create(DACStream_FileType _fileType, std::string _filePath,CStreamSettings::DACRepeat _repeat,int32_t _rep_count,int64_t memoryCacheSize){

    return std::make_shared<CDACStreamingManager>(_fileType, _filePath, _repeat, _rep_count, memoryCacheSize);
}

CDACStreamingManager::CDACStreamingManager(DACStream_FileType _fileType, std::string _filePath, CStreamSettings::DACRepeat _repeat,int32_t _rep_count,int64_t memoryCacheSize) :
    m_use_local_file(true),
    m_fileType(_fileType),
    m_host(""),
    m_port(""),
    m_filePath(_filePath),
    m_asionet(nullptr),
    m_repeat(_repeat),
    m_rep_count(_rep_count),
    m_memoryCacheSize(memoryCacheSize),
    m_readerController(nullptr)
{
    if (m_fileType == DACStream_FileType::TDMS_TYPE){
        m_readerController = new CReaderController(CStreamSettings::TDMS,m_filePath,m_repeat,m_rep_count,m_memoryCacheSize);
    }

    if (m_fileType == DACStream_FileType::WAV_TYPE){
        m_readerController = new CReaderController(CStreamSettings::WAV,m_filePath,m_repeat,m_rep_count,m_memoryCacheSize);
    }
}

CDACStreamingManager::Ptr CDACStreamingManager::Create(std::string _host,std::string _port){
    return std::make_shared<CDACStreamingManager>(_host,_port);
}

CDACStreamingManager::CDACStreamingManager(std::string _host, std::string _port):
    m_use_local_file(false),
    m_fileType(TDMS_TYPE),
    m_host(_host),
    m_port(_port),
    m_filePath(""),
    m_asionet(nullptr),
    m_repeat(CStreamSettings::DAC_REP_OFF),
    m_rep_count(0),
    m_memoryCacheSize(0),
    m_readerController(nullptr)
{
}

CDACStreamingManager::~CDACStreamingManager(){
    this->stop();
    this->stopServer();
    if (m_readerController){
        delete m_readerController;
        m_readerController = nullptr;
    }
}

auto CDACStreamingManager::startServer() -> void{
    m_asionet = nullptr;
    m_asionet = std::make_shared<CDACAsioNetController>();
    m_asionet->connectedNotify.connect([](std::string &host){
        aprintf(stdout,"Client connected to DAC streaming server %s\n", host.c_str());
    });
    m_asionet->disconnectedNotify.connect([](std::string &host){
        aprintf(stdout,"Client disconnected from DAC streaming server %s\n", host.c_str());
    });

    m_asionet->startAsioNet(net_lib::M_SERVER,m_host,m_port);
}

auto CDACStreamingManager::stopServer() -> void {
    m_asionet = nullptr;
}

auto CDACStreamingManager::run() -> void {
    if (!m_use_local_file){
        this->startServer();
    }
}

auto CDACStreamingManager::stop() -> void {
    if (!m_use_local_file){
        this->stopServer();
    }
}

auto CDACStreamingManager::getBuffer() -> const CDACAsioNetController::BufferPack {
    if (m_use_local_file){
        CDACAsioNetController::BufferPack pack;
        if (m_readerController){
            auto isOpen = m_readerController->isOpen();
            if (isOpen != CReaderController::OR_OK){
                if (isOpen == CReaderController::OR_CLOSE){
                    notifyStop(CDACStreamingManager::NR_MISSING_FILE);
                    return CDACAsioNetController::BufferPack();
                }
                notifyStop(CDACStreamingManager::NR_BROKEN);
                return CDACAsioNetController::BufferPack();
            }
            uint8_t *ch1_read = nullptr;
            uint8_t *ch2_read = nullptr;
            size_t size1_read = 0;
            size_t size2_read = 0;
            auto res = m_readerController->getBufferPrepared(&ch1_read,&size1_read,&ch2_read,&size2_read);
            if (size1_read != 0 || size2_read != 0){
                pack.ch1 = ch1_read;
                pack.ch2 = ch2_read;
                pack.size_ch1 = size1_read;
                pack.size_ch2 = size2_read;
                if (pack.size_ch1 || pack.size_ch2){
                    pack.empty = false;
                }
            }else{
                auto sendRes = CDACStreamingManager::NR_STOP;
                switch(res){
                    case CReaderController::BR_BROKEN:
                        sendRes = CDACStreamingManager::NR_BROKEN;
                    break;
                    case CReaderController::BR_EMPTY:
                        sendRes = CDACStreamingManager::NR_EMPTY;
                    break;
                    case CReaderController::BR_ENDED:
                        sendRes = CDACStreamingManager::NR_ENDED;
                    break;
                    default:
                    break;
                }

                notifyStop(sendRes);
            }
        }
        return pack;
    } else{
        if (m_asionet)
            return m_asionet->getBuffer();
    }
    return CDACAsioNetController::BufferPack();
}

auto CDACStreamingManager::isLocalMode() -> bool{
    return m_use_local_file;
}



