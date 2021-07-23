#pragma once

#include <list>
#include "EventHandlers.h"
#include "AsioNetSimple.h"
#include "log.h"

using namespace asionet_simple;

class CNetConfigManager
{
public:
    enum class Commands{
        BEGIN_SEND_SETTING,
        END_SEND_SETTING,
        SETTING_GET_SUCCES,
        SETTING_GET_FAIL,
        STOP_STREAMING,
        START_STREAMING,
        SERVER_STOPPED,
        SERVER_STARTED,
        SAVE_SETTING_TO_FILE,
        SAVE_TO_FILE_SUCCES,
        SAVE_TO_FILE_FAIL,
        LOAD_SETTING_FROM_FILE,
        LOAD_FROM_FILE_SUCCES,
        LOAD_FROM_FILE_FAIL
    };

    static std::shared_ptr<CNetConfigManager> instance()
    {
        static std::shared_ptr<CNetConfigManager> inst{new CNetConfigManager()};
        return inst;
    }

    CNetConfigManager();
    ~CNetConfigManager();


    auto startAsioNet(CAsioSocketSimple::ASMode _mode, std::string _host,std::string _port) -> bool;
    auto stopAsioNet() -> bool;
    auto isConnected() -> bool;

    auto addHandler(CAsioSocketSimple::ASEvents _event, std::function<void(std::string host)> _func) -> void;
    auto addHandlerSentCallback(std::function<void(std::error_code,int)> _func) -> void;
    auto addHandlerError(std::function<void(std::error_code error)> _func) -> void;
    auto addHandlerReceiveStrStr(std::function<void(std::string,std::string)> _func) -> void;
    auto addHandlerReceiveStrInt(std::function<void(std::string,uint32_t)> _func) -> void;
    auto addHandlerReceiveStrDouble(std::function<void(std::string,double)> _func) -> void;
    auto addHandlerReceiveCommand(std::function<void(uint32_t)> _func) -> void;

    auto sendData(std::string key,std::string value,bool async = true) -> bool;
    auto sendData(std::string key,uint32_t value,bool async = true) -> bool;
    auto sendData(std::string key,double value,bool async = true) -> bool;
    auto sendData(uint32_t command,bool async = true) -> bool;
    auto sendData(Commands command,bool async = true) -> bool;

private:
    struct dyn_buffer{
        uint8_t* m_buffers = nullptr;
        int      m_size = 0;
        int      m_data_size = 0;
        void push_back(uint8_t *_src,int _size);
        void resize(int _size);
        void removeAtStart(int _size);
        ~dyn_buffer() {if (m_buffers) delete[] m_buffers;}
    };

    CNetConfigManager(const CNetConfigManager &) = delete;
    CNetConfigManager(CNetConfigManager &&) = delete;

    auto start() -> bool;
    auto pack(std::string key,std::string value,size_t *len) -> CAsioSocketSimple::as_buffer;
    auto pack(std::string key,uint32_t value,size_t *len) -> CAsioSocketSimple::as_buffer;
    auto pack(std::string key,double value,size_t *len) -> CAsioSocketSimple::as_buffer;
    auto pack(uint32_t command,size_t *len) -> CAsioSocketSimple::as_buffer;

    auto receiveHandler(std::error_code error,uint8_t*,size_t) -> void;

    std::string                      m_host;
    std::string                      m_port;
    CAsioNetSimple*                  m_asionet;
    CAsioSocketSimple::ASMode        m_mode;
    EventList<std::string>           m_callback_Str;
    EventList<std::string,std::string>    m_callback_StrStr;
    EventList<std::string,uint32_t>  m_callback_StrInt;
    EventList<std::string,double>    m_callback_StrDouble;
    EventList<uint32_t>              m_callback_Int;
    EventList<std::error_code>       m_callback_Error;
    EventList<std::error_code,int>   m_callback_ErrorInt;
    dyn_buffer                       m_buffers;
};
