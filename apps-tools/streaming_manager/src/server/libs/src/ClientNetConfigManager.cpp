#include "ClientNetConfigManager.h"


ClientNetConfigManager::ClientNetConfigManager(std::string defualt_file_settings_path,std::string host,std::string port):CStreamSettings(),
    m_pBroadcast(nullptr),
    m_pNetConfManager(nullptr),
    m_currentState(States::NORMAL),
    m_file_settings(defualt_file_settings_path)
{
    m_pNetConfManager = std::make_shared<CNetConfigManager>();
    m_pNetConfManager->addHandlerReceiveCommand(std::bind(&ClientNetConfigManager::receiveCommand, this, std::placeholders::_1));
    m_pNetConfManager->addHandlerReceiveStrStr(std::bind(&ClientNetConfigManager::receiveValueStr, this, std::placeholders::_1, std::placeholders::_2));
    m_pNetConfManager->addHandlerReceiveStrInt(std::bind(&ClientNetConfigManager::receiveValueInt, this, std::placeholders::_1, std::placeholders::_2));
    m_pNetConfManager->addHandlerReceiveStrDouble(std::bind(&ClientNetConfigManager::receiveValueDouble, this, std::placeholders::_1, std::placeholders::_2));

    m_pNetConfManager->addHandlerError(std::bind(&ClientNetConfigManager::serverError, this, std::placeholders::_1));
    m_pNetConfManager->startAsioNet(asionet_simple::CAsioSocketSimple::ASMode::AS_SERVER,host,port);
    readFromFile(defualt_file_settings_path);
}

ClientNetConfigManager::~ClientNetConfigManager(){

}

auto ClientNetConfigManager::startBroadcast(asionet_broadcast::CAsioBroadcastSocket::ABMode mode,std::string host,std::string port) -> void{
    m_pBroadcast = asionet_broadcast::CAsioBroadcastSocket::Create(host,port);
    m_pBroadcast->InitServer(mode);
    m_pBroadcast->addHandler(asionet_broadcast::CAsioBroadcastSocket::ABEvents::AB_ERROR,[this](std::error_code er){
        fprintf(stderr,"[ServerNetConfigManager] Broadcast server error: %s (%d)\n",er.message().c_str(),er.value());
        m_errorCallback.emitEvent(0,Errors::BROADCAST_ERROR);
    });
}

auto ClientNetConfigManager::isConnected() -> bool{
    return m_pNetConfManager->isConnected();
}

auto ClientNetConfigManager::addHandlerError(std::function<void(ClientNetConfigManager::Errors)> _func) -> void{
    m_errorCallback.addListener(0,_func);
}

auto ClientNetConfigManager::addHandler(ClientNetConfigManager::Commands event,std::function<void()> _func) -> void{
    m_callbacks.addListener(static_cast<int>(event),_func);
}

auto ClientNetConfigManager::receiveCommand(uint32_t command) -> void{
    CNetConfigManager::Commands c = static_cast<CNetConfigManager::Commands>(command);
    if (c == CNetConfigManager::Commands::BEGIN_SEND_SETTING){
        m_currentState = States::GET_DATA;
        reset();
    }
    if (c == CNetConfigManager::Commands::END_SEND_SETTING){
        m_currentState = States::NORMAL;
        if (isSetted()){
            m_pNetConfManager->sendData(CNetConfigManager::Commands::SETTING_GET_SUCCES);
            m_callbacks.emitEvent(static_cast<int>(Commands::GET_NEW_SETTING));
        }else{
            reset();
            m_pNetConfManager->sendData(CNetConfigManager::Commands::SETTING_GET_FAIL);
        }
    }

    if (c == CNetConfigManager::Commands::START_STREAMING){
        m_callbacks.emitEvent(static_cast<int>(Commands::START_STREAMING));
    }

    if (c == CNetConfigManager::Commands::STOP_STREAMING){
        m_callbacks.emitEvent(static_cast<int>(Commands::STOP_STREAMING));
    }

    if (c == CNetConfigManager::Commands::LOAD_SETTING_FROM_FILE){
        if (readFromFile(m_file_settings)){
            m_pNetConfManager->sendData(CNetConfigManager::Commands::LOAD_FROM_FILE_SUCCES);
            m_callbacks.emitEvent(static_cast<int>(Commands::GET_NEW_SETTING));
        }else{
            m_pNetConfManager->sendData(CNetConfigManager::Commands::LOAD_FROM_FILE_FAIL);
        }
    }

    if (c == CNetConfigManager::Commands::SAVE_SETTING_TO_FILE){
        if (writeToFile(m_file_settings)){
            m_pNetConfManager->sendData(CNetConfigManager::Commands::SAVE_TO_FILE_SUCCES);
        }else{
            m_pNetConfManager->sendData(CNetConfigManager::Commands::SAVE_TO_FILE_FAIL);
        }
    }
}

auto ClientNetConfigManager::receiveValueStr(std::string key,std::string value) -> void{
    if (m_currentState == States::GET_DATA){
        if (setValue(key,value)){
            m_errorCallback.emitEvent(0,Errors::CANNT_SET_DATA);
        }
    }
}

auto ClientNetConfigManager::receiveValueInt(std::string key,uint32_t value) -> void{
    if (m_currentState == States::GET_DATA){
        if (setValue(key,value)){
            m_errorCallback.emitEvent(0,Errors::CANNT_SET_DATA);
        }
    }
}

auto ClientNetConfigManager::receiveValueDouble(std::string key,double value) -> void{
    if (m_currentState == States::GET_DATA){
        if (setValue(key,value)){
            m_errorCallback.emitEvent(0,Errors::CANNT_SET_DATA);
        }
    }
}


auto ClientNetConfigManager::serverError(std::error_code error) -> void{
    fprintf(stderr,"[ServerNetConfigManager] serverError: %s (%d)\n",error.message().c_str(),error.value());
    m_errorCallback.emitEvent(0,Errors::SERVER_INTERNAL);
    if (m_currentState == States::GET_DATA){
        reset();
        m_currentState = States::NORMAL;
        m_errorCallback.emitEvent(0,Errors::BREAK_RECEIVE_SETTINGS);
    }
}

auto ClientNetConfigManager::sendServerStarted() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::Commands::SERVER_STARTED);
}

auto ClientNetConfigManager::sendServerStopped() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::Commands::SERVER_STOPPED);
}
