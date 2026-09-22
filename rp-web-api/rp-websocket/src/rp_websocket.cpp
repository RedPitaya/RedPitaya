#include <rapidjson/document.h>
#include <stdint.h>

#include "common/rp_log.h"
#include "rp_websocket.h"
#include "websocket_json.hpp"
#include "websocket_server.h"



using namespace rp_websocket;

struct CWEBServer::Impl {
    std::shared_ptr<websocket_server> m_server = nullptr;
    rapidjson::Document m_cache;
};

CWEBServer::CWEBServer() {
    m_pimpl = new CWEBServer::Impl();
    m_pimpl->m_cache.SetObject();
}

CWEBServer::~CWEBServer() {
    receiveBool.disconnect_all();
    receiveInt.disconnect_all();
    receiveUInt.disconnect_all();
    receiveDouble.disconnect_all();
    receiveStr.disconnect_all();
    delete m_pimpl;
}

auto CWEBServer::startServer(uint16_t port) -> void {
    m_pimpl->m_server = std::make_shared<websocket_server>();
    m_pimpl->m_server->receiveHandle.connect([&](auto msg) { dispatch(*this, msg); });
    m_pimpl->m_server->start(port);
}

auto CWEBServer::startServerBinaray(uint16_t port) -> void {
    m_pimpl->m_server = std::make_shared<websocket_server>();
    m_pimpl->m_server->start(port);
}

auto CWEBServer::send(std::string_view key, bool value) -> bool {
    return send(encode(key, value, "bool"));
}

auto CWEBServer::send(std::string_view key, int value) -> bool {
    return send(encode(key, value, "int"));
}

auto CWEBServer::send(std::string_view key, uint32_t value) -> bool {
    return send(encode(key, value, "uint"));
}

auto CWEBServer::send(std::string_view key, float value) -> bool {
    return send(encode(key, value, "float"));
}

auto CWEBServer::send(std::string_view key, std::string_view value) -> bool {
    return send(encode(key, value, "string"));
}

auto CWEBServer::sendInBinarayMode(const char* data, size_t size) -> bool {
    if (m_pimpl->m_server) {
        return m_pimpl->m_server->send(data, size);
    }
    return false;
}

auto CWEBServer::send(std::string_view json) -> bool {
    if (m_pimpl->m_server) {
        return m_pimpl->m_server->send(json.data(), json.length());
    }
    return false;
}

auto CWEBServer::resetCache() -> void {
    m_pimpl->m_cache.SetObject();
}

auto CWEBServer::sendRequest(std::string_view key, bool value, bool reset_cache) -> void {
    if (reset_cache)
        resetCache();
    cacheValue(m_pimpl->m_cache, key, value, "bool");
}

auto CWEBServer::sendRequest(std::string_view key, int value, bool reset_cache) -> void {
    if (reset_cache)
        resetCache();
    cacheValue(m_pimpl->m_cache, key, value, "int");
}

auto CWEBServer::sendRequest(std::string_view key, uint32_t value, bool reset_cache) -> void {
    if (reset_cache)
        resetCache();
    cacheValue(m_pimpl->m_cache, key, value, "uint");
}

auto CWEBServer::sendRequest(std::string_view key, float value, bool reset_cache) -> void {
    if (reset_cache)
        resetCache();
    cacheValue(m_pimpl->m_cache, key, value, "float");
}

auto CWEBServer::sendRequest(std::string_view key, std::string_view value, bool reset_cache) -> void {
    if (reset_cache)
        resetCache();
    cacheValue(m_pimpl->m_cache, key, value, "string");
}

auto CWEBServer::sendCache() -> bool {
    if (m_pimpl->m_server) {
        if (!m_pimpl->m_cache.IsObject()) {
            m_pimpl->m_cache.SetObject();
        }
        const std::string json = writeDocument(m_pimpl->m_cache);
        return m_pimpl->m_server->send(json.c_str(), json.length());
    }
    return false;
}
