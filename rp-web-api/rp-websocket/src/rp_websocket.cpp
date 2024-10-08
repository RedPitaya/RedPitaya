#include <json/json.h>
#include <stdint.h>

#include "rp_websocket.h"
#include "websocket_server.h"
#include "rp.h"

using namespace rp_websocket;

struct CWEBServer::Impl {
   std::shared_ptr<websocket_server> m_server = nullptr;
   Json::Value m_cache;
};

CWEBServer::CWEBServer(){
    m_pimpl = new CWEBServer::Impl();
}

CWEBServer::~CWEBServer(){
    receiveBool.disconnect_all();
    receiveInt.disconnect_all();
    receiveUInt.disconnect_all();
    receiveDouble.disconnect_all();
    receiveStr.disconnect_all();
    delete m_pimpl;
}

auto CWEBServer::startServer(uint16_t port) -> void{
    m_pimpl->m_server = std::make_shared<websocket_server>();
    m_pimpl->m_server->receiveHandle.connect([&](auto msg){
        Json::Value root;
        Json::CharReaderBuilder builder;
        JSONCPP_STRING err;
        const auto rawJsonLength = static_cast<int>(msg.length());
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(msg.c_str(), msg.c_str() + rawJsonLength, &root, &err)) {
            ERROR_LOG("Error parse json: %s",msg.c_str())
            return;
        }
        try{
            for(auto itr = root.begin() ; itr != root.end() ; itr++ ){
                std::string key = itr.key().asString();
                auto item = root[key];
                std::string type = "";
                if (item.isMember("type") && item["type"].isString()){
                    type = item["type"].asString();
                }

                if (type == "int"){
                    if (item.isMember("value") && item["value"].isInt()){
                        auto value = item["value"].asInt();
                        receiveInt(key,value);
                    }
                }

                if (type == "uint"){
                    if (item.isMember("value") && item["value"].isUInt()){
                        auto value = item["value"].asUInt();
                        receiveUInt(key,value);
                    }
                }

                if (type == "bool"){
                    if (item.isMember("value") && item["value"].isBool()){
                        auto value = item["value"].asBool();
                        receiveBool(key,value);
                    }
                }

                if (type == "double"){
                    if (item.isMember("value") && item["value"].isDouble()){
                        auto value = item["value"].asDouble();
                        receiveDouble(key,value);
                    }
                }

                if (type == "string"){
                    if (item.isMember("value") && item["value"].isString()){
                        auto value = item["value"].asString();
                        receiveStr(key,value);
                    }
                }
            }
        }catch(...){
            ERROR_LOG("Error parse json: %s",msg.c_str())
            return;
        }
    });
    m_pimpl->m_server->start(port);
}

auto CWEBServer::send(std::string_view key, bool value) -> bool {
    if (m_pimpl->m_server){
        Json::Value root;
        Json::Value data;
        data["value"] = value;
        data["type"] = "bool";
        root[key.data()] = data;
        Json::StreamWriterBuilder builder;
        std::string json = Json::writeString(builder, root);
        return m_pimpl->m_server->send(json.c_str(),json.length());
    }
    return false;
}

auto CWEBServer::send(std::string_view key, int value) -> bool {
    if (m_pimpl->m_server){
        Json::Value root;
        Json::Value data;
        data["value"] = value;
        data["type"] = "int";
        root[key.data()] = data;
        Json::StreamWriterBuilder builder;
        std::string json = Json::writeString(builder, root);
        return m_pimpl->m_server->send(json.c_str(),json.length());
    }
    return false;
}

auto CWEBServer::send(std::string_view key, uint32_t value) -> bool {
    if (m_pimpl->m_server){
        Json::Value root;
        Json::Value data;
        data["value"] = value;
        data["type"] = "uint";
        root[key.data()] = data;
        Json::StreamWriterBuilder builder;
        std::string json = Json::writeString(builder, root);
        return m_pimpl->m_server->send(json.c_str(),json.length());
    }
    return false;
}

auto CWEBServer::send(std::string_view key, float value) -> bool {
    if (m_pimpl->m_server){
        Json::Value root;
        Json::Value data;
        data["value"] = value;
        data["type"] = "float";
        root[key.data()] = data;
        Json::StreamWriterBuilder builder;
        std::string json = Json::writeString(builder, root);
        return m_pimpl->m_server->send(json.c_str(),json.length());
    }
    return false;
}

auto CWEBServer::send(std::string_view key, std::string_view value) -> bool {
    if (m_pimpl->m_server){
        Json::Value root;
        Json::Value data;
        data["value"] = value.data();
        data["type"] = "string";
        root[key.data()] = data;
        Json::StreamWriterBuilder builder;
        std::string json = Json::writeString(builder, root);
        return m_pimpl->m_server->send(json.c_str(),json.length());
    }
    return false;
}

auto CWEBServer::send(std::string_view json) -> bool{
     if (m_pimpl->m_server){
        return m_pimpl->m_server->send(json.data(),json.length());
    }
    return false;
}

auto CWEBServer::resetCache() -> void{
    m_pimpl->m_cache = Json::Value();
}


auto CWEBServer::sendRequest(std::string_view key, bool value, bool reset_cache) -> void{
    if (reset_cache) resetCache();
    Json::Value data;
    data["value"] = value;
    data["type"] = "string";
    m_pimpl->m_cache[key.data()] = data;
}

auto CWEBServer::sendRequest(std::string_view key, int value, bool reset_cache) -> void{
    if (reset_cache) resetCache();
    Json::Value data;
    data["value"] = value;
    data["type"] = "int";
    m_pimpl->m_cache[key.data()] = data;
}

auto CWEBServer::sendRequest(std::string_view key, uint32_t value, bool reset_cache) -> void{
    if (reset_cache) resetCache();
    Json::Value data;
    data["value"] = value;
    data["type"] = "uint";
    m_pimpl->m_cache[key.data()] = data;
}

auto CWEBServer::sendRequest(std::string_view key, float value, bool reset_cache) -> void{
    if (reset_cache) resetCache();
    Json::Value data;
    data["value"] = value;
    data["type"] = "float";
    m_pimpl->m_cache[key.data()] = data;
}

auto CWEBServer::sendRequest(std::string_view key, std::string_view value, bool reset_cache) -> void{
    if (reset_cache) resetCache();
    Json::Value data;
    data["value"] = value.data();
    data["type"] = "string";
    m_pimpl->m_cache[key.data()] = data;
}

auto CWEBServer::sendCache() -> bool{
    if (m_pimpl->m_server){
        Json::StreamWriterBuilder builder;
        std::string json = Json::writeString(builder, m_pimpl->m_cache);
        return m_pimpl->m_server->send(json.c_str(),json.length());
    }
    return false;
}