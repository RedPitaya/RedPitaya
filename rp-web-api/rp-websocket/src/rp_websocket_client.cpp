#include <rapidjson/document.h>
#include <stdint.h>

#include "common/rp_log.h"
#include "rp_websocket.h"
#include "websocket_client.h"
#include "websocket_json.hpp"

using namespace rp_websocket;

struct CWEBClient::Impl {
    std::shared_ptr<websocket_client> m_client = nullptr;
    rapidjson::Document m_cache;
};

CWEBClient::CWEBClient() {
    m_pimpl = new CWEBClient::Impl();
    m_pimpl->m_cache.SetObject();
}

CWEBClient::~CWEBClient() {
    receiveBool.disconnect_all();
    receiveInt.disconnect_all();
    receiveUInt.disconnect_all();
    receiveDouble.disconnect_all();
    receiveStr.disconnect_all();
    connected.disconnect_all();
    disconnected.disconnect_all();
    delete m_pimpl;
}

auto CWEBClient::start(std::string_view host, uint16_t port) -> bool {
    m_pimpl->m_client = std::make_shared<websocket_client>();
    m_pimpl->m_client->receiveHandle.connect([&](auto msg) { dispatch(*this, msg); });
    m_pimpl->m_client->opened.connect([&]() { connected(); });
    m_pimpl->m_client->closed.connect([&]() { disconnected(); });
    return m_pimpl->m_client->start(std::string(host), port);
}

auto CWEBClient::stop() -> void {
    if (m_pimpl->m_client) {
        m_pimpl->m_client->stop();
    }
}

auto CWEBClient::isConnected() -> bool {
    return m_pimpl->m_client ? m_pimpl->m_client->isConnected() : false;
}

auto CWEBClient::send(std::string_view key, bool value) -> bool {
    return send(encode(key, value, "bool"));
}

auto CWEBClient::send(std::string_view key, int value) -> bool {
    return send(encode(key, value, "int"));
}

auto CWEBClient::send(std::string_view key, uint32_t value) -> bool {
    return send(encode(key, value, "uint"));
}

auto CWEBClient::send(std::string_view key, float value) -> bool {
    return send(encode(key, value, "double"));
}

auto CWEBClient::send(std::string_view key, std::string_view value) -> bool {
    return send(encode(key, value, "string"));
}

auto CWEBClient::send(std::string_view json) -> bool {
    if (m_pimpl->m_client) {
        return m_pimpl->m_client->send(json.data(), json.length());
    }
    return false;
}

auto CWEBClient::resetCache() -> void {
    m_pimpl->m_cache.SetObject();
}

auto CWEBClient::sendRequest(std::string_view key, bool value, bool reset_cache) -> void {
    if (reset_cache)
        resetCache();
    cacheValue(m_pimpl->m_cache, key, value, "bool");
}

auto CWEBClient::sendRequest(std::string_view key, int value, bool reset_cache) -> void {
    if (reset_cache)
        resetCache();
    cacheValue(m_pimpl->m_cache, key, value, "int");
}

auto CWEBClient::sendRequest(std::string_view key, uint32_t value, bool reset_cache) -> void {
    if (reset_cache)
        resetCache();
    cacheValue(m_pimpl->m_cache, key, value, "uint");
}

auto CWEBClient::sendRequest(std::string_view key, float value, bool reset_cache) -> void {
    if (reset_cache)
        resetCache();
    cacheValue(m_pimpl->m_cache, key, value, "double");
}

auto CWEBClient::sendRequest(std::string_view key, std::string_view value, bool reset_cache) -> void {
    if (reset_cache)
        resetCache();
    cacheValue(m_pimpl->m_cache, key, value, "string");
}

auto CWEBClient::sendCache() -> bool {
    if (m_pimpl->m_client) {
        if (!m_pimpl->m_cache.IsObject()) {
            m_pimpl->m_cache.SetObject();
        }
        const std::string json = writeDocument(m_pimpl->m_cache);
        return m_pimpl->m_client->send(json.c_str(), json.length());
    }
    return false;
}
