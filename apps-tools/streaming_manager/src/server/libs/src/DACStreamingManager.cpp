#include <chrono>
#include <iostream>
#include <fstream>
#include <time.h>
#include <functional>
#include <cstdlib>
#include "DACStreamingManager.h"



#define UNUSED(x) [&x]{}()

CDACStreamingManager::Ptr CDACStreamingManager::Create(DACStream_FileType _fileType, std::string _filePath, DACMode _mode){

    return std::make_shared<CDACStreamingManager>(_fileType, _filePath, _mode);
}

CDACStreamingManager::CDACStreamingManager(DACStream_FileType _fileType, std::string _filePath, DACMode _mode) :
    notifyStop(nullptr),
    m_use_local_file(true),
    m_fileType(_fileType),
    m_host(""),
    m_port(""),
    m_filePath(_filePath),
    m_asionet(nullptr),
    m_mode(_mode)
{   
    if (m_use_local_file){
    }
}

CDACStreamingManager::Ptr CDACStreamingManager::Create(std::string _host,std::string _port){
    return std::make_shared<CDACStreamingManager>(_host,_port);
}

CDACStreamingManager::CDACStreamingManager(std::string _host, std::string _port):
    notifyStop(nullptr),
    m_use_local_file(false),
    m_fileType(TDMS_TYPE),
    m_host(_host),
    m_port(_port),
    m_filePath(""),
    m_asionet(nullptr),
    m_mode(DACMode::RAW)
{
}

CDACStreamingManager::~CDACStreamingManager(){
    this->stop();
    this->stopServer();
}

auto CDACStreamingManager::startServer() -> void{
    if (m_asionet){
        delete m_asionet;
        m_asionet = nullptr;
    }
    m_asionet = new CDACAsioNetController();
    m_asionet->addHandler(CDACAsioNetController::Events::CONNECTED, [](std::string host){
        std::cout << "Client connected to DAC streaming server " << host << "\n" ;
    });
    m_asionet->addHandler(CDACAsioNetController::Events::DISCONNECTED, [](std::string host){
        std::cout << "Client disconnected from DAC streaming server " << host << "\n" ;
    });

    m_asionet->startAsioNet(asionet_simple::CAsioSocketSimple::ASMode::AS_SERVER,m_host,m_port);
}

auto CDACStreamingManager::stopServer() -> void {
    if (m_asionet) {
        delete m_asionet;
        m_asionet = nullptr;
    }
}

auto CDACStreamingManager::run() -> void {
    if (m_use_local_file){
        
    }
    else
        this->startServer();
}

auto CDACStreamingManager::stop() -> void {
    if (m_use_local_file){
        
    } else{
        this->stopServer();
    }
}

auto CDACStreamingManager::getBuffer() -> const CDACAsioNetController::BufferPack {
    if (m_use_local_file){
    } else{
        if (m_asionet)
            return m_asionet->getBuffer();
    }
    return CDACAsioNetController::BufferPack();
}

auto CDACStreamingManager::isLocalMode() -> bool{
    return m_use_local_file;
}



