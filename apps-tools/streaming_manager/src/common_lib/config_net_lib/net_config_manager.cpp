#include "net_config_manager.h"
#include <time.h>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <thread>
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

namespace {

// Deleting the connection joins its asio thread, which cannot happen on that thread: there the
// deletion is handed to a thread of its own.
auto deleteAsioNet(net_lib::CAsioNetSimple* net) -> void {
    if (net->runningInIoThread()) {
        net->disconnectSignals();
        std::thread([net]() { delete net; }).detach();
    } else {
        delete net;
    }
}

}  // namespace

CNetConfigManager::~CNetConfigManager() {
    stopAsioNet();
    // A handler that stopped the connection may still be running on it (an owner without a
    // shared_ptr gets no protection from weak_from_this()): the last release of the connection
    // here joins its io thread before the buffer goes.
    std::shared_ptr<net_lib::CAsioNetSimple> retired;
    {
        std::lock_guard lock(m_netMtx);
        retired = std::move(m_retired);
    }
    retired.reset();
    delete[] m_buffers.m_buffers;
}

auto CNetConfigManager::startAsioNet(net_lib::EMode _mode, std::string _host, uint16_t _port) -> bool {
    m_mode = _mode;
    m_host = _host;
    m_port = _port;
    // a server without a host listens on every interface
    if ((m_mode != net_lib::EMode::M_SERVER && m_host == "") || m_port == 0)
        return false;
    return start();
}

auto CNetConfigManager::net() -> std::shared_ptr<net_lib::CAsioNetSimple> {
    std::lock_guard lock(m_netMtx);
    return m_asionet;
}

auto CNetConfigManager::stopAsioNet() -> bool {
    std::shared_ptr<net_lib::CAsioNetSimple> net;
    {
        std::lock_guard lock(m_netMtx);
        net = std::move(m_asionet);
    }
    if (!net)
        return false;
    net->disconnect();
    // A sender may still hold it (the last owner deletes it through deleteAsioNet); whatever it
    // reports from now on is not ours.
    net->disconnectSignals();
    // Stopped from its own handler: the handler goes on with this object after we return, so the
    // connection is kept until the destructor can wait for it (released here, it would be deleted
    // on a thread of its own while this object goes).
    if (net->runningInIoThread()) {
        std::lock_guard lock(m_netMtx);
        m_retired = std::move(net);
    }
    return true;
}

auto CNetConfigManager::waitSent(std::chrono::milliseconds timeout) -> bool {
    auto asionet = net();
    return asionet && asionet->waitSent(timeout);
}

bool CNetConfigManager::start() {
    // a server without a host listens on every interface
    if ((m_mode != net_lib::EMode::M_SERVER && m_host == "") || m_port == 0)
        return false;

    stopAsioNet();
    m_buffers.clear();

    auto asionet = std::shared_ptr<net_lib::CAsioNetSimple>(new net_lib::CAsioNetSimple(m_mode, m_host, m_port), deleteAsioNet);

    asionet->connectNotify.connect([=, this](auto& host) {
        // A new peer starts a new stream: nothing left over from the previous one.
        m_buffers.clear();
        this->connectNotify(host);
    });
    asionet->disconnectNotify.connect([=, this](auto& host) { this->disconnectNotify(host); });
    asionet->connectTimeoutNotify.connect([=, this](auto code) { this->connectTimeoutNotify(code); });
    asionet->errorNotify.connect([=, this](auto code) { this->errorNotify(code); });
    asionet->sendNotify.connect([=, this](auto e, auto s) { this->sendNotify(e, s); });
    asionet->recivedNotify.connect(&CNetConfigManager::receiveHandler, this);

    {
        std::lock_guard lock(m_netMtx);
        m_asionet = asionet;
    }
    asionet->start();
    return true;
}

bool CNetConfigManager::isConnected() {
    auto asionet = net();
    return asionet && asionet->isConnected();
}

auto CNetConfigManager::sendData(std::string key, std::string value, bool async) -> bool {
    auto asionet = net();
    if (!asionet || !asionet->isConnected())
        return false;
    size_t len = 0;
    auto buff = packStrStr(key, value, &len);
    return asionet->sendData(async, buff, len);
}

bool CNetConfigManager::sendConfig(std::string value, bool async) {
    auto asionet = net();
    if (!asionet || !asionet->isConnected())
        return false;
    size_t len = 0;
    auto buff = packConfig(value, &len);
    return asionet->sendData(async, buff, len);
}

bool CNetConfigManager::sendCommand(ECommands command, const std::string& tag, bool async) {
    auto asionet = net();
    if (!asionet || !asionet->isConnected())
        return false;
    size_t len = 0;
    auto buff = packCommand(command, tag, &len);
    return asionet->sendData(async, buff, len);
}

auto CNetConfigManager::receiveHandler(std::error_code, uint8_t* _buff, size_t _size) -> void {
    // A handler may drop the last owner of this object (a client list replaced from a callback):
    // it lives until the loop below is done with its buffer.
    auto self = weak_from_this().lock();
    m_buffers.push_back(_buff, _size);
    while (m_buffers.m_data_size >= 8 && m_buffers.m_buffers) {
        const uint32_t* words = (uint32_t*)m_buffers.m_buffers;
        uint32_t header = words[0];
        uint32_t segment_size = words[1];
        uint32_t prefix_len = 0;
        if (header == HEADER_CONFIG) {
            prefix_len = 12;
        } else if (header == HEADER_STR_STR || header == HEADER_COMMAND) {
            prefix_len = 16;
        } else {
            return rejectStream("Broken header");
        }
        // The packers write exactly prefix + payload; anything else is not this protocol.
        if (segment_size < prefix_len || segment_size > MAX_SEGMENT_SIZE)
            return rejectStream("Broken segment size");
        if (m_buffers.m_data_size < segment_size)
            break;
        const uint64_t payload = segment_size - prefix_len;
        if (header == HEADER_CONFIG) {
            uint32_t data_size = words[2];
            if (data_size != payload)
                return rejectStream("Broken segment size");
            std::string data(reinterpret_cast<char const*>(&(m_buffers.m_buffers[12])), data_size);
            m_buffers.removeAtStart(segment_size);
            receivedConfigNotify(data);
        } else if (header == HEADER_STR_STR) {
            uint32_t key_size = words[2];
            uint32_t data_size = words[3];
            if (uint64_t(key_size) + data_size != payload)
                return rejectStream("Broken segment size");
            std::string key(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16])), key_size);
            std::string data(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16 + key_size])), data_size);
            m_buffers.removeAtStart(segment_size);
            receivedStringNotify(key, data);
        } else {
            uint32_t command = words[2];
            uint32_t data_size = words[3];
            if (data_size != payload)
                return rejectStream("Broken segment size");
            std::string key(reinterpret_cast<char const*>(&(m_buffers.m_buffers[16])), data_size);
            m_buffers.removeAtStart(segment_size);
            receivedCommandNotify(command, key);
        }
    }
}

auto CNetConfigManager::rejectStream(const char* reason) -> void {
    // The rest of the stream cannot be framed: drop it with the connection.
    ERROR_LOG("Config channel: %s, the connection is closed", reason)
    m_buffers.clear();
    if (auto asionet = net())
        asionet->closeConnection();
}

auto CNetConfigManager::dyn_buffer::push_back(uint8_t* _src, uint32_t _size) -> void {
    // The socket hands over 4 KB at a time: growing to the exact size copied the whole buffer
    // for every piece, a packet of the largest size took ~128 MB of copies.
    const uint64_t need = uint64_t(m_data_size) + _size;
    if (need > m_size)
        resize(uint32_t(std::max<uint64_t>(need, std::min<uint64_t>(uint64_t(m_size) * 2, UINT32_MAX))));
    memcpy_neon(m_buffers + m_data_size, _src, _size);
    m_data_size += _size;
}

auto CNetConfigManager::dyn_buffer::resize(uint32_t _size) -> void {
    if (_size == 0) {
        delete[] m_buffers;
        m_buffers = nullptr;
        m_size = 0;
        m_data_size = 0;
        return;
    }
    uint8_t* new_buff = new uint8_t[_size];
    m_data_size = std::min(m_data_size, _size);
    if (m_buffers && m_data_size)
        memcpy(new_buff, m_buffers, m_data_size);
    m_size = _size;
    delete[] m_buffers;
    m_buffers = new_buff;
}

auto CNetConfigManager::dyn_buffer::removeAtStart(uint32_t _size) -> void {
    if (_size > m_data_size)
        throw std::runtime_error("Remove buffer wrong size");
    if (_size < m_data_size)
        memmove(m_buffers, m_buffers + _size, m_data_size - _size);
    m_data_size -= _size;
}

auto CNetConfigManager::parseTriggerTarget(const std::string& tag, ETriggerTarget& target) -> bool {
    if (tag == std::to_string(TT_ADC)) {
        target = TT_ADC;
    } else if (tag == std::to_string(TT_DAC)) {
        target = TT_DAC;
    } else if (tag == std::to_string(TT_GPIO)) {
        target = TT_GPIO;
    } else {
        return false;
    }
    return true;
}

auto CNetConfigManager::dyn_buffer::clear() -> void {
    m_data_size = 0;
}

auto CNetConfigManager::getHost() -> std::string {
    return m_host;
}

auto CNetConfigManager::getPort() -> uint16_t {
    return m_port;
}
