#include "net_config_manager.h"
#include <time.h>
#include <cstdlib>
#include "data_lib/neon_asm.h"
#include "logger_lib/file_logger.h"

#define HEADER_STR_STR 0xABCDEF
#define HEADER_COMMAND 0xABCDED
#define HEADER_CONFIG 0xABCDEB

auto packStrStr(std::string key, std::string value, size_t* len) -> net_lib::net_buffer {
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
    memcpy_neon(buffer.get() + prefix_len, key.data(), key_size);
    memcpy_neon(buffer.get() + prefix_len + key_size, value.data(), data_size);
    *len = segment_size;
    return buffer;
}

auto packConfig(std::string value, size_t* len) -> net_lib::net_buffer {
    uint32_t data_size = value.size();
    uint32_t header = HEADER_CONFIG;
    uint32_t prefix_len = sizeof(uint32_t) * 3;
    uint32_t segment_size = data_size + prefix_len;
    auto buffer = std::shared_ptr<uint8_t[]>(new uint8_t[segment_size]);
    ((uint32_t*)buffer.get())[0] = (uint32_t)header;
    ((uint32_t*)buffer.get())[1] = (uint32_t)segment_size;
    ((uint32_t*)buffer.get())[2] = (uint32_t)data_size;
    memcpy_neon(buffer.get() + prefix_len, value.data(), data_size);
    *len = segment_size;
    return buffer;
}

auto packCommand(uint32_t command, std::string tag, size_t* len) -> net_lib::net_buffer {
    uint32_t tag_size = tag.size();
    uint32_t header = HEADER_COMMAND;
    uint32_t prefix_len = sizeof(uint32_t) * 4;
    uint32_t segment_size = tag_size + prefix_len;
    auto buffer = std::shared_ptr<uint8_t[]>(new uint8_t[segment_size]);
    ((uint32_t*)buffer.get())[0] = (uint32_t)header;
    ((uint32_t*)buffer.get())[1] = (uint32_t)segment_size;
    ((uint32_t*)buffer.get())[2] = (uint32_t)command;
    ((uint32_t*)buffer.get())[3] = (uint32_t)tag_size;
    memcpy_neon(buffer.get() + prefix_len, tag.data(), tag_size);
    *len = segment_size;
    return buffer;
}

CNetConfigManager::CNetConfigManager() : m_host(""), m_port(0), m_asionet(nullptr), m_mode(net_lib::EMode::M_CLIENT), m_buffers() {}

CNetConfigManager::~CNetConfigManager() {
    stopAsioNet();
    delete[] m_buffers.m_buffers;
}

auto CNetConfigManager::startAsioNet(net_lib::EMode _mode, std::string _host, uint16_t _port) -> bool {
    m_mode = _mode;
    m_host = _host;
    m_port = _port;
    if (m_host == "" || m_port == 0)
        return false;
    return start();
}

auto CNetConfigManager::stopAsioNet() -> bool {
    if (m_asionet) {
        m_asionet->disconnect();
        delete m_asionet;
        m_asionet = nullptr;
        return true;
    }
    return false;
}

bool CNetConfigManager::start() {
    if (m_host == "" || m_port == 0)
        return false;

    stopAsioNet();

    m_asionet = new net_lib::CAsioNetSimple(m_mode, m_host, m_port);

    m_asionet->connectNotify.connect([=, this](auto& host) { this->connectNotify(host); });
    m_asionet->disconnectNotify.connect([=, this](auto& host) { this->disconnectNotify(host); });
    m_asionet->connectTimeoutNotify.connect([=, this](auto code) { this->connectTimeoutNotify(code); });
    m_asionet->errorNotify.connect([=, this](auto code) { this->errorNotify(code); });
    m_asionet->sendNotify.connect([=, this](auto e, auto s) { this->sendNotify(e, s); });
    m_asionet->recivedNotify.connect(&CNetConfigManager::receiveHandler, this);

    m_asionet->start();
    return true;
}

bool CNetConfigManager::isConnected() {
    if (!m_asionet)
        return false;
    return m_asionet->isConnected();
}

auto CNetConfigManager::sendData(std::string key, std::string value, bool async) -> bool {
    if (!m_asionet)
        return false;
    if (!m_asionet->isConnected())
        return false;
    size_t len = 0;
    auto buff = packStrStr(key, value, &len);
    bool ret = m_asionet->sendData(async, buff, len);
    return ret;
}

bool CNetConfigManager::sendConfig(std::string value, bool async) {
    if (!m_asionet)
        return false;
    if (!m_asionet->isConnected())
        return false;
    size_t len = 0;
    auto buff = packConfig(value, &len);
    bool ret = m_asionet->sendData(async, buff, len);
    return ret;
}

bool CNetConfigManager::sendCommand(ECommands command, const std::string& tag, bool async) {
    if (!m_asionet)
        return false;
    if (!m_asionet->isConnected())
        return false;
    size_t len = 0;
    auto buff = packCommand(command, tag, &len);
    bool ret = m_asionet->sendData(async, buff, len);
    return ret;
}

auto CNetConfigManager::receiveHandler(std::error_code, uint8_t* _buff, size_t _size) -> void {
    m_buffers.push_back(_buff, _size);
    while (m_buffers.m_data_size > 0) {
        if (m_buffers.m_data_size < 8) {
            break;
        }
        if (!m_buffers.m_buffers)
            break;
        uint32_t header = ((uint32_t*)m_buffers.m_buffers)[0];
        uint32_t segment_size = ((uint32_t*)m_buffers.m_buffers)[1];
        if (m_buffers.m_data_size < segment_size) {
            break;
        } else if (header == HEADER_CONFIG) {
            uint32_t data_size = ((uint32_t*)m_buffers.m_buffers)[2];
            std::string data(reinterpret_cast<char const*>(&(m_buffers.m_buffers[12])), data_size);
            m_buffers.removeAtStart(segment_size);
            receivedConfigNotify(data);
        } else if (header == HEADER_STR_STR) {
            uint32_t key_size = ((uint32_t*)m_buffers.m_buffers)[2];
            uint32_t data_size = ((uint32_t*)m_buffers.m_buffers)[3];
            std::string key(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16])), key_size);
            std::string data(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16 + key_size])), data_size);
            m_buffers.removeAtStart(segment_size);
            receivedStringNotify(key, data);
        } else if (header == HEADER_COMMAND) {
            uint32_t command = ((uint32_t*)m_buffers.m_buffers)[2];
            uint32_t data_size = ((uint32_t*)m_buffers.m_buffers)[3];
            std::string key(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16])), data_size);
            m_buffers.removeAtStart(segment_size);
            receivedCommandNotify(command, key);
        } else {
            TRACE_SHORT("Broken header")
            throw std::runtime_error("Broken header");
        }
    }
}

auto CNetConfigManager::dyn_buffer::push_back(uint8_t* _src, uint32_t _size) -> void {
    if (_size + m_data_size > m_size) {
        resize(_size + m_data_size);
    }
    memcpy_neon((&(*m_buffers) + m_data_size), _src, _size);
    m_data_size += _size;
}

auto CNetConfigManager::dyn_buffer::resize(uint32_t _size) -> void {
    if (_size == 0) {
        delete[] m_buffers;
        m_buffers = nullptr;
        m_size = 0;
        return;
    }
    if (_size < m_size) {
        m_size = _size;
    }
    uint8_t* new_buff = new uint8_t[_size];
    if (m_buffers) {
        for (uint32_t i = 0; i < m_size; i++) {
            new_buff[i] = m_buffers[i];
        }
    }
    m_size = _size;
    delete[] m_buffers;
    m_buffers = new_buff;
}

auto CNetConfigManager::dyn_buffer::removeAtStart(uint32_t _size) -> void {
    if (_size > m_data_size)
        throw std::runtime_error("Remove buffer wrong size");
    for (uint32_t i = 0; i < m_data_size - _size; i++) {
        m_buffers[i] = (&(*m_buffers) + _size)[i];
    }
    m_data_size -= _size;
}

auto CNetConfigManager::getHost() -> std::string {
    return m_host;
}

auto CNetConfigManager::getPort() -> uint16_t {
    return m_port;
}
