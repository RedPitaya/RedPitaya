#include <iostream>
#include <fstream>
#include "stream_settings.h"
#include "json/json.h"
#define UNUSED(x) [&x]{}()

using namespace std;

CStreamSettings::CStreamSettings(){
    m_port = "";
    m_dac_file = "";
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
    m_dac_gain = X1;
    m_dac_file_type = WAV;
    m_dac_mode = DAC_NET;
    m_dac_repeat = DAC_REP_OFF;
    m_dac_port = "";
    m_dac_memoryUsage = 1024 * 1024;
    m_dac_repeatCount = 0;
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
    m_Bac_dc =
    m_Bdac_file =
    m_Bdac_gain = 
    m_Bdac_mode = 
    m_Bdac_file_type = 
    m_Bdac_port =
    m_Bdac_repeat =
    m_Bdac_memoryUsage =
    m_Bdac_repeatCount = false;
}

bool CStreamSettings::isSetted(){
    bool res = true;
    res =   m_Bport &&
            m_Bdac_file &&
            m_Bprotocol &&
            m_Bsamples &&
            m_Bformat &&
            m_Btype &&
            m_Bchannels &&
            m_Bres &&
            m_Bdecimation &&
            m_Battenuator &&
            m_Bcalib &&
            m_Bac_dc &&
            m_Bdac_gain &&
            m_Bdac_mode &&
            m_Bdac_file_type &&
            m_Bdac_port &&
            m_dac_memoryUsage &&
            m_Bdac_repeat &&
            m_Bdac_repeatCount;

    // std::cerr << " port " << m_Bport 
    //           << " dac_file " << m_Bdac_file 
    //           << " protocol " << m_Bprotocol 
    //           << " smaples " << m_Bsamples
    //           << " format " << m_Bformat 
    //           << " type " << m_Btype
    //           << " channel " << m_Bchannels
    //           << " res " << m_Bres 
    //           << " dec " << m_Bdecimation 
    //           << " atte " << m_Battenuator
    //           << " calib " << m_Bcalib
    //           << " dac_gain " << m_Bdac_gain
    //           << " m_Bdac_file_type " << m_Bdac_file_type
    //           << " dc " << m_Bac_dc << "\n";
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
        root["dac_file"] = getDACFile();
        root["dac_file_type"] = getDACFileType();
        root["dac_gain"] = getDACGain();
        root["dac_mode"] = getDACMode();
        root["dac_repeat"] = getDACRepeat();
        root["dac_repeatCount"] = getDACRepeatCount();
        root["dac_port"] = getDACPort();
        root["dac_memoryUsage"] = getDACMemoryUsage();

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

auto CStreamSettings::getJson()-> std::string{
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
        root["dac_file"] = getDACFile();
        root["dac_file_type"] = getDACFileType();
        root["dac_gain"] = getDACGain();
        root["dac_mode"] = getDACMode();
        root["dac_repeat"] = getDACRepeat();
        root["dac_repeatCount"] = getDACRepeatCount();
        root["dac_port"] = getDACPort();
        root["dac_memoryUsage"] = getDACMemoryUsage();

        Json::StreamWriterBuilder builder;
        const std::string json = Json::writeString(builder, root);
        return json;
    }
    return "INCOMPLETE SETTING";
}

auto CStreamSettings::String()-> std::string{
    if (isSetted()){
        std::string  str = "";
        str = str + "Port:\t\t\t"+getPort()+"\n";

        std::string  protocol = "ERROR";
        switch (getProtocol()) {
            case TCP:
                protocol = "TCP";
                break;
            case UDP:
                protocol = "UDP";
                break;
        }
        str = str + "Protocol:\t\t" + protocol  +"\n";

        std::string  channels = "ERROR";
        switch (getChannels()) {
            case CH1:
                channels = "Channel 1";
                break;
            case CH2:
                channels = "Channel 2";
                break;
            case BOTH:
                channels = "Both channel";
                break;
        }
        str = str + "Channels:\t\t" + channels  +"\n";

        str = str + "Decimation:\t\t" + std::to_string(getDecimation())  +"\n";

        std::string  resolution = "ERROR";
        switch (getResolution()) {
            case BIT_8:
                resolution = "8 Bit";
                break;
            case BIT_16:
                resolution = "16 Bit";
                break;
        }
        str = str + "Resolution:\t\t" + resolution  +"\n";

        std::string  attenuator = "ERROR";
        switch (getAttenuator()) {
            case A_1_1:
                attenuator = "1:1";
                break;
            case A_1_20:
                attenuator = "1:20";
                break;
        }
        str = str + "Attenuator:\t\t" + attenuator  +" (125-14 and 250-12 only)\n";
        str = str + "Calibration:\t\t" + (getCalibration() ? "Enable" : "Disable")  +" (125-14 and 250-12 only)\n";

        std::string  coupling = "ERROR";
        switch (getAC_DC()) {
            case AC:
                coupling = "AC";
                break;
            case DC:
                coupling = "DC";
                break;
        }
        str = str + "AC/DC mode:\t\t" + coupling  +" (250-12 only)\n";

        std::string  savetype = "ERROR";
        switch (getSaveType()) {
            case SaveType::NET:
                savetype = "Network";
                break;
            case SaveType::FILE:
                savetype = "Local file";
        }
        str = str + "Mode:\t\t\t" + savetype  + "\n";

        str = str + "Samples:\t\t" + (getSamples() == -1 ? "Unlimited" : std::to_string(getSamples()))  +" (In file mode)\n";

        std::string  format = "ERROR";
        switch (getFormat()) {
            case DataFormat::WAV:
                format = "WAV";
                break;
            case DataFormat::TDMS:
                format = "TDMS";
                break;
            case DataFormat::CSV:
                format = "CSV";
                break;
            default:
                break;
        }
        str = str + "Data format:\t\t" + format  +" (In file mode)\n";

        std::string  type = "ERROR";
        switch (getType()) {
            case DataType::RAW:
                type = "RAW";
                break;
            case DataType::VOLT:
                type = "Voltage";
        }
        str = str + "Data type:\t\t" + type  +" (In file mode)\n";

        str = str + "\n******************** DAC  streaming ********************\n";
        std::string  dac_mode = "ERROR";
        
        switch (getDACMode()) {
            case DACType::DAC_NET:
                dac_mode = "Network";
                break;
            case DACType::DAC_FILE:
                dac_mode = "Local file";
        }
        str = str + "DAC Mode:\t\t" + dac_mode  + "\n";

        str = str + "Local file:\t\t" + getDACFile() + "\n";
        str = str + "Port:\t\t\t" + getDACPort() + "\n";
        
        std::string  dac_format = "ERROR";
        switch (getDACFileType()) {
            case DataFormat::WAV:
                dac_format = "WAV";
                break;
            case DataFormat::TDMS:
                dac_format = "TDMS";
                break;
            default:
                break;
        }
        str = str + "Data format:\t\t" + dac_format  +" (In DAC file mode)\n";

        std::string  dac_repeat = "ERROR";
        switch (getDACRepeat()) {
            case DACRepeat::DAC_REP_OFF:
                dac_repeat = "OFF";
                break;
            case DACRepeat::DAC_REP_INF:
                dac_repeat = "INF";
                break;
            case DACRepeat::DAC_REP_ON:
                dac_repeat = "ON";
                break;
        }
        str = str + "DAC repeat:\t\t" + dac_repeat +" (In file mode)\n";
        str = str + "DAC repeat count:\t\t" + std::to_string(getDACRepeatCount())  +" (In DAC file mode\n";
        str = str + "DAC memory cache:\t\t" + std::to_string(getDACMemoryUsage())  +" (In DAC file mode\n";


        std::string  dac_gain = "ERROR";
        switch (getDACGain()) {
            case DACGain::X1:
                dac_gain = "X1";
                break;
            case DACGain::X5:
                dac_gain = "X5";
                break;
        }
        str = str + "DAC Gain:\t\t" + dac_gain  +" (250-12 only)\n";        
        return str;
    }
    return "INCOMPLETE SETTING";
}


auto CStreamSettings::readFromFile(string _filename) -> bool {

    Json::Value root;
    std::ifstream file(_filename , 	ios::in);
    if (!file.is_open()) {
        std::cerr << "file "<< _filename.c_str() <<" read failed: " << std::strerror(errno) << "\n";
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
    if (root.isMember("resolution"))
        setResolution(static_cast<Resolution>(root["resolution"].asInt()));
    if (root.isMember("dac_file_type"))
        setDACFileType(static_cast<DataFormat>(root["dac_file_type"].asInt()));
    if (root.isMember("dac_gain"))
        setDACGain(static_cast<DACGain>(root["dac_gain"].asInt()));
    if (root.isMember("dac_file"))
        setDACFile(root["dac_file"].asString());
    if (root.isMember("dac_mode"))
        setDACMode(static_cast<DACType>(root["dac_mode"].asInt()));
    if (root.isMember("dac_repeat"))
        setDACRepeat(static_cast<DACRepeat>(root["dac_repeat"].asInt()));
    if (root.isMember("dac_repeatCount"))
        setDACRepeatCount(static_cast<uint32_t>(root["dac_repeatCount"].asInt()));
    if (root.isMember("dac_port"))
        setDACPort(root["dac_port"].asString());
    if (root.isMember("dac_memoryUsage"))
        setDACMemoryUsage(root["dac_memoryUsage"].asInt64());
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
    if (key == "dac_file") {
        setDACFile(value);
        return true;
    }
    if (key == "dac_port") {
        setDACPort(value);
        return true;
    }
    return false;
}

auto CStreamSettings::setValue(std::string key,int64_t value) -> bool{
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

    if (key == "dac_file_type") {
        setDACFileType(static_cast<DataFormat>(value));
        return true;
    }

    if (key == "dac_gain") {
        setDACGain(static_cast<DACGain>(value));
        return true;
    }

    if (key == "dac_mode") {
        setDACMode(static_cast<DACType>(value));
        return true;
    }

    if (key == "dac_repeat") {
        setDACRepeat(static_cast<DACRepeat>(value));
        return true;
    }

    if (key == "dac_repeatCount") {
        setDACRepeatCount(static_cast<uint32_t>(value));
        return true;
    }

    if (key == "dac_memoryUsage") {
        setDACMemoryUsage(static_cast<int64_t>(value));
        return true;
    }
    return false;
}

auto CStreamSettings::setValue(std::string,double) -> bool{
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

auto CStreamSettings::setDACFile(std::string _value) -> void{
    m_dac_file = _value;
    m_Bdac_file = true;
}

auto CStreamSettings::getDACFile() -> std::string{
    return m_dac_file;
}

auto CStreamSettings::setDACFileType(CStreamSettings::DataFormat _value) -> void{
    if (_value == CSV) {
        m_dac_file_type = UNDEF;
    }else{
        m_dac_file_type = _value;
    }
    m_Bdac_file_type = true;
}

auto CStreamSettings::getDACFileType() -> CStreamSettings::DataFormat{
    return m_dac_file_type;
}

auto CStreamSettings::setDACGain(CStreamSettings::DACGain _value) -> void{
    m_dac_gain = _value;
    m_Bdac_gain = true;
}

auto CStreamSettings::getDACGain() -> CStreamSettings::DACGain{
    return m_dac_gain;
}

auto CStreamSettings::setDACMode(CStreamSettings::DACType _value) -> void{
    m_dac_mode = _value;
    m_Bdac_mode = true;
}

auto CStreamSettings::getDACMode() -> CStreamSettings::DACType{
    return m_dac_mode;
}

auto CStreamSettings::setDACRepeat(DACRepeat _value) -> void{
    m_dac_repeat = _value;
    m_Bdac_repeat = true;
}

auto CStreamSettings::getDACRepeat() -> DACRepeat{
    return m_dac_repeat;
}

auto CStreamSettings::getDACPort() -> std::string{
    return m_dac_port;
}

auto CStreamSettings::setDACPort(std::string _port) -> void{
    m_dac_port = _port;
    m_Bdac_port = true;
}

auto CStreamSettings::getDACMemoryUsage() -> int64_t{
    return m_dac_memoryUsage;
}

auto CStreamSettings::setDACMemoryUsage(int64_t _value) -> void{
    m_dac_memoryUsage = _value;
    m_Bdac_memoryUsage = true;
}

auto CStreamSettings::setDACRepeatCount(uint32_t _value) -> void{
    m_dac_repeatCount = _value;
    m_Bdac_repeatCount = true;
}

auto CStreamSettings:: getDACRepeatCount() -> uint32_t{
    return m_dac_repeatCount;
}
