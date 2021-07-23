#include <chrono>
#include <iostream>
#include <fstream>
#include <time.h>
#include <functional>
#include <cstdlib>

#include "NetConfigManager.h"

#define UNUSED(x) [&x]{}()

#define HEADER_STR_STR      0xABCDEF
#define HEADER_STR_INT      0xABCDEE
#define HEADER_STR_DOUBLE   0xABCDEC
#define HEADER_COMMAND      0xABCDED


using namespace asionet_simple;

CAsioSocketSimple::as_buffer CNetConfigManager::pack(std::string key,std::string value,size_t *len){
    uint32_t key_size = key.size();
    uint32_t data_size = value.size();
    uint32_t header = HEADER_STR_STR;
    uint32_t prefix_len = sizeof(uint32_t) * 4;
    uint32_t segment_size = key_size + data_size + prefix_len;
    CAsioSocketSimple::as_buffer buffer = new uint8_t[segment_size];
    ((uint32_t*)buffer)[0] = (uint32_t)header;
    ((uint32_t*)buffer)[1] = (uint32_t)segment_size;
    ((uint32_t*)buffer)[2] = (uint32_t)key_size;
    ((uint32_t*)buffer)[3] = (uint32_t)data_size;
    memcpy_neon((&(*buffer)+prefix_len), key.data(), key_size);
    memcpy_neon((&(*buffer)+prefix_len + key_size), value.data(), data_size);
    *len = segment_size;
    return buffer;
}

CAsioSocketSimple::as_buffer CNetConfigManager::pack(std::string key,uint32_t value,size_t *len){
    uint32_t key_size = key.size();
    uint32_t data_size = sizeof(uint32_t);
    uint32_t header = HEADER_STR_INT;
    uint32_t prefix_len = sizeof(uint32_t) * 4;
    uint32_t segment_size = key_size + data_size + prefix_len;
    CAsioSocketSimple::as_buffer buffer = new uint8_t[segment_size];
    ((uint32_t*)buffer)[0] = (uint32_t)header;
    ((uint32_t*)buffer)[1] = (uint32_t)segment_size;
    ((uint32_t*)buffer)[2] = (uint32_t)key_size;
    ((uint32_t*)buffer)[3] = (uint32_t)data_size;
    memcpy_neon((&(*buffer)+prefix_len), key.data(), key_size);
    memcpy_neon((&(*buffer)+prefix_len + key_size), &value, data_size);
    *len = segment_size;
    return buffer;
}

CAsioSocketSimple::as_buffer CNetConfigManager::pack(std::string key, double value,size_t *len){
    static_assert(sizeof(double) == 8,"Double have wrong size");
    uint32_t key_size = key.size();
    uint32_t data_size = sizeof(double);
    uint32_t header = HEADER_STR_DOUBLE;
    uint32_t prefix_len = sizeof(uint32_t) * 4;
    uint32_t segment_size = key_size + data_size + prefix_len;
    CAsioSocketSimple::as_buffer buffer = new uint8_t[segment_size];
    ((uint32_t*)buffer)[0] = (uint32_t)header;
    ((uint32_t*)buffer)[1] = (uint32_t)segment_size;
    ((uint32_t*)buffer)[2] = (uint32_t)key_size;
    ((uint32_t*)buffer)[3] = (uint32_t)data_size;
    memcpy_neon((&(*buffer)+prefix_len), key.data(), key_size);
    memcpy_neon((&(*buffer)+prefix_len + key_size), &value, data_size);
    *len = segment_size;
    return buffer;
}

CAsioSocketSimple::as_buffer CNetConfigManager::pack(uint32_t command,size_t *len){
    uint32_t header = HEADER_COMMAND;
    uint32_t segment_size = sizeof(uint32_t) * 3;
    CAsioSocketSimple::as_buffer buffer = new uint8_t[segment_size];
    ((uint32_t*)buffer)[0] = (uint32_t)header;
    ((uint32_t*)buffer)[1] = (uint32_t)segment_size;
    ((uint32_t*)buffer)[2] = (uint32_t)command;
    *len = segment_size;
    return buffer;
}

CNetConfigManager::CNetConfigManager() :
    m_host(""),
    m_port(""),
    m_asionet(nullptr),
    m_mode(CAsioSocketSimple::ASMode::AS_CLIENT),
    m_buffers()
{    
}

CNetConfigManager::~CNetConfigManager()
{
   stopAsioNet();
}

bool CNetConfigManager::startAsioNet(CAsioSocketSimple::ASMode _mode, std::string _host,std::string _port){
    m_mode = _mode;
    m_host = _host;
    m_port = _port;
    if (m_host == ""  || m_port == "")
        return false; 
    return start();
}

bool CNetConfigManager::stopAsioNet(){
    if (m_asionet) {
        m_asionet->disconnect();
        delete m_asionet;
        m_asionet = nullptr;
        return true;
    }
    return false;
}

void CNetConfigManager::addHandler(CAsioSocketSimple::ASEvents _event, std::function<void(std::string host)> _func){
    switch(_event){
        case CAsioSocketSimple::ASEvents::AS_CONNECT:{
            this->m_callback_Str.addListener((int)CAsioSocketSimple::ASEvents::AS_CONNECT,_func);
            break;
        }
        case CAsioSocketSimple::ASEvents::AS_DISCONNECT:{
            this->m_callback_Str.addListener((int)CAsioSocketSimple::ASEvents::AS_DISCONNECT,_func);
            break;
        }
        default:
            assert(false);
    }
}

void CNetConfigManager::addHandlerError(std::function<void(std::error_code error)> _func){
    this->m_callback_Error.addListener((int)CAsioSocketSimple::ASEvents::AS_ERROR,_func);
}

void CNetConfigManager::addHandlerSentCallback(std::function<void(std::error_code,int)> _func){
    this->m_callback_ErrorInt.addListener((int)CAsioSocketSimple::ASEvents::AS_SEND_DATA,_func);
}

void CNetConfigManager::addHandlerReceiveStrStr(std::function<void(std::string,std::string)> _func){
    this->m_callback_StrStr.addListener(HEADER_STR_STR,_func);
}

void CNetConfigManager::addHandlerReceiveStrInt(std::function<void(std::string, uint32_t)> _func) {
    this->m_callback_StrInt.addListener(HEADER_STR_INT,_func);
}

void CNetConfigManager::addHandlerReceiveStrDouble(std::function<void(std::string, double)> _func) {
    this->m_callback_StrDouble.addListener(HEADER_STR_DOUBLE,_func);
}

void CNetConfigManager::addHandlerReceiveCommand(std::function<void(uint32_t)> _func) {
    this->m_callback_Int.addListener(HEADER_COMMAND,_func);
}

bool CNetConfigManager::start(){
    if (m_host == ""  || m_port == "")
        return false;

    if (m_asionet){
        delete m_asionet;
        m_asionet = nullptr;
    }
    m_asionet = new CAsioNetSimple(m_mode, m_host, m_port);
    m_asionet->addCall_Connect([this](std::string host){
                                        //LOG_P("Connected: %s\n",host.c_str() );
                                        this->m_callback_Str.emitEvent((int)CAsioSocketSimple::ASEvents::AS_CONNECT,host);
                                    });
    m_asionet->addCall_Disconnect([this](std::string host)
                                    {
                                        //LOG_P("Disconnected: %s \n",host.c_str());
                                        this->m_callback_Str.emitEvent((int)CAsioSocketSimple::ASEvents::AS_DISCONNECT,host);
                                    });
    m_asionet->addCall_Error([this](std::error_code error)
                                    {
                                        //LOG_P("Error: %d (%s)\n", error.value(),error.message().c_str());
                                        this->m_callback_Error.emitEvent((int)CAsioSocketSimple::ASEvents::AS_ERROR,error);
                                    });
    m_asionet->addCall_Send([this](std::error_code error,size_t size)
                                    {
                                        //LOG_P("Data sent (%d): %d\n",size,error.value() );
                                        this->m_callback_ErrorInt.emitEvent((int)CAsioSocketSimple::ASEvents::AS_SEND_DATA,error,size);
                                    });
    
    m_asionet->addCall_Received(std::bind(&CNetConfigManager::receiveHandler, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    
    m_asionet->start();
    return true;
}

bool CNetConfigManager::isConnected(){
    if (!m_asionet) return false;
    return m_asionet->isConnected();
}

bool CNetConfigManager::sendData(std::string key,std::string value,bool async){
    if (!m_asionet) return false;
    if (!m_asionet->isConnected()) return false;
    size_t len = 0;
    auto buff = pack(key,value,&len);
    bool ret = m_asionet->sendData(async,buff,len);
    if (!async)
        delete[] buff; // delete only in sync mode
    return ret;
}

bool CNetConfigManager::sendData(std::string key,uint32_t value,bool async){
    if (!m_asionet) return false;
    if (!m_asionet->isConnected()) return false;
    size_t len = 0;
    auto buff = pack(key,value,&len);
    bool ret = m_asionet->sendData(async,buff,len);
    if (!async)
        delete[] buff; // delete only in sync mode
    return ret;
}
bool CNetConfigManager::sendData(std::string key,double value,bool async){
    if (!m_asionet) return false;
    if (!m_asionet->isConnected()) return false;
    size_t len = 0;
    auto buff = pack(key,value,&len);
    bool ret = m_asionet->sendData(async,buff,len);
    if (!async)
        delete[] buff; // delete only in sync mode
    return ret;
}
bool CNetConfigManager::sendData(uint32_t command,bool async){
    if (!m_asionet) return false;
    if (!m_asionet->isConnected()) return false;
    size_t len = 0;
    auto buff = pack(command,&len);
    bool ret = m_asionet->sendData(async,buff,len);
    if (!async)
        delete[] buff; // delete only in sync mode
    return ret;
}

auto CNetConfigManager::sendData(Commands command,bool async) -> bool{
    if (!m_asionet) return false;
    if (!m_asionet->isConnected()) return false;
    size_t len = 0;
    auto buff = pack(static_cast<uint32_t>(command),&len);
    bool ret = m_asionet->sendData(async,buff,len);
    if (!async)
        delete[] buff; // delete only in sync mode
    return ret;
}


void CNetConfigManager::receiveHandler(std::error_code error,uint8_t* _buff,size_t _size){
    UNUSED(error);
    m_buffers.push_back(_buff,_size);

    while(m_buffers.m_data_size > 0){
        if (m_buffers.m_data_size < 8){
            break;
        }
        uint32_t header = ((uint32_t*)m_buffers.m_buffers)[0];
        int segment_size = ((uint32_t*)m_buffers.m_buffers)[1];
        
        if (m_buffers.m_data_size < segment_size){
            break;
        }

        if (header == HEADER_STR_STR){
            uint32_t key_size = ((uint32_t*)m_buffers.m_buffers)[2];
            uint32_t data_size = ((uint32_t*)m_buffers.m_buffers)[3];
            std::string key(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16])), key_size);
            std::string data(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16+key_size])), data_size);
            m_buffers.removeAtStart(segment_size);
            this->m_callback_StrStr.emitEvent(HEADER_STR_STR,key,data);
        }
        else if (header == HEADER_STR_INT){
            uint32_t key_size = ((uint32_t*)m_buffers.m_buffers)[2];
            std::string key(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16])), key_size);
            uint32_t data = (reinterpret_cast<uint32_t*>(&(m_buffers.m_buffers[16+key_size])))[0];
            m_buffers.removeAtStart(segment_size);
            this->m_callback_StrInt.emitEvent(HEADER_STR_INT,key,data);
        }
        else if (header == HEADER_STR_DOUBLE){
            uint32_t key_size = ((uint32_t*)m_buffers.m_buffers)[2];
            std::string key(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16])), key_size);
            double data = (reinterpret_cast<double*>(&(m_buffers.m_buffers[16+key_size])))[0];
            m_buffers.removeAtStart(segment_size);
            this->m_callback_StrDouble.emitEvent(HEADER_STR_DOUBLE,key,data);
        }else if (header == HEADER_COMMAND){
            uint32_t command = ((uint32_t*)m_buffers.m_buffers)[2];
            m_buffers.removeAtStart(segment_size);
            this->m_callback_Int.emitEvent(HEADER_COMMAND,command);
        }else{
            throw std::runtime_error("Broken header");
        }

    }
}

void CNetConfigManager::dyn_buffer::push_back(uint8_t *_src,int _size){
    if (_size + m_data_size > m_size){
        resize(_size + m_data_size);
    }
    memcpy_neon((&(*m_buffers)+m_data_size), _src, _size);
    m_data_size += _size;
}

void CNetConfigManager::dyn_buffer::resize(int _size){
    if (_size < m_size){
        m_size = _size;
    }
    uint8_t *new_buff = new uint8_t[_size];
    if (m_buffers)
        memcpy_neon(new_buff,m_buffers,m_size);
    m_size = _size;
    if (m_buffers) delete[] m_buffers;
    m_buffers = new_buff;
}

void CNetConfigManager::dyn_buffer::removeAtStart(int _size){
    if (_size > m_data_size) throw std::runtime_error("[Error] remove buffer wrong size");
    memcpy_neon(m_buffers ,(&(*m_buffers)+_size),m_data_size - _size);
    m_data_size -= _size;
}







