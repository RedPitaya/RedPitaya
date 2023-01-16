#include <mutex>
#include <algorithm>
#include "client_net_config_manager.h"
#include "data_lib/thread_cout.h"

// IN seconds
#define BROADCAST_TIMEOUT 10

std::mutex g_broadcast_mutex;
std::mutex g_client_mutex;

ClientNetConfigManager::ClientNetConfigManager(std::string default_file_settings_path,bool loadConfig):CStreamSettings(),
    m_pBroadcast(nullptr),
    m_file_settings(default_file_settings_path),
    m_broadcastClients()
{
    if (loadConfig)
        readFromFile(default_file_settings_path);
}

ClientNetConfigManager::~ClientNetConfigManager(){
    removeHadlers();
    for(auto &cli:m_clients){
        cli->m_manager->stopAsioNet();
    }
}

auto ClientNetConfigManager::removeHadlers() -> void{
    broadCastNewClientNofiy.disconnect_all();
    serverConnectedNofiy.disconnect_all();
    getNewSettingsNofiy.disconnect_all();
    successSendConfigNofiy.disconnect_all();
    failSendConfigNofiy.disconnect_all();
    successSaveConfigNofiy.disconnect_all();
    failSaveConfigNofiy.disconnect_all();

    serverStartedTCPNofiy.disconnect_all();
    serverStartedUDPNofiy.disconnect_all();
    serverStartedSDNofiy.disconnect_all();
    serverStoppedNofiy.disconnect_all();
    serverStoppedSDFullNofiy.disconnect_all();
    serverStoppedSDDoneNofiy.disconnect_all();

    serverDacStartedNofiy.disconnect_all();
    serverDacStartedSDNofiy.disconnect_all();
    serverDacStoppedNofiy.disconnect_all();
    serverDacStoppedSDDoneNofiy.disconnect_all();
    serverDacStoppedSDEmptyNofiy.disconnect_all();
    serverDacStoppedSDBrokenNofiy.disconnect_all();
    serverDacStoppedSDMissingNofiy.disconnect_all();

    serverLoopbackStartedNofiy.disconnect_all();
    serverLoopbackStoppedNofiy.disconnect_all();
    serverLoopbackBusyNofiy.disconnect_all();
    serverLoopbackCopySettingsDoneNofiy.disconnect_all();

    serverModeTCPNofiy.disconnect_all();
    serverModeUDPNofiy.disconnect_all();
    serverModeSDNofiy.disconnect_all();

    startADCDoneNofiy.disconnect_all();
    startDACDoneNofiy.disconnect_all();

    errorNofiy.disconnect_all();
}

auto ClientNetConfigManager::startBroadcast(std::string host,std::string port) -> void{
    m_pBroadcast = broadcast_lib::CAsioBroadcastSocket::create(broadcast_lib::CLIENT,host,port);
    m_pBroadcast->initClient();
    m_pBroadcast->errorNotify.connect([=](std::error_code er){
        aprintf(stderr,"[ServerNetConfigManager] Broadcast client error: %s (%d)\n",er.message().c_str(),er.value());
        errorNofiy(Errors::BROADCAST_ERROR,host,er);
    });

    m_pBroadcast->receivedNotify.connect([this,host](std::error_code er,uint8_t* buf,size_t size){
        bool riseEmit = false;
        std::string h = "";
        if (!er) {
            const std::lock_guard<std::mutex> lock(g_broadcast_mutex);
            BroadCastClients cl;
            cl.mode = broadcast_lib::AB_NONE;
            cl.ts = std::time(0);
            std::string s = std::string((char *) buf, size);
            uint8_t model =  s[s.size()-1] - '0';
            cl.model = static_cast<broadcast_lib::EModel>(model);
            s.pop_back();
            if (s[s.size()-1] == 'M'){
                cl.mode = broadcast_lib::AB_SERVER_MASTER;
            }else
            if (s[s.size()-1] == 'S'){
                cl.mode = broadcast_lib::AB_SERVER_SLAVE;
            }else{
                errorNofiy(Errors::BROADCAST_ERROR_PARSE,host,er);
            }
            s.pop_back();
            cl.host = h = s;
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
            aprintf(stderr,"[ServerNetConfigManager] Broadcast client error: %s (%d)\n",er.message().c_str(),er.value());
            errorNofiy(Errors::BROADCAST_ERROR,host,er);
        }

        if (riseEmit) broadCastNewClientNofiy(h);
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

auto ClientNetConfigManager::getHosts() -> std::list<std::string>{
    std::list<std::string> list;
    for(auto &c : m_clients){
        if (c->m_manager->isConnected()){
            list.push_back(c->m_manager->getHost());
        }
    }
    return list;
}


auto ClientNetConfigManager::connectToServers(std::vector<std::string> _hosts,std::string port) -> void{
    m_clients.clear();
    for(std::string& host:_hosts){
        auto cl = std::make_shared<Clients>();
        cl->m_manager = std::make_shared<CNetConfigManager>();
        cl->m_mode = broadcast_lib::AB_NONE;

        auto cl_weak = std::weak_ptr<Clients>(cl);

        cl->m_manager->receivedCommandNotify.connect(std::bind(&ClientNetConfigManager::receiveCommand, this, std::placeholders::_1,cl_weak));
        cl->m_manager->receivedStringNotify.connect(std::bind(&ClientNetConfigManager::receiveValueStr, this, std::placeholders::_1, std::placeholders::_2,cl_weak));
        cl->m_manager->receivedIntNotify.connect(std::bind(&ClientNetConfigManager::receiveValueInt, this, std::placeholders::_1, std::placeholders::_2,cl_weak));
        cl->m_manager->receivedDoubleNotify.connect(std::bind(&ClientNetConfigManager::receiveValueDouble, this, std::placeholders::_1, std::placeholders::_2,cl_weak));

        cl->m_manager->errorNotify.connect(std::bind(&ClientNetConfigManager::serverError, this, std::placeholders::_1,cl_weak));
        cl->m_manager->connectTimeoutNotify.connect(std::bind(&ClientNetConfigManager::connectTimeoutError, this, std::placeholders::_1,cl_weak));

        cl->m_manager->startAsioNet(net_lib::EMode::M_CLIENT,host,port);
        m_clients.push_back(cl);
    }
}

auto ClientNetConfigManager::diconnectAll() -> void{
    const std::lock_guard<std::mutex> lock(g_client_mutex);
    for(const auto &c : m_clients){
        c->m_manager->stopAsioNet();
    }
}


auto ClientNetConfigManager::receiveCommand(uint32_t command,std::weak_ptr<Clients> cl) -> void{
    auto sender = cl.lock();
    if (!sender) return;
    CNetConfigManager::ECommands c = static_cast<CNetConfigManager::ECommands>(command);
    if (c == CNetConfigManager::ECommands::MASTER_CONNETED){
        g_client_mutex.lock();
        sender->m_mode = broadcast_lib::AB_SERVER_MASTER;
        g_client_mutex.unlock();
        serverConnectedNofiy(sender->m_manager->getHost());
    }
    if (c == CNetConfigManager::ECommands::SLAVE_CONNECTED){
        g_client_mutex.lock();
        sender->m_mode = broadcast_lib::AB_SERVER_SLAVE;
        g_client_mutex.unlock();
        serverConnectedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::BEGIN_SEND_SETTING){

        g_client_mutex.lock();
        sender->m_current_state = Clients::States::GET_DATA;
        sender->m_client_settings.reset();
        g_client_mutex.unlock();
    }

    if (c == CNetConfigManager::ECommands::END_SEND_SETTING){
        if (sender->m_client_settings.isSetted())
            getNewSettingsNofiy(sender->m_manager->getHost());
        g_client_mutex.lock();
        sender->m_current_state = Clients::States::NORMAL;
        if (sender->m_client_settings.isSetted()){
            sender->m_manager->sendData(CNetConfigManager::ECommands::SETTING_GET_SUCCESS);
        }else{
            sender->m_client_settings.reset();
            sender->m_manager->sendData(CNetConfigManager::ECommands::SETTING_GET_FAIL);
        }
        g_client_mutex.unlock();

    }

    if (c == CNetConfigManager::ECommands::SETTING_GET_SUCCESS){
        successSendConfigNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SETTING_GET_FAIL){
        failSendConfigNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SAVE_TO_FILE_SUCCES){
        successSaveConfigNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SAVE_TO_FILE_FAIL){
        failSaveConfigNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_STARTED_TCP){
        serverStartedTCPNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_STARTED_UDP){
        serverStartedUDPNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_STARTED_SD){
        serverStartedSDNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_STOPPED){
        serverStoppedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_STOPPED_SD_DONE){
        serverStoppedSDDoneNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_STOPPED_SD_FULL){
        serverStoppedSDFullNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_DAC_STARTED){
        serverDacStartedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_DAC_STARTED_SD){
        serverDacStartedSDNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_DAC_STOPPED){
        serverDacStoppedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_DAC_STOPPED_SD_DONE){
        serverDacStoppedSDDoneNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_DAC_STOPPED_SD_EMPTY){
        serverDacStoppedSDEmptyNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_DAC_STOPPED_SD_BROKEN){
        serverDacStoppedSDBrokenNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_DAC_STOPPED_SD_MISSING){
        serverDacStoppedSDMissingNofiy(sender->m_manager->getHost());
    }

    // Loopback commands
    if (c == CNetConfigManager::ECommands::SERVER_LOOPBACK_STARTED){
        serverLoopbackStartedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_LOOPBACK_STOPPED){
        serverLoopbackStoppedNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_LOOPBACK_BUSY){
        serverLoopbackBusyNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::COPY_SETTINGS_TO_TEST_SETTINGS_DONE){
        serverLoopbackCopySettingsDoneNofiy(sender->m_manager->getHost());
    }

    // Server mode

    if (c == CNetConfigManager::ECommands::SERVER_MODE_SD){
        serverModeSDNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_MODE_TCP){
        serverModeTCPNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::SERVER_MODE_UDP){
        serverModeUDPNofiy(sender->m_manager->getHost());
    }


    //ADC/DAC control

    if (c == CNetConfigManager::ECommands::START_ADC_DONE){
        startADCDoneNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::START_DAC_DONE){
        startDACDoneNofiy(sender->m_manager->getHost());
    }

    if (c == CNetConfigManager::ECommands::CONFIG_FILE_MISSED){
        configFileMissedNotify(sender->m_manager->getHost());
    }
}

auto ClientNetConfigManager::isServersConnected() -> bool{
    const std::lock_guard<std::mutex> lock(g_client_mutex);
    bool ret = true;
    for(auto &c : m_clients){
        if (c->m_mode == broadcast_lib::AB_NONE && c->m_manager->isConnected()){
            ret = false;
            break;
        }
    }
    return ret;
}

auto ClientNetConfigManager::receiveValueStr(std::string key,std::string value,std::weak_ptr<Clients> cl) -> void{
    auto sender = cl.lock();
    if (!sender) return;
    if (sender->m_current_state == Clients::States::GET_DATA){
        if (!sender->m_client_settings.setValue(key,value)){
            errorNofiy(Errors::CANNT_SET_DATA_TO_CONFIG,sender->m_manager->getHost(),std::error_code());
        }
    }
}

auto ClientNetConfigManager::receiveValueInt(std::string key,uint32_t value,std::weak_ptr<Clients> cl) -> void{
    auto sender = cl.lock();
    if (!sender) return;
    if (sender->m_current_state == Clients::States::GET_DATA){
        if (!sender->m_client_settings.setValue(key,(int64_t)value)){
            errorNofiy(Errors::CANNT_SET_DATA_TO_CONFIG,sender->m_manager->getHost(),std::error_code());
        }
    }
}

auto ClientNetConfigManager::receiveValueDouble(std::string key,double value,std::weak_ptr<Clients> cl) -> void{
    auto sender = cl.lock();
    if (!sender) return;
    if (sender->m_current_state == Clients::States::GET_DATA){
        if (!sender->m_client_settings.setValue(key,value)){
            errorNofiy(Errors::CANNT_SET_DATA_TO_CONFIG,sender->m_manager->getHost(),std::error_code());
        }
    }
}


auto ClientNetConfigManager::serverError(std::error_code error,std::weak_ptr<Clients> cl) -> void{
//    aprintf(stderr,"[ClientNetConfigManager] Server error: %s (%d)\n",error.message().c_str(),error.value());
    auto sender = cl.lock();
    if (!sender) return;
    errorNofiy(Errors::SERVER_INTERNAL,sender-> m_manager->getHost(),error);
    if (sender->m_current_state  == Clients::States::GET_DATA){
        sender->m_client_settings.reset();
        sender->m_current_state = Clients::States::NORMAL;
        errorNofiy(Errors::CANNT_SET_DATA_TO_CONFIG,sender->m_manager->getHost(),error);
    }
}

auto ClientNetConfigManager::connectTimeoutError(std::error_code error,std::weak_ptr<Clients> cl) -> void{
    auto sender = cl.lock();
    if (!sender) return;
//    aprintf(stderr,"[ClientNetConfigManager] Connection timeout error: %s (%d)\n",error.message().c_str(),error.value());
    errorNofiy(Errors::CONNECT_TIMEOUT,sender->m_manager->getHost(),error);
}

auto ClientNetConfigManager::sendConfig(const std::string &host) -> bool{
    const std::lock_guard<std::mutex> lock(g_client_mutex);
    for(const auto &c : m_clients){
        if (c->m_mode != broadcast_lib::AB_NONE && host == c->m_manager->getHost()){
            auto ret =  sendConfig(c,true);
            if (!ret) {
                c->m_manager->stopAsioNet();
            }
            return ret;
        }
    }
    return false;
}

auto ClientNetConfigManager::sendTestConfig(const std::string &host,const CStreamSettings &settings) -> bool{
    const std::lock_guard<std::mutex> lock(g_client_mutex);
    for(const auto &c : m_clients){
        if (c->m_mode != broadcast_lib::AB_NONE && host == c->m_manager->getHost()){
            auto ret =  sendTestConfig(c,true,settings);
            if (!ret) {
                c->m_manager->stopAsioNet();
            }
            return ret;
        }
    }
    return false;
}


auto ClientNetConfigManager::sendConfig(std::shared_ptr<Clients> _client, bool _async) -> bool{
    if (_client->m_manager->isConnected()) {
        if (!_client->m_manager->sendData(CNetConfigManager::ECommands::BEGIN_SEND_SETTING,_async)) return false;
        if (!_client->m_manager->sendData("port",getPort(),_async)) return false;
        if (!_client->m_manager->sendData("protocol",static_cast<uint32_t>(getProtocol()),_async)) return false;
        if (!_client->m_manager->sendData("samples",static_cast<uint32_t>(getSamples()),_async)) return false;
        if (!_client->m_manager->sendData("format",static_cast<uint32_t>(getFormat()),_async)) return false;
        if (!_client->m_manager->sendData("type",static_cast<uint32_t>(getType()),_async)) return false;
        if (!_client->m_manager->sendData("save_type",static_cast<uint32_t>(getSaveType()),_async)) return false;
        if (!_client->m_manager->sendData("channels",static_cast<uint32_t>(getChannels()),_async)) return false;
        if (!_client->m_manager->sendData("resolution",static_cast<uint32_t>(getResolution()),_async)) return false;
        if (!_client->m_manager->sendData("decimation",static_cast<uint32_t>(getDecimation()),_async)) return false;
        if (!_client->m_manager->sendData("attenuator",static_cast<uint32_t>(getAttenuator()),_async)) return false;
        if (!_client->m_manager->sendData("calibration",static_cast<uint32_t>(getCalibration()),_async)) return false;
        if (!_client->m_manager->sendData("coupling",static_cast<uint32_t>(getAC_DC()),_async)) return false;

        if (!_client->m_manager->sendData("dac_file",getDACFile(),_async)) return false;
        if (!_client->m_manager->sendData("dac_port",getDACPort(),_async)) return false;
        if (!_client->m_manager->sendData("dac_file_type",static_cast<uint32_t>(getDACFileType()),_async)) return false;
        if (!_client->m_manager->sendData("dac_gain",static_cast<uint32_t>(getDACGain()),_async)) return false;
        if (!_client->m_manager->sendData("dac_mode",static_cast<uint32_t>(getDACMode()),_async)) return false;
        if (!_client->m_manager->sendData("dac_repeat",static_cast<uint32_t>(getDACRepeat()),_async)) return false;
        if (!_client->m_manager->sendData("dac_repeatCount",static_cast<uint32_t>(getDACRepeatCount()),_async)) return false;
        if (!_client->m_manager->sendData("dac_memoryUsage",static_cast<uint32_t>(getDACMemoryUsage()),_async)) return false;
        if (!_client->m_manager->sendData("dac_speed",static_cast<uint32_t>(getDACHz()),_async)) return false;

        if (!_client->m_manager->sendData("loopback_timeout",static_cast<uint32_t>(getLoopbackTimeout()),_async)) return false;
        if (!_client->m_manager->sendData("loopback_speed",static_cast<uint32_t>(getLoopbackSpeed()),_async)) return false;
        if (!_client->m_manager->sendData("loopback_mode",static_cast<uint32_t>(getLoopbackMode()),_async)) return false;
        if (!_client->m_manager->sendData("loopback_channels",static_cast<uint32_t>(getLoopbackChannels()),_async)) return false;

        if (!_client->m_manager->sendData("board_mode",static_cast<uint32_t>(getBoardMode()),_async)) return false;

        if (!_client->m_manager->sendData(CNetConfigManager::ECommands::END_SEND_SETTING,_async)) return false;
        return true;
    }
    return false;
}

auto ClientNetConfigManager::sendTestConfig(std::shared_ptr<Clients> _client, bool _async,const CStreamSettings &settings) -> bool{
    if (_client->m_manager->isConnected()) {
        if (!_client->m_manager->sendData(CNetConfigManager::ECommands::BEGIN_SEND_TEST_SETTING,_async)) return false;
        if (!_client->m_manager->sendData("port",settings.getPort(),_async)) return false;
        if (!_client->m_manager->sendData("protocol",static_cast<uint32_t>(settings.getProtocol()),_async)) return false;
        if (!_client->m_manager->sendData("samples",static_cast<uint32_t>(settings.getSamples()),_async)) return false;
        if (!_client->m_manager->sendData("format",static_cast<uint32_t>(settings.getFormat()),_async)) return false;
        if (!_client->m_manager->sendData("type",static_cast<uint32_t>(settings.getType()),_async)) return false;
        if (!_client->m_manager->sendData("save_type",static_cast<uint32_t>(settings.getSaveType()),_async)) return false;
        if (!_client->m_manager->sendData("channels",static_cast<uint32_t>(settings.getChannels()),_async)) return false;
        if (!_client->m_manager->sendData("resolution",static_cast<uint32_t>(settings.getResolution()),_async)) return false;
        if (!_client->m_manager->sendData("decimation",static_cast<uint32_t>(settings.getDecimation()),_async)) return false;
        if (!_client->m_manager->sendData("attenuator",static_cast<uint32_t>(settings.getAttenuator()),_async)) return false;
        if (!_client->m_manager->sendData("calibration",static_cast<uint32_t>(settings.getCalibration()),_async)) return false;
        if (!_client->m_manager->sendData("coupling",static_cast<uint32_t>(settings.getAC_DC()),_async)) return false;

        if (!_client->m_manager->sendData("dac_file",settings.getDACFile(),_async)) return false;
        if (!_client->m_manager->sendData("dac_port",settings.getDACPort(),_async)) return false;
        if (!_client->m_manager->sendData("dac_file_type",static_cast<uint32_t>(settings.getDACFileType()),_async)) return false;
        if (!_client->m_manager->sendData("dac_gain",static_cast<uint32_t>(settings.getDACGain()),_async)) return false;
        if (!_client->m_manager->sendData("dac_mode",static_cast<uint32_t>(settings.getDACMode()),_async)) return false;
        if (!_client->m_manager->sendData("dac_repeat",static_cast<uint32_t>(settings.getDACRepeat()),_async)) return false;
        if (!_client->m_manager->sendData("dac_repeatCount",static_cast<uint32_t>(settings.getDACRepeatCount()),_async)) return false;
        if (!_client->m_manager->sendData("dac_memoryUsage",static_cast<uint32_t>(settings.getDACMemoryUsage()),_async)) return false;
        if (!_client->m_manager->sendData("dac_speed",static_cast<uint32_t>(settings.getDACHz()),_async)) return false;

        if (!_client->m_manager->sendData("loopback_timeout",static_cast<uint32_t>(settings.getLoopbackTimeout()),_async)) return false;
        if (!_client->m_manager->sendData("loopback_speed",static_cast<uint32_t>(settings.getLoopbackSpeed()),_async)) return false;
        if (!_client->m_manager->sendData("loopback_mode",static_cast<uint32_t>(settings.getLoopbackMode()),_async)) return false;
        if (!_client->m_manager->sendData("loopback_channels",static_cast<uint32_t>(settings.getLoopbackChannels()),_async)) return false;

        if (!_client->m_manager->sendData("board_mode",static_cast<uint32_t>(settings.getBoardMode()),_async)) return false;

        if (!_client->m_manager->sendData(CNetConfigManager::ECommands::END_SEND_TEST_SETTING,_async)) return false;
        return true;
    }
    return false;
}

auto ClientNetConfigManager::requestConfig(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::REQUEST_SERVER_SETTINGS);
    }
    return false;
}


auto ClientNetConfigManager::sendCopyConfigToTest(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::COPY_SETTINGS_TO_TEST_SETTINGS);
    }
    return false;
}


auto ClientNetConfigManager::requestTestConfig(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::REQUEST_SERVER_TEST_SETTINGS);
    }
    return false;
}

auto ClientNetConfigManager::getLocalSettingsOfHost(const std::string &host) -> CStreamSettings*{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return (CStreamSettings*)&(it->operator->()->m_client_settings);
    }
    return nullptr;
}

auto ClientNetConfigManager::getLocalTestSettingsOfHost(const std::string &host) -> CStreamSettings*{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return (CStreamSettings*)&(it->operator->()->m_testSettings);
    }
    return nullptr;
}

auto ClientNetConfigManager::sendSaveToFile(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::SAVE_SETTING_TO_FILE);
    }
    return false;
}

auto ClientNetConfigManager::sendStart(const std::string &host,bool test_mode) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(test_mode ?  CNetConfigManager::ECommands::START_STREAMING_TEST : CNetConfigManager::ECommands::START_STREAMING);
    }
    return false;
}

auto ClientNetConfigManager::sendStop(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::STOP_STREAMING);
    }
    return false;
}

auto ClientNetConfigManager::sendDACStart(const std::string &host,bool test_mode) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(test_mode ?  CNetConfigManager::ECommands::START_DAC_STREAMING_TEST :CNetConfigManager::ECommands::START_DAC_STREAMING);
    }
    return false;
}

auto ClientNetConfigManager::sendDACStop(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::STOP_DAC_STREAMING);
    }
    return false;
}

auto ClientNetConfigManager::sendStartADC(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::START_ADC);
    }
    return false;
}

auto ClientNetConfigManager::sendStartDAC(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::START_DAC);
    }
    return false;
}


auto ClientNetConfigManager::sendGetServerMode(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::GET_SERVER_MODE);
    }
    return false;
}

auto ClientNetConfigManager::sendGetServerTestMode(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::GET_SERVER_TEST_MODE);
    }
    return false;
}

auto ClientNetConfigManager::getModeByHost(const std::string &host) -> broadcast_lib::EMode{
    const std::lock_guard<std::mutex> lock(g_client_mutex);
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_mode;
    }
    return broadcast_lib::AB_NONE;
}


auto ClientNetConfigManager::sendLoopbackStart(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::SERVER_LOOPBACK_START);
    }
    return false;
}

auto ClientNetConfigManager::sendLoopbackStop(const std::string &host) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        return it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::SERVER_LOOPBACK_STOP);
    }
    return false;
}

auto ClientNetConfigManager::sendLoopbackDACSpeed(const std::string &host,int32_t value,bool _async) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        if (!it->operator->()->m_manager->sendData("loopback_speed",static_cast<uint32_t>(value),_async)) return false;
        if (!it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::END_SEND_SETTING,_async)) return false;
        return true;
    }
    return false;
}

auto ClientNetConfigManager::sendLoopbackMode(const std::string &host,int32_t value,bool _async) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        if (!it->operator->()->m_manager->sendData("loopback_mode",static_cast<uint32_t>(value),_async)) return false;
        if (!it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::END_SEND_SETTING,_async)) return false;
        return true;
    }
    return false;
}

auto ClientNetConfigManager::sendLoopbackChannels(const std::string &host,int32_t value,bool _async) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        if (!it->operator->()->m_manager->sendData("loopback_channels",static_cast<uint32_t>(value),_async)) return false;
        if (!it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::END_SEND_SETTING,_async)) return false;
        return true;
    }
    return false;
}

auto ClientNetConfigManager::sendLoopbackTimeout(const std::string &host,int32_t value,bool _async) -> bool{
    auto it = std::find_if(std::begin(m_clients),std::end(m_clients),[&host](const std::shared_ptr<Clients> c){
        return c->m_manager->getHost()  == host;
    });
    if (it != std::end(m_clients)){
        if (!it->operator->()->m_manager->sendData("loopback_timeout",static_cast<uint32_t>(value),_async)) return false;
        if (!it->operator->()->m_manager->sendData(CNetConfigManager::ECommands::END_SEND_SETTING,_async)) return false;
        return true;
    }
    return false;
}
