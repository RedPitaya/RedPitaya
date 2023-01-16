#include "server_net_config_manager.h"
#include "data_lib/thread_cout.h"

#define UNUSED(x) [&x]{}()

ServerNetConfigManager::ServerNetConfigManager(std::string defualt_file_settings_path,broadcast_lib::EMode mode,std::string host,std::string port):
    m_pBroadcast(nullptr),
    m_pNetConfManager(nullptr),
    m_currentState(States::NORMAL),
    m_file_settings(defualt_file_settings_path),
    m_mode(mode)
{
    m_pNetConfManager = std::make_shared<CNetConfigManager>();
    m_pNetConfManager->receivedCommandNotify.connect(&ServerNetConfigManager::receiveCommand,this);
    m_pNetConfManager->receivedStringNotify.connect(&ServerNetConfigManager::receiveValueStr,this);
    m_pNetConfManager->receivedIntNotify.connect(&ServerNetConfigManager::receiveValueInt,this);
    m_pNetConfManager->receivedDoubleNotify.connect(&ServerNetConfigManager::receiveValueDouble,this);
    m_pNetConfManager->connectNotify.connect(&ServerNetConfigManager::connected,this);
    m_pNetConfManager->disconnectNotify.connect(&ServerNetConfigManager::disconnected,this);
    m_pNetConfManager->errorNotify.connect(&ServerNetConfigManager::serverError,this);

    startServer(host,port);
    m_settings.readFromFile(defualt_file_settings_path);
}

ServerNetConfigManager::~ServerNetConfigManager(){
    stop();
}

auto ServerNetConfigManager::stop() -> void {
    if (m_pBroadcast)
        m_pBroadcast->closeSocket();
    m_pNetConfManager->stopAsioNet();
}

auto ServerNetConfigManager::startServer(std::string host,std::string port) -> void {
    m_pNetConfManager->startAsioNet(net_lib::M_SERVER,host,port);
}


auto ServerNetConfigManager::startBroadcast(broadcast_lib::EModel model,std::string host,std::string port) -> void {
    m_pBroadcast = broadcast_lib::CAsioBroadcastSocket::create(model,host,port);
    m_pBroadcast->initServer(m_mode);
    m_pBroadcast->errorNotify.connect([this](std::error_code er){
        aprintf(stderr,"[ServerNetConfigManager] Broadcast server error: %s (%d)\n",er.message().c_str(),er.value());
        errorNofiy(Errors::BROADCAST_ERROR);
    });
}

auto ServerNetConfigManager::isConnected() -> bool{
    return m_pNetConfManager->isConnected();
}

auto ServerNetConfigManager::connected(std::string) -> void {
//    aprintf(stderr,"[DEBUG:connected] %s\n",host.c_str());
    m_currentState = States::NORMAL;
    clientConnectedNofiy();
    if (m_mode == broadcast_lib::AB_SERVER_MASTER)
        m_pNetConfManager->sendData(CNetConfigManager::ECommands::MASTER_CONNETED);
    if (m_mode == broadcast_lib::AB_SERVER_SLAVE)
        m_pNetConfManager->sendData(CNetConfigManager::ECommands::SLAVE_CONNECTED);
}

auto ServerNetConfigManager::disconnected(std::string) -> void{
//    aprintf(stderr,"[DEBUG:disconnected] %s\n",host.c_str());
    clientDisconnectedNofiy();
}

auto ServerNetConfigManager::receiveCommand(uint32_t command) -> void{
//    aprintf(stderr,"[DEBUG:receiveCommand] %d\n",command);

    auto c = static_cast<CNetConfigManager::ECommands>(command);
    if (c == CNetConfigManager::ECommands::BEGIN_SEND_SETTING){
        m_currentState = States::GET_DATA;
        m_settings.reset();
    }

    if (c == CNetConfigManager::ECommands::END_SEND_SETTING){
        m_currentState = States::NORMAL;
        if (m_settings.isSetted()){
            getNewSettingsNofiy();
            m_pNetConfManager->sendData(CNetConfigManager::ECommands::SETTING_GET_SUCCESS);
        }else{
            m_settings.reset();
            m_pNetConfManager->sendData(CNetConfigManager::ECommands::SETTING_GET_FAIL);
        }
    }

    if (c == CNetConfigManager::ECommands::BEGIN_SEND_TEST_SETTING){
        m_currentState = States::GET_TEMP_DATA;
        m_testSettings.reset();
    }

    if (c == CNetConfigManager::ECommands::END_SEND_TEST_SETTING){
        m_currentState = States::NORMAL;
        if (m_testSettings.isSetted()){
            getNewTestSettingsNofiy();
            m_pNetConfManager->sendData(CNetConfigManager::ECommands::TEST_SETTING_GET_SUCCESS);
        }else{
            m_testSettings.reset();
            m_pNetConfManager->sendData(CNetConfigManager::ECommands::TEST_SETTING_GET_FAIL);
        }
    }

    if (c == CNetConfigManager::ECommands::START_ADC){
        startADCNofiy();
    }

    if (c == CNetConfigManager::ECommands::START_DAC){
        startDACNofiy();
    }

    if (c == CNetConfigManager::ECommands::START_STREAMING){
        startStreamingNofiy();
    }

    if (c == CNetConfigManager::ECommands::START_STREAMING_TEST){
        startStreamingTestNofiy();
    }

    if (c == CNetConfigManager::ECommands::STOP_STREAMING){
        stopStreamingNofiy();
    }

    if (c == CNetConfigManager::ECommands::START_DAC_STREAMING){
        startDacStreamingNofiy();
    }

    if (c == CNetConfigManager::ECommands::START_DAC_STREAMING_TEST){
        startDacStreamingTestNofiy();
    }

    if (c == CNetConfigManager::ECommands::STOP_DAC_STREAMING){
        stopDacStreamingNofiy();
    }

    if (c == CNetConfigManager::ECommands::COPY_SETTINGS_TO_TEST_SETTINGS){
        m_testSettings = m_settings;
        m_pNetConfManager->sendData(CNetConfigManager::ECommands::COPY_SETTINGS_TO_TEST_SETTINGS_DONE);
    }

    if (c == CNetConfigManager::ECommands::GET_SERVER_MODE){
        if (m_settings.getSaveType() == CStreamSettings::FILE){
            sendServerModeSD();
        }else if (m_settings.getSaveType() == CStreamSettings::NET){
            if (m_settings.getProtocol() == CStreamSettings::TCP){
                sendServerModeTCP();
            }else if (m_settings.getProtocol() == CStreamSettings::TCP){
                sendServerModeUDP();
            }else{
                aprintf(stderr,"[ServerNetConfigManager] Error in GET_SERVER_MODE. Unknown settings\n");
            }
        }
    }

    if (c == CNetConfigManager::ECommands::GET_SERVER_TEST_MODE){
        if (m_testSettings.getSaveType() == CStreamSettings::FILE){
            sendServerModeSD();
        }else if (m_testSettings.getSaveType() == CStreamSettings::NET){
            if (m_testSettings.getProtocol() == CStreamSettings::TCP){
                sendServerModeTCP();
            }else if (m_testSettings.getProtocol() == CStreamSettings::TCP){
                sendServerModeUDP();
            }else{
                aprintf(stderr,"[ServerNetConfigManager] Error in GET_SERVER_MODE. Unknown settings\n");
            }
        }
    }

    if (c == CNetConfigManager::ECommands::LOAD_SETTING_FROM_FILE){
        if (m_settings.readFromFile(m_file_settings)){
            getNewSettingsNofiy();
            m_pNetConfManager->sendData(CNetConfigManager::ECommands::LOAD_FROM_FILE_SUCCES);
        }else{
            m_pNetConfManager->sendData(CNetConfigManager::ECommands::LOAD_FROM_FILE_FAIL);
        }
    }

    if (c == CNetConfigManager::ECommands::SAVE_SETTING_TO_FILE){
        if (m_settings.writeToFile(m_file_settings)){
            m_pNetConfManager->sendData(CNetConfigManager::ECommands::SAVE_TO_FILE_SUCCES);
        }else{
            m_pNetConfManager->sendData(CNetConfigManager::ECommands::SAVE_TO_FILE_FAIL);
        }
    }

    if (c == CNetConfigManager::ECommands::REQUEST_SERVER_SETTINGS){
        sendConfig(false,false);
    }

    if (c == CNetConfigManager::ECommands::REQUEST_SERVER_TEST_SETTINGS){
        sendConfig(true,false);
    }

    // Loopback commands

    if (c == CNetConfigManager::ECommands::SERVER_LOOPBACK_START){
        startLoopbackModeNofiy();
    }

    if (c == CNetConfigManager::ECommands::SERVER_LOOPBACK_STOP){
        stopLoopbackModeNofiy();
    }

    // Server mode

    if (c== CNetConfigManager::ECommands::GET_SERVER_MODE){
        getServerModeNofiy();
    }

    if (c== CNetConfigManager::ECommands::GET_SERVER_TEST_MODE){
        getServerModeTestNofiy();
    }
}

auto ServerNetConfigManager::receiveValueStr(std::string key,std::string value) -> void {
    if (m_currentState == States::GET_DATA){
        if (!m_settings.setValue(key,value)){
            errorNofiy(Errors::CANNT_SET_DATA);
        }
    }

    if (m_currentState == States::GET_TEMP_DATA){
        if (!m_testSettings.setValue(key,value)){
            errorNofiy(Errors::CANNT_SET_DATA);
        }
    }
}

auto ServerNetConfigManager::receiveValueInt(std::string key,uint32_t value) -> void {
    if (m_currentState == States::GET_DATA){
        if (!m_settings.setValue(key,(int64_t)value)){
            errorNofiy(Errors::CANNT_SET_DATA);
        }
    }

    if (m_currentState == States::GET_TEMP_DATA){
        if (!m_testSettings.setValue(key,(int64_t)value)){
            errorNofiy(Errors::CANNT_SET_DATA);
        }
    }
}

auto ServerNetConfigManager::receiveValueDouble(std::string key,double value) -> void {
    if (m_currentState == States::GET_DATA){
        if (!m_settings.setValue(key,value)){
            errorNofiy(Errors::CANNT_SET_DATA);
        }
    }

    if (m_currentState == States::GET_TEMP_DATA){
        if (!m_testSettings.setValue(key,value)){
            errorNofiy(Errors::CANNT_SET_DATA);
        }
    }
}


auto ServerNetConfigManager::serverError(std::error_code) -> void {
    //aprintf(stderr,"[ServerNetConfigManager] serverError: %s (%d)\n",error.message().c_str(),error.value());
    errorNofiy(Errors::SERVER_INTERNAL);
    if (m_currentState == States::GET_DATA){
        m_settings.reset();
        m_currentState = States::NORMAL;
        errorNofiy(Errors::BREAK_RECEIVE_SETTINGS);
    }

    if (m_currentState == States::GET_TEMP_DATA){
        m_testSettings.reset();
        m_currentState = States::NORMAL;
        errorNofiy(Errors::BREAK_RECEIVE_SETTINGS);
    }
}

auto ServerNetConfigManager::sendConfigFileMissed() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::CONFIG_FILE_MISSED);
}

auto ServerNetConfigManager::sendADCStarted() -> bool {
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::START_ADC_DONE);
}

auto ServerNetConfigManager::sendDACStarted() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::START_DAC_DONE);
}

auto ServerNetConfigManager::sendServerStartedTCP() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_STARTED_TCP);
}

auto ServerNetConfigManager::sendServerStartedUDP() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_STARTED_UDP);
}

auto ServerNetConfigManager::sendServerStartedSD() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_STARTED_SD);
}

auto ServerNetConfigManager::sendServerModeTCP() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_MODE_TCP);
}

auto ServerNetConfigManager::sendServerModeUDP() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_MODE_UDP);
}

auto ServerNetConfigManager::sendServerModeSD() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_MODE_SD);
}

auto ServerNetConfigManager::sendDACServerStarted() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_DAC_STARTED);
}

auto ServerNetConfigManager::sendDACServerStartedSD() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_DAC_STARTED_SD);
}

auto ServerNetConfigManager::sendServerStopped() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_STOPPED);
}

auto ServerNetConfigManager::sendServerStoppedSDFull() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_STOPPED_SD_FULL);
}

auto ServerNetConfigManager::sendServerStoppedDone() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_STOPPED_SD_DONE);
}

auto ServerNetConfigManager::sendDACServerStopped() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_DAC_STOPPED);
}

auto ServerNetConfigManager::sendDACServerStoppedSDDone() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_DAC_STOPPED_SD_DONE);
}

auto ServerNetConfigManager::sendDACServerStoppedSDEmpty() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_DAC_STOPPED_SD_EMPTY);
}

auto ServerNetConfigManager::sendDACServerStoppedSDBroken() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_DAC_STOPPED_SD_BROKEN);
}

auto ServerNetConfigManager::sendDACServerStoppedSDMissingFile() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_DAC_STOPPED_SD_MISSING);
}

auto ServerNetConfigManager::sendServerStartedLoopBackMode() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_LOOPBACK_STARTED);
}

auto ServerNetConfigManager::sendServerStoppedLoopBackMode() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_LOOPBACK_STOPPED);
}

auto ServerNetConfigManager::sendStreamServerBusy() -> bool{
    return m_pNetConfManager->sendData(CNetConfigManager::ECommands::SERVER_LOOPBACK_BUSY);
}

auto ServerNetConfigManager::sendConfig(bool sendTest,bool _async) -> bool{
    if (m_pNetConfManager->isConnected()) {
        CStreamSettings s = sendTest ? m_testSettings : m_settings;

        if (!m_pNetConfManager->sendData(sendTest ? CNetConfigManager::ECommands::BEGIN_SEND_TEST_SETTING : CNetConfigManager::ECommands::BEGIN_SEND_SETTING,_async)) return false;
        if (!m_pNetConfManager->sendData("port",s.getPort(),_async)) return false;
        if (!m_pNetConfManager->sendData("protocol",static_cast<uint32_t>(s.getProtocol()),_async)) return false;
        if (!m_pNetConfManager->sendData("samples",static_cast<uint32_t>(s.getSamples()),_async)) return false;
        if (!m_pNetConfManager->sendData("format",static_cast<uint32_t>(s.getFormat()),_async)) return false;
        if (!m_pNetConfManager->sendData("type",static_cast<uint32_t>(s.getType()),_async)) return false;
        if (!m_pNetConfManager->sendData("save_type",static_cast<uint32_t>(s.getSaveType()),_async)) return false;
        if (!m_pNetConfManager->sendData("channels",static_cast<uint32_t>(s.getChannels()),_async)) return false;
        if (!m_pNetConfManager->sendData("resolution",static_cast<uint32_t>(s.getResolution()),_async)) return false;
        if (!m_pNetConfManager->sendData("decimation",static_cast<uint32_t>(s.getDecimation()),_async)) return false;
        if (!m_pNetConfManager->sendData("attenuator",static_cast<uint32_t>(s.getAttenuator()),_async)) return false;
        if (!m_pNetConfManager->sendData("calibration",static_cast<uint32_t>(s.getCalibration()),_async)) return false;
        if (!m_pNetConfManager->sendData("coupling",static_cast<uint32_t>(s.getAC_DC()),_async)) return false;

        if (!m_pNetConfManager->sendData("dac_file",s.getDACFile(),_async)) return false;
        if (!m_pNetConfManager->sendData("dac_port",s.getDACPort(),_async)) return false;
        if (!m_pNetConfManager->sendData("dac_file_type",static_cast<uint32_t>(s.getDACFileType()),_async)) return false;
        if (!m_pNetConfManager->sendData("dac_gain",static_cast<uint32_t>(s.getDACGain()),_async)) return false;
        if (!m_pNetConfManager->sendData("dac_mode",static_cast<uint32_t>(s.getDACMode()),_async)) return false;
        if (!m_pNetConfManager->sendData("dac_repeat",static_cast<uint32_t>(s.getDACRepeat()),_async)) return false;
        if (!m_pNetConfManager->sendData("dac_repeatCount",static_cast<uint32_t>(s.getDACRepeatCount()),_async)) return false;
        if (!m_pNetConfManager->sendData("dac_memoryUsage",static_cast<uint32_t>(s.getDACMemoryUsage()),_async)) return false;
        if (!m_pNetConfManager->sendData("dac_speed",static_cast<uint32_t>(s.getDACHz()),_async)) return false;

        if (!m_pNetConfManager->sendData("loopback_timeout",static_cast<uint32_t>(s.getLoopbackTimeout()),_async)) return false;
        if (!m_pNetConfManager->sendData("loopback_speed",static_cast<uint32_t>(s.getLoopbackSpeed()),_async)) return false;
        if (!m_pNetConfManager->sendData("loopback_mode",static_cast<uint32_t>(s.getLoopbackMode()),_async)) return false;
        if (!m_pNetConfManager->sendData("loopback_channels",static_cast<uint32_t>(s.getLoopbackChannels()),_async)) return false;

        if (!m_pNetConfManager->sendData("board_mode",static_cast<uint32_t>(s.getBoardMode()),_async)) return false;

        if (!m_pNetConfManager->sendData(sendTest ? CNetConfigManager::ECommands::END_SEND_TEST_SETTING : CNetConfigManager::ECommands::END_SEND_SETTING,_async)) return false;
        return true;
    }
    return false;
}

auto ServerNetConfigManager::getSettings() -> const CStreamSettings{
    return m_settings;
}

auto ServerNetConfigManager::getSettingsRef() -> CStreamSettings&{
    return m_settings;
}


auto ServerNetConfigManager::getTempSettings() -> const CStreamSettings{
    if (m_testSettings.isSetted()){
        return m_testSettings;
    }
    m_testSettings = m_settings;
    return m_testSettings;
}

auto ServerNetConfigManager::setMode(broadcast_lib::EMode mode) -> void{
    m_mode = mode;
}
