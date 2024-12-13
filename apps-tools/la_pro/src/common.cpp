#include "json/json.h"
#include "rp_hw-profiles.h"
#include "common.h"
#include "rp.h"

auto getMAXFreq() -> uint32_t{
    uint32_t value = 125e6;
    if (rp_HPGetBaseSpeedHz(&value) != RP_HP_OK){
       ERROR_LOG("Can't get base speed");
    }
    return value;
}

auto getNameFromConfig(std::string &json) -> std::string{
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING err;
    if (json.length() == 0) return "";
    const auto rawJsonLength = static_cast<int>(json.length());
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(json.c_str(), json.c_str() + rawJsonLength, &root, &err)) {
        ERROR_LOG("Error parse json: %s err %s",json.c_str(),err.c_str())
        return "";
    }
    try{
        for(auto itr = root.begin() ; itr != root.end() ; itr++ ){
            std::string key = itr.key().asString();
            if (key == "name"){
                return root["name"].asString();
            }
        }
    }catch(...){
        ERROR_LOG("Error parse json: %s",json.c_str())
        return "";
    }
    return "";
}


auto getParamFromConfig(std::string &json) -> std::string{
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING err;
    if (json.length() == 0) return "";
    const auto rawJsonLength = static_cast<int>(json.length());
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(json.c_str(), json.c_str() + rawJsonLength, &root, &err)) {
        ERROR_LOG("Error parse json: %s err %s",json.c_str(),err.c_str())
        return "";
    }
    try{
        for(auto itr = root.begin() ; itr != root.end() ; itr++ ){
            std::string key = itr.key().asString();
            if (key == "config"){
                Json::Value node = root["config"];
                Json::StreamWriterBuilder builder;
                return Json::writeString(builder, node);
            }

        }
    }catch(...){
        ERROR_LOG("Error parse json: %s",json.c_str())
        return "";
    }
    return "";
}

auto getConfig(std::string &name, std::string &param_json) -> std::string{

    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING err;
    if (param_json.length() == 0) return "";
    const auto rawJsonLength = static_cast<int>(param_json.length());
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(param_json.c_str(), param_json.c_str() + rawJsonLength, &root, &err)) {
        ERROR_LOG("Error parse json: %s err %s",param_json.c_str(),err.c_str())
        return "";
    }

    Json::Value data;
    data["name"] = name;
    data["config"] = root;
    Json::StreamWriterBuilder builder2;
    return Json::writeString(builder2, data);
}

auto annoToJSON(std::map<uint8_t,std::string> map) -> std::string{
    Json::Value root;
    Json::StreamWriterBuilder builder;
    for ( const auto &itm : map ) {
        root[std::to_string(itm.first)] = itm.second;
    }
    return Json::writeString(builder, root);
}

