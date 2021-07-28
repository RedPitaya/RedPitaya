#include <iostream>
#include <fstream>
#include "stream_settings.h"
#include "json/json.h"
#define UNUSED(x) [&x]{}()

using namespace std;

CStreamSettings::CStreamSettings(){
    m_port = "";
    m_protocol = TCP;
    m_samples = -1;
    m_format = WAV;
    m_type = RAW;
    m_saveType = NET;
    m_channels = CH1;
    m_res = BIT_8;
    m_decimation = 1;
    m_attenuator = A_1_1;
    m_calib = false;
    m_ac_dc = AC;
    reset();
}

void CStreamSettings::reset(){
    m_Bport =
    m_Bprotocol =
    m_Bsamples =
    m_Bformat =
    m_Btype =
    m_BsaveType = 
    m_Bchannels =
    m_Bres =
    m_Bdecimation =
    m_Battenuator = 
    m_Bcalib =
    m_Bac_dc = false;
}

bool CStreamSettings::isSetted(){
    bool res = true;
    res =   m_Bport &&
            m_Bprotocol &&
            m_Bsamples &&
            m_Bformat &&
            m_Btype &&
            m_Bchannels &&
            m_Bres &&
            m_Bdecimation &&
            m_Battenuator &&
            m_Bcalib &&
            m_Bac_dc;

    return res;
}

bool CStreamSettings::writeToFile(string _filename){
    if (isSetted()){
        Json::Value root;
        root["port"] = getPort();
        root["protocol"] = getProtocol();
        root["samples"] = getSamples();
        root["format"] = getFormat();
        root["type"] = getType();
        root["save_type"] = getSaveType();
        root["channels"] = getChannels();
        root["resolution"] = getResolution();
        root["decimation"] = getDecimation();
        root["attenuator"] = getAttenuator();
        root["calibration"] = getCalibration();
        root["coupling"] = getAC_DC();
        Json::StreamWriterBuilder builder;
        const std::string json_file = Json::writeString(builder, root);
        ofstream file(_filename , 	ios::out | ios::trunc);
        if (!file.is_open()) {
            std::cerr << "file write failed: " << std::strerror(errno) << "\n";
            return false;
        }
        file << json_file;
        return true;
    }
    return false;
}

auto CStreamSettings::readFromFile(string _filename) -> bool {

    Json::Value root;
    std::ifstream file(_filename , 	ios::in);
    if (!file.is_open()) {
        std::cerr << "file read failed: " << std::strerror(errno) << "\n";
        return false;
    }

    reset();
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, file, &root, &errs)) {
        std::cerr << "[CStreamSettings] Error parse json" << errs << std::endl;
        return false;
    }
    if (root.isMember("port"))
        setPort(root["port"].asString());
    if (root.isMember("protocol"))
        setProtocol(static_cast<Protocol>(root["protocol"].asInt()));
    if (root.isMember("samples"))
        setSamples(root["samples"].asInt());
    if (root.isMember("format"))
        setFormat(static_cast<DataFormat>(root["format"].asInt()));
    if (root.isMember("type"))
        setType(static_cast<DataType>(root["type"].asInt()));
    if (root.isMember("save_type"))
        setSaveType(static_cast<SaveType>(root["save_type"].asInt()));
    if (root.isMember("channels"))
        setChannels(static_cast<Channel>(root["channels"].asInt()));
    if (root.isMember("resolution"))
        setResolution(static_cast<Resolution>(root["resolution"].asInt()));
    if (root.isMember("decimation"))
        setDecimation(root["decimation"].asUInt());
    if (root.isMember("attenuator"))
        setAttenuator(static_cast<Attenuator>(root["attenuator"].asInt()));
    if (root.isMember("calibration"))
        setCalibration(root["calibration"].asBool());
    if (root.isMember("coupling"))
        setAC_DC(static_cast<AC_DC>(root["coupling"].asInt()));
    return isSetted();

}

void CStreamSettings::setPort(string _port){
    m_port  = _port;
    m_Bport = true;
}

string CStreamSettings::getPort(){
    return m_port;
}

void  CStreamSettings::setProtocol(CStreamSettings::Protocol _protocol){
    m_protocol  = _protocol;
    m_Bprotocol = true;
}

CStreamSettings::Protocol CStreamSettings::getProtocol(){
    return m_protocol;
}

void CStreamSettings::setSamples(int32_t _samples){
    m_samples  = _samples;
    m_Bsamples = true;
}

int32_t CStreamSettings::getSamples(){
    return  m_samples;
}

void CStreamSettings::setFormat(CStreamSettings::DataFormat _format){
    m_format = _format;
    m_Bformat = true;
}

CStreamSettings::DataFormat CStreamSettings::getFormat(){
    return m_format;
}

void CStreamSettings::setType(DataType _type){
    m_type = _type;
    m_Btype = true;
}

CStreamSettings::DataType CStreamSettings::getType(){
    return m_type;
}

auto CStreamSettings::setSaveType(CStreamSettings::SaveType _type) -> void{
    m_saveType = _type;
    m_BsaveType = true;
}

auto CStreamSettings::getSaveType() -> CStreamSettings::SaveType{
    return m_saveType;
}

void CStreamSettings::setChannels(CStreamSettings::Channel _channels){
    m_channels = _channels;
    m_Bchannels = true;
}

CStreamSettings::Channel CStreamSettings::getChannels(){
    return m_channels;
}

void CStreamSettings::setResolution(Resolution _resolution){
    m_res = _resolution;
    m_Bres = true;
}

CStreamSettings::Resolution CStreamSettings::getResolution(){
    return m_res;
}

void CStreamSettings::setDecimation(uint32_t _decimation){
    m_decimation = _decimation;
    m_Bdecimation = true;
}

uint32_t CStreamSettings::getDecimation(){
    return m_decimation;
}

auto CStreamSettings::setValue(std::string key,std::string value) -> bool{
    if (key == "port") {
        setPort(value);
        return true;
    }
    return false;
}

auto CStreamSettings::setValue(std::string key,uint32_t value) -> bool{
    if (key == "protocol") {
        setProtocol(static_cast<Protocol>(value));
        return true;
    }

    if (key == "samples") {
        setSamples(value);
        return true;
    }

    if (key == "format") {
        setFormat(static_cast<DataFormat>(value));
        return true;
    }

    if (key == "type") {
        setType(static_cast<DataType>(value));
        return true;
    }

    if (key == "save_type") {
        setSaveType(static_cast<SaveType>(value));
        return true;
    }

    if (key == "channels") {
        setChannels(static_cast<Channel>(value));
        return true;
    }

    if (key == "resolution") {
        setResolution(static_cast<Resolution>(value));
        return true;
    }

    if (key == "decimation") {
        setDecimation(value);
        return true;
    }

    if (key == "attenuator") {
        setAttenuator(static_cast<Attenuator>(value));
        return true;
    }

    if (key == "calibration") {
        setCalibration(static_cast<bool>(value));
        return true;
    }

    if (key == "coupling") {
        setAC_DC(static_cast<AC_DC>(value));
        return true;
    }
    return false;
}

auto CStreamSettings::setValue(std::string key,double value) -> bool{
    UNUSED(key);
    UNUSED(value);
    return false;
}


void CStreamSettings::setAttenuator(CStreamSettings::Attenuator _attenuator){
    m_attenuator = _attenuator;
    m_Battenuator = true;
}

CStreamSettings::Attenuator CStreamSettings::getAttenuator(){
    return m_attenuator;
}

void CStreamSettings::setCalibration(bool _calibration){
    m_calib = _calibration;
    m_Bcalib = true;
}

bool CStreamSettings::getCalibration(){
    return m_calib;
}

void CStreamSettings::setAC_DC(CStreamSettings::AC_DC _value){
    m_ac_dc = _value;
    m_Bac_dc = true;
}

CStreamSettings::AC_DC CStreamSettings::getAC_DC(){
    return m_ac_dc;
}
