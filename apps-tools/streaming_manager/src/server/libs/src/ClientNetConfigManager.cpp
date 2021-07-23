#include <mutex>
#include "ClientNetConfigManager.h"
// IN seconds
#define BROADCAST_TIMEOUT 10

std::mutex g_broadcast_mutex;
std::mutex g_client_mutex;

ClientNetConfigManager::ClientNetConfigManager(std::string default_file_settings_path):CStreamSettings(),
    m_pBroadcast(nullptr),
    m_file_settings(default_file_settings_path),
    m_broadcastClients()
{
    readFromFile(default_file_settings_path);
}

ClientNetConfigManager::~ClientNetConfigManager(){

}

auto ClientNetConfigManager::startBroadcast(std::string host,std::string port) -> void{
    m_pBroadcast = asionet_broadcast::CAsioBroadcastSocket::Create(host,port);
    m_pBroadcast->InitClient();
    m_pBroadcast->addHandler(asionet_broadcast::CAsioBroadcastSocket::ABEvents::AB_ERROR,[this](std::error_code er){
        fprintf(stderr,"[ServerNetConfigManager] Broadcast client error: %s (%d)\n",er.message().c_str(),er.value());
        m_errorCallback.emitEvent(0,Errors::BROADCAST_ERROR);
    });

    m_pBroadcast->addHandler(asionet_broadcast::CAsioBroadcastSocket::ABEvents::AB_RECIVED_DATA,[this](std::error_code er,uint8_t* buf,size_t size){
        bool riseEmit = false;
        if (!er) {
            const std::lock_guard<std::mutex> lock(g_broadcast_mutex);
            BroadCastClients cl;
            cl.mode = asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_NONE;
            cl.ts = std::time(0);
            std::string s = std::string((char *) buf, size);
            if (s[s.size()-1] == 'M'){
                cl.mode = asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_MASTER;
            }else
            if (s[s.size()-1] == 'S'){
                cl.mode = asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_SLAVE;
            }else{
                m_errorCallback.emitEvent(0,Errors::BROADCAST_ERROR_PARSE);
            }
            s.pop_back();
            cl.host = s;
            auto find = std::find_if(std::begin(m_broadcastClients),std::end(m_broadcastClients),[&cl](const BroadCastClients &c){
                return c.host == cl.host;
            });
            if (find == std::end(m_broadcastClients)){
                m_broadcastClients.push_back(cl);
                riseEmit = true;
            }else{
                find->ts = std::time(0);
            }
            m_broadcastClients.remove_if([](const BroadCastClients &c){
                         return (std::time(0) - c.ts > BROADCAST_TIMEOUT);
            });
        }else{
            fprintf(stderr,"[ServerNetConfigManager] Broadcast client error: %s (%d)\n",er.message().c_str(),er.value());
            m_errorCallback.emitEvent(0,Errors::BROADCAST_ERROR);
        }

        if (riseEmit) m_callbacks.emitEvent((int)Commands::BROADCAST_NEW_CLIENT);
    });
}

auto ClientNetConfigManager::getBroadcastClients() -> const std::list<ClientNetConfigManager::BroadCastClients>{
    const std::lock_guard<std::mutex> lock(g_broadcast_mutex);
    m_broadcastClients.remove_if([](const BroadCastClients &c){
        return (std::time(0) - c.ts > BROADCAST_TIMEOUT);
    });
    auto copyClients = m_broadcastClients;
    return copyClients;
}


auto ClientNetConfigManager::addHandlerError(std::function<void(ClientNetConfigManager::Errors)> _func) -> void{
    m_errorCallback.addListener(0,_func);
}

auto ClientNetConfigManager::addHandler(ClientNetConfigManager::Commands event,std::function<void()> _func) -> void{
    m_callbacks.addListener(static_cast<int>(event),_func);
}

auto ClientNetConfigManager::connectToServers(std::vector<std::string> _hosts,std::string port) -> void{
    m_clients.clear();
    for(std::string host:_hosts){
        Clients cl;
        cl.m_manager = std::make_shared<CNetConfigManager>();
        cl.m_mode = asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_NONE;
        cl.m_manager->addHandlerReceiveCommand(std::bind(&ClientNetConfigManager::receiveCommand, this, std::placeholders::_1,cl.m_manager));
        cl.m_manager->addHandlerReceiveStrStr(std::bind(&ClientNetConfigManager::receiveValueStr, this, std::placeholders::_1, std::placeholders::_2,cl.m_manager));
        cl.m_manager->addHandlerReceiveStrInt(std::bind(&ClientNetConfigManager::receiveValueInt, this, std::placeholders::_1, std::placeholders::_2,cl.m_manager));
        cl.m_manager->addHandlerReceiveStrDouble(std::bind(&ClientNetConfigManager::receiveValueDouble, this, std::placeholders::_1, std::placeholders::_2,cl.m_manager));

        cl.m_manager->addHandlerError(std::bind(&ClientNetConfigManager::serverError, this, std::placeholders::_1,cl.m_manager));
        cl.m_manager->startAsioNet(asionet_simple::CAsioSocketSimple::ASMode::AS_CLIENT,host,port);
        m_clients.push_back(cl);
    }
}

auto ClientNetConfigManager::receiveCommand(uint32_t command,std::shared_ptr<CNetConfigManager> sender) -> void{
    CNetConfigManager::Commands c = static_cast<CNetConfigManager::Commands>(command);

    if (c == CNetConfigManager::Commands::MASTER_CONNETED){
        g_client_mutex.lock();
        auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&sender](const Clients& c){
            return c.m_manager == sender;
        });
        if (it != std::end(m_clients)){
            it->m_mode = asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_MASTER;
        }
        g_client_mutex.unlock();
        m_callbacks.emitEvent(static_cast<int>(Commands::SERVER_CONNECTED));
    }
    if (c == CNetConfigManager::Commands::SLAVE_CONNECTED){
        g_client_mutex.lock();
        auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&sender](const Clients& c){
            return c.m_manager == sender;
        });
        if (it != std::end(m_clients)){
            it->m_mode = asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_SLAVE;
        }
        g_client_mutex.unlock();
        m_callbacks.emitEvent(static_cast<int>(Commands::SERVER_CONNECTED));
    }

//    if (c == CNetConfigManager::Commands::END_SEND_SETTING){
//        m_currentState = States::NORMAL;
//        if (isSetted()){
//            m_pNetConfManager->sendData(CNetConfigManager::Commands::SETTING_GET_SUCCES);
//            m_callbacks.emitEvent(static_cast<int>(Commands::GET_NEW_SETTING));
//        }else{
//            reset();
//            m_pNetConfManager->sendData(CNetConfigManager::Commands::SETTING_GET_FAIL);
//        }
//    }
//
//    if (c == CNetConfigManager::Commands::START_STREAMING){
//        m_callbacks.emitEvent(static_cast<int>(Commands::START_STREAMING));
//    }
//
//    if (c == CNetConfigManager::Commands::STOP_STREAMING){
//        m_callbacks.emitEvent(static_cast<int>(Commands::STOP_STREAMING));
//    }
//
//    if (c == CNetConfigManager::Commands::LOAD_SETTING_FROM_FILE){
//        if (readFromFile(m_file_settings)){
//            m_pNetConfManager->sendData(CNetConfigManager::Commands::LOAD_FROM_FILE_SUCCES);
//            m_callbacks.emitEvent(static_cast<int>(Commands::GET_NEW_SETTING));
//        }else{
//            m_pNetConfManager->sendData(CNetConfigManager::Commands::LOAD_FROM_FILE_FAIL);
//        }
//    }
//
//    if (c == CNetConfigManager::Commands::SAVE_SETTING_TO_FILE){
//        if (writeToFile(m_file_settings)){
//            m_pNetConfManager->sendData(CNetConfigManager::Commands::SAVE_TO_FILE_SUCCES);
//        }else{
//            m_pNetConfManager->sendData(CNetConfigManager::Commands::SAVE_TO_FILE_FAIL);
//        }
//    }
}

auto ClientNetConfigManager::isServersConnected() -> bool{
    bool ret = true;
    g_client_mutex.lock();
    for(auto &c : m_clients){
        if (c.m_mode == asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_NONE){
            ret = false;
            break;
        }
    }
    g_client_mutex.unlock();
    return ret;
}

auto ClientNetConfigManager::receiveValueStr(std::string key,std::string value,std::shared_ptr<CNetConfigManager> sender) -> void{
//    if (m_currentState == States::GET_DATA){
//        if (setValue(key,value)){
//            m_errorCallback.emitEvent(0,Errors::CANNT_SET_DATA);
//        }
//    }
}

auto ClientNetConfigManager::receiveValueInt(std::string key,uint32_t value,std::shared_ptr<CNetConfigManager> sender) -> void{
//    if (m_currentState == States::GET_DATA){
//        if (setValue(key,value)){
//            m_errorCallback.emitEvent(0,Errors::CANNT_SET_DATA);
//        }
//    }
}

auto ClientNetConfigManager::receiveValueDouble(std::string key,double value,std::shared_ptr<CNetConfigManager> sender) -> void{
//    if (m_currentState == States::GET_DATA){
//        if (setValue(key,value)){
//            m_errorCallback.emitEvent(0,Errors::CANNT_SET_DATA);
//        }
//    }
}


auto ClientNetConfigManager::serverError(std::error_code error,std::shared_ptr<CNetConfigManager> sender) -> void{
//    fprintf(stderr,"[ServerNetConfigManager] serverError: %s (%d)\n",error.message().c_str(),error.value());
//    m_errorCallback.emitEvent(0,Errors::SERVER_INTERNAL);
//    if (m_currentState == States::GET_DATA){
//        reset();
//        m_currentState = States::NORMAL;
//        m_errorCallback.emitEvent(0,Errors::BREAK_RECEIVE_SETTINGS);
//    }
}