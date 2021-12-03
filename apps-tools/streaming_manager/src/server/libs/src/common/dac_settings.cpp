#include <iostream>
#include <fstream>
#include "dac_settings.h"
#include "json/json.h"

using namespace std;


auto DacSettings::readFromFile(string _filename) -> std::vector<DacSettings> {
    std::vector<DacSettings> settings;
    Json::Value root;
    std::ifstream file(_filename , 	ios::in);
    if (!file.is_open()) {
        std::cerr << "file "<< _filename.c_str() <<" read failed: " << std::strerror(errno) << "\n";
        return settings;
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, file, &root, &errs)) {
        std::cerr << "[CDacSettings] Error parse json" << errs << std::endl;
        return settings;
    }
    std::string config_port = "8901";

    if (root.isMember("config_port") && root["config_port"].isString()){
        if (root.isMember("config_port") && root["config_port"].isString()){
            config_port = root["config_port"].asString();
        }else{
            std::cerr << "[CDacSettings] Can't parse config_port value: " << root["config_port"].toStyledString() << std::endl;
        }
    }

    if (root.isMember("nodes") && root["nodes"].isArray()){
        auto array = root["nodes"];
        for(Json::Value::ArrayIndex i = 0 ; i < array.size(); i++){
            auto obj = array[i];
            DacSettings s;
            if (obj.isMember("host") && obj["host"].isString()){
                s.host = obj["host"].asString();
            }else{
                std::cerr << "[CDacSettings] Can't parse host value: " << obj["host"].toStyledString() << std::endl;
            }

            if (obj.isMember("port") && obj["port"].isString()){
                s.port = obj["port"].asString();
            }else{
                std::cerr << "[CDacSettings] Can't parse port value: " << obj["port"].toStyledString() << std::endl;
            }

            s.config_port = config_port;

            if (obj.isMember("file") && obj["file"].isString()){
                s.dac_file = obj["file"].asString();
            }else{
                std::cerr << "[CDacSettings] Can't parse file value: " << obj["file"].toStyledString() << std::endl;
            }

            if (obj.isMember("file_type") && obj["file_type"].isString()){
                auto value = obj["file_type"].asString();
                if (value == "WAV"){
                    s.file_type = CStreamSettings::WAV;
                }

                if (value == "TDMS"){
                    s.file_type = CStreamSettings::TDMS;
                }
            }else{
                std::cerr << "[CDacSettings] Can't parse file_type value: " << obj["file_type"].toStyledString() << std::endl;
            }

            if (obj.isMember("repeat_mode") && obj["repeat_mode"].isString()){
                auto value = obj["repeat_mode"].asString();
                if (value == "OFF"){
                    s.dac_repeat_mode = CStreamSettings::DAC_REP_OFF;
                }

                if (value == "ON"){
                    s.dac_repeat_mode = CStreamSettings::DAC_REP_ON;
                }

                if (value == "INF"){
                    s.dac_repeat_mode = CStreamSettings::DAC_REP_INF;
                }

            }else{
                std::cerr << "[CDacSettings] Can't parse repeat_mode value: " << obj["repeat_mode"].toStyledString() << std::endl;
            }

            if (obj.isMember("repeats") && obj["repeats"].isInt64()){
                s.dac_repeat = obj["repeats"].asInt64();
            }else{
                std::cerr << "[CDacSettings] Can't parse repeats value: " << obj["repeats"].toStyledString() << std::endl;
            }

            if (obj.isMember("memory_cache") && obj["memory_cache"].isInt64()){
                s.dac_memory = obj["memory_cache"].asInt64();
            }else{
                std::cerr << "[CDacSettings] Can't parse memory_cache value: " << obj["memory_cache"].toStyledString() << std::endl;
            }

            if (obj.isMember("dac_speed") && obj["dac_speed"].isInt()){
                s.dac_speed = obj["dac_speed"].asInt();
            }else{
                std::cerr << "[CDacSettings] Can't parse dac_speed value: " << obj["dac_speed"].toStyledString() << std::endl;
            }

            if (obj.isMember("verbous") && obj["verbous"].isBool()){
                s.verbous = obj["verbous"].asBool();
            }else{
                std::cerr << "[CDacSettings] Can't parse verbous value: " << obj["verbous"].toStyledString() << std::endl;
            }

            settings.push_back(s);
        }
    }
    return settings;
}

