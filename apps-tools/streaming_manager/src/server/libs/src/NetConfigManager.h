#pragma once

#include <list>
#include "EventHandlers.h"
#include "AsioNetSimple.h"
#include "log.h"

using namespace std;
using namespace asionet_simple;

class CNetConfigManager
{
public:

    static shared_ptr<CNetConfigManager> instance()
    {
        static shared_ptr<CNetConfigManager> inst{new CNetConfigManager()};
        return inst;
    }

    CNetConfigManager();
    ~CNetConfigManager();


    auto startAsioNet(Mode _mode, string _host,string _port) -> bool;
    auto stopAsioNet() -> bool;
    auto isConnected() -> bool;

    auto addHandler(Events _event, function<void(string host)> _func) -> void;
    auto addHandlerSentCallback(function<void(error_code,int)> _func) -> void;
    auto addHandlerError(function<void(error_code error)> _func) -> void;
    auto addHandlerReceiveStrStr(function<void(string,string)> _func) -> void;

    auto sendData(string key,string value,bool async = true) -> bool;
    

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
    auto pack(string key,string value,size_t *len) -> asionet_simple::buffer;
    auto receiveHandler(error_code error,uint8_t*,size_t) -> void;

    string                    m_host;
    string                    m_port;
    CAsioNetSimple*           m_asionet;
    Mode                      m_mode;
    EventList<string>         m_callback_Str;
    EventList<string,string>  m_callback_StrStr;
    EventList<error_code>     m_callback_Error;
    EventList<error_code,int> m_callback_ErrorInt;
    dyn_buffer                m_buffers;
};
