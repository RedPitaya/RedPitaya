#include <chrono>
#include <iostream>
#include <fstream>
#include <time.h>
#include <functional>
#include <cstdlib>
#include <cassert>
#include "data_lib/thread_cout.h"
#include "net_config_manager.h"

#define UNUSED(x) [&x]{}()

#define HEADER_STR_STR      0xABCDEF
#define HEADER_STR_INT      0xABCDEE
#define HEADER_STR_DOUBLE   0xABCDEC
#define HEADER_COMMAND      0xABCDED



auto pack(std::string key,std::string value,size_t *len) -> net_lib::net_buffer {
    uint32_t key_size = key.size();
    uint32_t data_size = value.size();
    uint32_t header = HEADER_STR_STR;
    uint32_t prefix_len = sizeof(uint32_t) * 4;
    uint32_t segment_size = key_size + data_size + prefix_len;
    auto buffer = std::shared_ptr<uint8_t[]>(new uint8_t[segment_size]);
    ((uint32_t*)buffer.get())[0] = (uint32_t)header;
    ((uint32_t*)buffer.get())[1] = (uint32_t)segment_size;
    ((uint32_t*)buffer.get())[2] = (uint32_t)key_size;
    ((uint32_t*)buffer.get())[3] = (uint32_t)data_size;
    memcpy_neon(buffer.get()+prefix_len, key.data(), key_size);
    memcpy_neon(buffer.get()+prefix_len + key_size, value.data(), data_size);
    *len = segment_size;
    return buffer;
}

auto pack(std::string key,uint32_t value,size_t *len) -> net_lib::net_buffer {
    uint32_t key_size = key.size();
    uint32_t data_size = sizeof(uint32_t);
    uint32_t header = HEADER_STR_INT;
    uint32_t prefix_len = sizeof(uint32_t) * 4;
    uint32_t segment_size = key_size + data_size + prefix_len;
    auto buffer = std::shared_ptr<uint8_t[]>(new uint8_t[segment_size]);
    ((uint32_t*)buffer.get())[0] = (uint32_t)header;
    ((uint32_t*)buffer.get())[1] = (uint32_t)segment_size;
    ((uint32_t*)buffer.get())[2] = (uint32_t)key_size;
    ((uint32_t*)buffer.get())[3] = (uint32_t)data_size;
    memcpy_neon(buffer.get()+prefix_len, key.data(), key_size);
    memcpy_neon(buffer.get()+prefix_len + key_size, &value, data_size);
    *len = segment_size;
    return buffer;
}

auto pack(std::string key, double value,size_t *len) -> net_lib::net_buffer {
    static_assert(sizeof(double) == 8,"Double have wrong size");
    uint32_t key_size = key.size();
    uint32_t data_size = sizeof(double);
    uint32_t header = HEADER_STR_DOUBLE;
    uint32_t prefix_len = sizeof(uint32_t) * 4;
    uint32_t segment_size = key_size + data_size + prefix_len;
    auto buffer = std::shared_ptr<uint8_t[]>(new uint8_t[segment_size]);
    ((uint32_t*)buffer.get())[0] = (uint32_t)header;
    ((uint32_t*)buffer.get())[1] = (uint32_t)segment_size;
    ((uint32_t*)buffer.get())[2] = (uint32_t)key_size;
    ((uint32_t*)buffer.get())[3] = (uint32_t)data_size;
    memcpy_neon(buffer.get() + prefix_len, key.data(), key_size);
    memcpy_neon(buffer.get() + prefix_len + key_size, &value, data_size);
    *len = segment_size;
    return buffer;
}

auto pack(uint32_t command,size_t *len) -> net_lib::net_buffer {
    uint32_t header = HEADER_COMMAND;
    uint32_t segment_size = sizeof(uint32_t) * 3;
    auto buffer = std::shared_ptr<uint8_t[]>(new uint8_t[segment_size]);
    ((uint32_t*)buffer.get())[0] = (uint32_t)header;
    ((uint32_t*)buffer.get())[1] = (uint32_t)segment_size;
    ((uint32_t*)buffer.get())[2] = (uint32_t)command;
    *len = segment_size;
    return buffer;
}

CNetConfigManager::CNetConfigManager() :
    m_host(""),
    m_port(""),
    m_asionet(nullptr),
    m_mode(net_lib::EMode::M_CLIENT),
    m_buffers()
{    
}

CNetConfigManager::~CNetConfigManager()
{   
   stopAsioNet();
   delete[] m_buffers.m_buffers;
}

auto CNetConfigManager::startAsioNet(net_lib::EMode _mode, std::string _host,std::string _port) -> bool {
    m_mode = _mode;
    m_host = _host;
    m_port = _port;
    if (m_host == ""  || m_port == "")
        return false; 
    return start();
}

auto CNetConfigManager::stopAsioNet() -> bool{
    if (m_asionet) {
        m_asionet->disconnect();
        delete m_asionet;
        m_asionet = nullptr;
        return true;
    }
    return false;
}

bool CNetConfigManager::start(){
    if (m_host == ""  || m_port == "")
        return false;

    if (m_asionet){
        delete m_asionet;
        m_asionet = nullptr;
    }
    m_asionet = new net_lib::CAsioNetSimple(m_mode, m_host, m_port);

    m_asionet->connectNotify.connect([=](auto &host){
        this->connectNotify(host);
    });
    m_asionet->disconnectNotify.connect([=](auto &host){this->disconnectNotify(host);});
    m_asionet->connectTimeoutNotify.connect([=](auto code){this->connectTimeoutNotify(code);});
    m_asionet->errorNotify.connect([=](auto code){this->errorNotify(code);});
    m_asionet->sendNotify.connect([=](auto e,auto s){this->sendNotify(e,s);});
    m_asionet->recivedNotify.connect(&CNetConfigManager::receiveHandler,this);
    
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
    return ret;
}

bool CNetConfigManager::sendData(std::string key,uint32_t value,bool async){
    if (!m_asionet) return false;
    if (!m_asionet->isConnected()) return false;
    size_t len = 0;
    auto buff = pack(key,value,&len);
    bool ret = m_asionet->sendData(async,buff,len);
    return ret;
}
bool CNetConfigManager::sendData(std::string key,double value,bool async){
    if (!m_asionet) return false;
    if (!m_asionet->isConnected()) return false;
    size_t len = 0;
    auto buff = pack(key,value,&len);
    bool ret = m_asionet->sendData(async,buff,len);
    return ret;
}
bool CNetConfigManager::sendData(uint32_t command,bool async){
    if (!m_asionet) return false;
    if (!m_asionet->isConnected()) return false;
    size_t len = 0;
    auto buff = pack(command,&len);
    bool ret = m_asionet->sendData(async,buff,len);
    return ret;
}

auto CNetConfigManager::sendData(ECommands command,bool async) -> bool{
    if (!m_asionet) return false;
    if (!m_asionet->isConnected()) return false;
    size_t len = 0;
    auto buff = pack(static_cast<uint32_t>(command),&len);
    bool ret = m_asionet->sendData(async,buff,len);
    return ret;
}

auto CNetConfigManager::receiveHandler(std::error_code,uint8_t* _buff,size_t _size) -> void {
    m_buffers.push_back(_buff,_size);

    while(m_buffers.m_data_size > 0){
        if (m_buffers.m_data_size < 8){
            break;
        }
        if (!m_buffers.m_buffers) break;
        uint32_t header = ((uint32_t*)m_buffers.m_buffers)[0];
        uint32_t segment_size = ((uint32_t*)m_buffers.m_buffers)[1];
        
        if (m_buffers.m_data_size < segment_size){
            break;
        }

        if (header == HEADER_STR_STR){
            uint32_t key_size = ((uint32_t*)m_buffers.m_buffers)[2];
            uint32_t data_size = ((uint32_t*)m_buffers.m_buffers)[3];
            std::string key(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16])), key_size);
            std::string data(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16+key_size])), data_size);
            m_buffers.removeAtStart(segment_size);
            receivedStringNotify(key,data);
        }
        else if (header == HEADER_STR_INT){
            uint32_t key_size = ((uint32_t*)m_buffers.m_buffers)[2];
            std::string key(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16])), key_size);
            uint32_t data = (reinterpret_cast<uint32_t*>(&(m_buffers.m_buffers[16+key_size])))[0];
            m_buffers.removeAtStart(segment_size);
            receivedIntNotify(key,data);
        }
        else if (header == HEADER_STR_DOUBLE){
            uint32_t key_size = ((uint32_t*)m_buffers.m_buffers)[2];
            std::string key(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16])), key_size);
            double data = (reinterpret_cast<double*>(&(m_buffers.m_buffers[16+key_size])))[0];
            m_buffers.removeAtStart(segment_size);
            receivedDoubleNotify(key,data);
        }else if (header == HEADER_COMMAND){
            uint32_t command = ((uint32_t*)m_buffers.m_buffers)[2];
            m_buffers.removeAtStart(segment_size);
            receivedCommandNotify(command);
        }else{
            throw std::runtime_error("Broken header");
        }

    }
}

auto CNetConfigManager::dyn_buffer::push_back(uint8_t *_src,uint32_t _size) -> void {
    if (_size + m_data_size > m_size){
        resize(_size + m_data_size);
    }
    memcpy_neon((&(*m_buffers)+m_data_size), _src, _size);
    m_data_size += _size;
}

auto CNetConfigManager::dyn_buffer::resize(uint32_t _size) -> void{
    if (_size == 0){
        delete[] m_buffers;
        m_buffers = nullptr;
        m_size = 0;
        return;
    }
    if (_size < m_size){
        m_size = _size;
    }
    uint8_t *new_buff = new uint8_t[_size];
    if (m_buffers)
        memcpy_neon(new_buff,m_buffers,m_size);
    m_size = _size;
    delete[] m_buffers;
    m_buffers = new_buff;
}

auto CNetConfigManager::dyn_buffer::removeAtStart(uint32_t _size) -> void{
    if (_size > m_data_size) throw std::runtime_error("[Error] remove buffer wrong size");
    memcpy_neon(m_buffers ,(&(*m_buffers)+_size),m_data_size - _size);
    m_data_size -= _size;
}

auto CNetConfigManager::getHost() -> std::string{
    return m_host;
}

auto CNetConfigManager::getPort() -> std::string{
    return m_port;
}







