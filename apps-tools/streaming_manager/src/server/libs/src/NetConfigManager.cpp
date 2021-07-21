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

asionet_simple::buffer CNetConfigManager::pack(string key,string value,size_t *len){
    uint32_t key_size = key.size();
    uint32_t data_size = value.size();
    uint32_t header = HEADER_STR_STR;
    uint32_t prefix_len = sizeof(uint32_t) * 4;
    uint32_t segment_size = key_size + data_size + prefix_len;
    asionet_simple::buffer buffer = new uint8_t[segment_size];
    ((uint32_t*)buffer)[0] = (uint32_t)header;
    ((uint32_t*)buffer)[1] = (uint32_t)segment_size;
    ((uint32_t*)buffer)[2] = (uint32_t)key_size;
    ((uint32_t*)buffer)[3] = (uint32_t)data_size;
    memcpy_neon((&(*buffer)+prefix_len), key.data(), key_size);
    memcpy_neon((&(*buffer)+prefix_len + key_size), value.data(), data_size);
    *len = segment_size;
    return buffer;
}

CNetConfigManager::CNetConfigManager() :
    m_host(""),
    m_port(""),
    m_asionet(nullptr),
    m_mode(CLIENT),    
    m_buffers()
{    
}

CNetConfigManager::~CNetConfigManager()
{
   stopAsioNet();
}

bool CNetConfigManager::startAsioNet(Mode _mode, string _host,string _port){
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

void CNetConfigManager::addHandler(Events _event, std::function<void(std::string host)> _func){
    switch(_event){
        case CONNECT:{
            this->m_callback_Str.addListener(CONNECT,_func);
            break;
        }
        case DISCONNECT:{
            this->m_callback_Str.addListener(DISCONNECT,_func);
            break;
        }
        default:
            assert(false);
    }
}

void CNetConfigManager::addHandlerError(std::function<void(std::error_code error)> _func){
    this->m_callback_Error.addListener(ERROR,_func);   
}

void CNetConfigManager::addHandlerSentCallback(function<void(error_code,int)> _func){
    this->m_callback_ErrorInt.addListener(SEND_DATA,_func);
}

void CNetConfigManager::addHandlerReceiveStrStr(function<void(string,string)> _func){
    this->m_callback_StrStr.addListener(HEADER_STR_STR,_func);
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
                                        LOG_P("Connected: %s\n",host);
                                        this->m_callback_Str.emitEvent(CONNECT,host);                                         
                                    });
    m_asionet->addCall_Disconnect([this](std::string host)
                                    {
                                        LOG_P("Disconnected: %s\n",host);
                                        this->m_callback_Str.emitEvent(DISCONNECT,host);
                                    });
    m_asionet->addCall_Error([this](error_code error)
                                    {
                                        LOG_P("Error: %d\n",error);
                                        this->m_callback_Error.emitEvent(ERROR,error);
                                    });
    m_asionet->addCall_Send([this](error_code error,size_t size)
                                    {
                                        LOG_P("Data sent (%d): %d\n",size,error);
                                        this->m_callback_ErrorInt.emitEvent(SEND_DATA,error,size);
                                    });
    
    m_asionet->addCall_Received(std::bind(&CNetConfigManager::receiveHandler, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    
    m_asionet->start();
    return true;
}

bool CNetConfigManager::isConnected(){
    if (!m_asionet) return false;
    return m_asionet->isConnected();
}

bool CNetConfigManager::sendData(string key,string value,bool async){
    if (!m_asionet) return false;
    if (!m_asionet->isConnected()) return false;
    size_t len = 0;
    auto buff = pack(key,value,&len);
    bool ret = m_asionet->sendData(async,buff,len);
    if (!async)
        delete[] buff; // delete only in sync mode
    return ret;
}


void CNetConfigManager::receiveHandler(error_code error,uint8_t* _buff,size_t _size){
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
            std::string key(reinterpret_cast<char const*>(&m_buffers.m_buffers[4]), key_size);
            std::string data(reinterpret_cast<char const*>(&m_buffers.m_buffers[4+key_size]), data_size);
            m_buffers.removeAtStart(segment_size);
            this->m_callback_StrStr.emitEvent(HEADER_STR_STR,key,data);
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
    memcpy_neon(m_buffers ,(&(*m_buffers)+m_data_size),m_data_size - _size);
    m_data_size -= _size;
}







