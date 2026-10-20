#include "stream_settings.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include "data_lib/network_header.h"
#include "json/json.h"
#include "logger_lib/file_logger.h"

#define FILE_PATH "/home/redpitaya/streaming_files/dac"

namespace {

thread_local std::string g_dirPath = FILE_PATH;

auto to_uint64(char const* s) -> uint64_t {
    if (s == NULL || *s == '\0')
        throw std::invalid_argument("null or empty string argument");

    uint64_t result = 0;
    while (*s) {
        if (*s < '0' || *s > '9')
            throw std::invalid_argument("invalid input string");
        const uint64_t digit = static_cast<uint64_t>(*s - '0');
        if (result > (std::numeric_limits<uint64_t>::max() - digit) / 10)
            throw std::out_of_range("value does not fit in 64 bits");
        result = result * 10 + digit;
        ++s;
    }
    return result;
}

auto to_uint(char const* s) -> uint32_t {
    const uint64_t wide = to_uint64(s);
    if (wide > std::numeric_limits<uint32_t>::max())
        throw std::out_of_range("value does not fit in 32 bits");
    return static_cast<uint32_t>(wide);
}

// Decimal with an optional leading minus, the rest as strict as to_uint.
auto to_int(char const* s) -> int32_t {
    const bool negative = s != NULL && *s == '-';
    const uint64_t magnitude = to_uint64(negative ? s + 1 : s);
    if (negative ? magnitude > uint64_t{1} << 31 : magnitude > std::numeric_limits<int32_t>::max())
        throw std::out_of_range("value does not fit in 32 bits");
    return negative ? static_cast<int32_t>(-static_cast<int64_t>(magnitude)) : static_cast<int32_t>(magnitude);
}

}  // namespace

CStreamSettings::CStreamSettings() {
    resetDefault();
}

auto CStreamSettings::resetDefault() -> void {
    m_adcsettings = ADCSettings();
    m_memorysettings = MemorySettings();
    m_dacsettings = DACSettings();
    m_exttrigsettings = ExtTriggerSettings();
    m_gpiosettings = GPIOSettings();
}

CStreamSettings::~CStreamSettings() {}

CStreamSettings::CStreamSettings(const CStreamSettings& src) {
    copy(src);
}

CStreamSettings& CStreamSettings::operator=(const CStreamSettings& src) {
    copy(src);
    return *this;
}

auto CStreamSettings::copy(const CStreamSettings& src) -> void {
    m_adcsettings = src.m_adcsettings;
    m_dacsettings = src.m_dacsettings;
    m_memorysettings = src.m_memorysettings;
    m_exttrigsettings = src.m_exttrigsettings;
    m_gpiosettings = src.m_gpiosettings;
}

bool CStreamSettings::writeToFile(std::string _filename) {
    const std::string json_file = toJson();

    try {
        auto path = std::filesystem::path(_filename);
        std::filesystem::create_directories(path.parent_path());
    } catch (std::filesystem::filesystem_error const& ex) {
        ERROR_LOG("Error create dir %s", ex.what())
    }

    std::ofstream file(_filename, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        ERROR_LOG("File write failed %s", std::strerror(errno))
        return false;
    }

    file << json_file;
    file.close();
    return true;
}

auto CStreamSettings::toJson() const -> std::string {
    Json::Value root;
    Json::Value adc_config;
    Json::Value dac_config;
    Json::Value memory_config;
    Json::Value ext_trig_config;
    Json::Value gpio_config;

    adc_config["format_sd"] = getADCFormat().name();
    adc_config["data_type_sd"] = getADCType().name();
    adc_config["samples_limit_sd"] = getADCSamples();

    adc_config["adc_pass_mode"] = getADCPassMode().name();
    adc_config["resolution"] = getADCResolution().name();
    adc_config["adc_decimation"] = getADCDecimation();
    adc_config["use_calib"] = getADCCalibration().name();

    adc_config["adc_capture_time"] = getADCCaptureTime().name();

    for (auto i = 1u; i <= 4; i++) {
        adc_config["channel_state_" + std::to_string(i)] = getADCChannels(i).name();
        adc_config["channel_attenuator_" + std::to_string(i)] = getADCAttenuator(i).name();
        adc_config["channel_ac_dc_" + std::to_string(i)] = getADCAC_DC(i).name();
    }

    adc_config["adc_trigger_mode"] = getADCTriggerMode().name();
    adc_config["adc_trigger_sources"] = getADCTriggerSources();
    adc_config["adc_trigger_level_low"] = getADCTriggerLevelLow();
    adc_config["adc_trigger_level_high"] = getADCTriggerLevelHigh();
    adc_config["adc_trigger_edge"] = getADCTriggerEdge().name();
    adc_config["adc_event_source"] = getADCEventSource().name();
    adc_config["adc_trigger_blocks"] = getADCTriggerBlocks();

    dac_config["dac_rate"] = getDACSpeed();
    dac_config["file_sd"] = getDACFile();
    dac_config["file_type_sd"] = getDACFileType().name();
    dac_config["dac_pass_mode"] = getDACPassMode().name();

    dac_config["repeat"] = getDACRepeat().name();
    dac_config["repeatCount"] = getDACRepeatCount();

    for (auto i = 1u; i <= 2; i++) {
        dac_config["channel_gain_" + std::to_string(i)] = getDACGain(i).name();
        dac_config["dac_trigger_mode_" + std::to_string(i)] = getDACTriggerMode(i).name();
    }
    dac_config["dac_trigger_sources"] = getDACTriggerSources();
    dac_config["dac_event_source"] = getDACEventSource().name();

    memory_config["block_size"] = getMemoryBlockSize();
    memory_config["adc_size"] = getADCSize();
    memory_config["dac_size"] = getDACSize();
    memory_config["gpio_size"] = getGPIOSize();

    ext_trig_config["ext_trig_conditioner"] = getExtTrigConditioner().name();
    ext_trig_config["ext_trig_edge"] = getExtTrigEdge().name();
    ext_trig_config["ext_trig_debounce"] = getExtTrigDebounce();
    ext_trig_config["ext_trig_dio0"] = getExtTrigDio0().name();

    gpio_config["gpio_in_streaming"] = getGPIOInStreaming().name();
    gpio_config["gpio_out_streaming"] = getGPIOOutStreaming().name();
    gpio_config["gpio_outputs_p"] = getGPIOOutputsP();
    gpio_config["gpio_outputs_n"] = getGPIOOutputsN();
    gpio_config["gpio_polarity"] = getGPIOPolarity();
    gpio_config["gpio_decimation"] = getGPIODecimation();
    gpio_config["gpio_rle"] = getGPIORLE().name();
    gpio_config["gpio_out_step"] = getGPIOOutStep();
    gpio_config["gpio_pattern_mask"] = getGPIOPatternMask();
    gpio_config["gpio_pattern_value"] = getGPIOPatternValue();
    gpio_config["gpio_pattern_pos_edge"] = getGPIOPatternPosEdge();
    gpio_config["gpio_pattern_neg_edge"] = getGPIOPatternNegEdge();
    gpio_config["gpio_trigger_sources"] = getGPIOTriggerSources();
    gpio_config["gpio_event_source"] = getGPIOEventSource().name();
    gpio_config["gpio_in_trigger_mode"] = getGPIOInTriggerMode().name();
    gpio_config["gpio_out_trigger_mode"] = getGPIOOutTriggerMode().name();
    gpio_config["gpio_trigger_blocks"] = getGPIOTriggerBlocks();

    root["adc_streaming"] = adc_config;
    root["dac_streaming"] = dac_config;
    root["memory_manager"] = memory_config;
    root["ext_trigger"] = ext_trig_config;
    root["gpio_streaming"] = gpio_config;

    Json::StreamWriterBuilder builder;
    const std::string json = Json::writeString(builder, root);
    return json;
}

auto CStreamSettings::parseJson(const std::string& json) -> bool {
    Json::Value root;
    Json::Value adc_config;
    Json::Value dac_config;
    Json::Value memory_config;
    Json::Value ext_trig_config;
    Json::Value gpio_config;

    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING errs;
    auto is = std::istringstream(json);
    if (!parseFromStream(builder, is, &root, &errs)) {
        std::cerr << "[CStreamSettings] Error parse json" << errs << std::endl;
        return false;
    }

    if (!root.isObject()) {
        std::cerr << "[CStreamSettings] Error parse json. The document is not an object" << std::endl;
        return false;
    }

    if (root.isMember("adc_streaming")) {
        adc_config = root["adc_streaming"];
    } else {
        std::cerr << "[CStreamSettings] Error parse json. Invalid file" << std::endl;
        return false;
    }

    if (root.isMember("dac_streaming")) {
        dac_config = root["dac_streaming"];
    } else {
        std::cerr << "[CStreamSettings] Error parse json. Invalid file" << std::endl;
        return false;
    }

    if (root.isMember("memory_manager")) {
        memory_config = root["memory_manager"];
    } else {
        std::cerr << "[CStreamSettings] Error parse json. Invalid file" << std::endl;
        return false;
    }

    // Optional: files written before these sections existed keep the defaults.
    if (root.isMember("ext_trigger"))
        ext_trig_config = root["ext_trigger"];
    if (root.isMember("gpio_streaming"))
        gpio_config = root["gpio_streaming"];

    try {
        if (adc_config.isMember("format_sd"))
            setADCFormat(DataFormat::from_string(adc_config["format_sd"].asString()));
        if (adc_config.isMember("data_type_sd"))
            setADCType(DataType::from_string(adc_config["data_type_sd"].asString()));
        if (adc_config.isMember("samples_limit_sd"))
            setADCSamples(adc_config["samples_limit_sd"].asUInt64());
        if (adc_config.isMember("adc_pass_mode"))
            setADCPassMode(PassMode::from_string(adc_config["adc_pass_mode"].asString()));
        if (adc_config.isMember("resolution"))
            setADCResolution(Resolution::from_string(adc_config["resolution"].asString()));
        if (adc_config.isMember("adc_decimation"))
            setADCDecimation(adc_config["adc_decimation"].asUInt());
        if (adc_config.isMember("adc_capture_time"))
            setADCCaptureTime(ADCCaptureTime::from_string(adc_config["adc_capture_time"].asString()));
        if (adc_config.isMember("use_calib"))
            setADCCalibration(State::from_string(adc_config["use_calib"].asString()));
        for (auto i = 1u; i <= 4; i++) {
            if (adc_config.isMember("channel_state_" + std::to_string(i)))
                setADCChannels(i, State::from_string(adc_config["channel_state_" + std::to_string(i)].asString()));
            if (adc_config.isMember("channel_attenuator_" + std::to_string(i)))
                setADCAttenuator(i, Attenuator::from_string(adc_config["channel_attenuator_" + std::to_string(i)].asString()));
            if (adc_config.isMember("channel_ac_dc_" + std::to_string(i)))
                setADCAC_DC(i, AC_DC::from_string(adc_config["channel_ac_dc_" + std::to_string(i)].asString()));
        }
        if (adc_config.isMember("adc_trigger_mode"))
            setADCTriggerMode(State::from_string(adc_config["adc_trigger_mode"].asString()));
        if (adc_config.isMember("adc_trigger_sources"))
            setADCTriggerSources(adc_config["adc_trigger_sources"].asUInt());
        if (adc_config.isMember("adc_trigger_level_low"))
            setADCTriggerLevelLow(adc_config["adc_trigger_level_low"].asInt());
        if (adc_config.isMember("adc_trigger_level_high"))
            setADCTriggerLevelHigh(adc_config["adc_trigger_level_high"].asInt());
        if (adc_config.isMember("adc_trigger_edge"))
            setADCTriggerEdge(TriggerEdge::from_string(adc_config["adc_trigger_edge"].asString()));
        if (adc_config.isMember("adc_event_source"))
            setADCEventSource(EventSource::from_string(adc_config["adc_event_source"].asString()));
        if (adc_config.isMember("adc_trigger_blocks"))
            setADCTriggerBlocks(adc_config["adc_trigger_blocks"].asUInt());

        if (dac_config.isMember("dac_rate"))
            setDACSpeed(dac_config["dac_rate"].asUInt());
        if (dac_config.isMember("file_sd"))
            setDACFile(dac_config["file_sd"].asString());
        if (dac_config.isMember("file_type_sd"))
            setDACFileType(DataFormat::from_string(dac_config["file_type_sd"].asString()));
        if (dac_config.isMember("dac_pass_mode"))
            setDACPassMode(DACPassMode::from_string(dac_config["dac_pass_mode"].asString()));
        if (dac_config.isMember("repeat"))
            setDACRepeat(DACRepeat::from_string(dac_config["repeat"].asString()));
        if (dac_config.isMember("repeatCount"))
            setDACRepeatCount(dac_config["repeatCount"].asUInt());
        for (auto i = 1u; i <= 2; i++) {
            if (dac_config.isMember("channel_gain_" + std::to_string(i)))
                setDACGain(i, DACGain::from_string(dac_config["channel_gain_" + std::to_string(i)].asString()));
            if (dac_config.isMember("dac_trigger_mode_" + std::to_string(i)))
                setDACTriggerMode(i, State::from_string(dac_config["dac_trigger_mode_" + std::to_string(i)].asString()));
        }
        if (dac_config.isMember("dac_trigger_sources"))
            setDACTriggerSources(dac_config["dac_trigger_sources"].asUInt());
        if (dac_config.isMember("dac_event_source"))
            setDACEventSource(EventSource::from_string(dac_config["dac_event_source"].asString()));

        if (memory_config.isMember("block_size"))
            setMemoryBlockSize(memory_config["block_size"].asUInt());
        if (memory_config.isMember("adc_size"))
            setADCSize(memory_config["adc_size"].asUInt());
        if (memory_config.isMember("dac_size"))
            setDACSize(memory_config["dac_size"].asUInt());
        if (memory_config.isMember("gpio_size"))
            setGPIOSize(memory_config["gpio_size"].asUInt());

        if (ext_trig_config.isMember("ext_trig_conditioner"))
            setExtTrigConditioner(State::from_string(ext_trig_config["ext_trig_conditioner"].asString()));
        if (ext_trig_config.isMember("ext_trig_edge"))
            setExtTrigEdge(TriggerEdge::from_string(ext_trig_config["ext_trig_edge"].asString()));
        if (ext_trig_config.isMember("ext_trig_debounce"))
            setExtTrigDebounce(ext_trig_config["ext_trig_debounce"].asUInt());
        if (ext_trig_config.isMember("ext_trig_dio0"))
            setExtTrigDio0(State::from_string(ext_trig_config["ext_trig_dio0"].asString()));

        if (gpio_config.isMember("gpio_in_streaming"))
            setGPIOInStreaming(State::from_string(gpio_config["gpio_in_streaming"].asString()));
        if (gpio_config.isMember("gpio_out_streaming"))
            setGPIOOutStreaming(State::from_string(gpio_config["gpio_out_streaming"].asString()));
        if (gpio_config.isMember("gpio_outputs_p"))
            setGPIOOutputsP(gpio_config["gpio_outputs_p"].asUInt());
        if (gpio_config.isMember("gpio_outputs_n"))
            setGPIOOutputsN(gpio_config["gpio_outputs_n"].asUInt());
        if (gpio_config.isMember("gpio_polarity"))
            setGPIOPolarity(gpio_config["gpio_polarity"].asUInt());
        if (gpio_config.isMember("gpio_decimation"))
            setGPIODecimation(gpio_config["gpio_decimation"].asUInt());
        if (gpio_config.isMember("gpio_rle"))
            setGPIORLE(State::from_string(gpio_config["gpio_rle"].asString()));
        if (gpio_config.isMember("gpio_out_step"))
            setGPIOOutStep(gpio_config["gpio_out_step"].asUInt());
        if (gpio_config.isMember("gpio_pattern_mask"))
            setGPIOPatternMask(gpio_config["gpio_pattern_mask"].asUInt());
        if (gpio_config.isMember("gpio_pattern_value"))
            setGPIOPatternValue(gpio_config["gpio_pattern_value"].asUInt());
        if (gpio_config.isMember("gpio_pattern_pos_edge"))
            setGPIOPatternPosEdge(gpio_config["gpio_pattern_pos_edge"].asUInt());
        if (gpio_config.isMember("gpio_pattern_neg_edge"))
            setGPIOPatternNegEdge(gpio_config["gpio_pattern_neg_edge"].asUInt());
        if (gpio_config.isMember("gpio_trigger_sources"))
            setGPIOTriggerSources(gpio_config["gpio_trigger_sources"].asUInt());
        if (gpio_config.isMember("gpio_event_source"))
            setGPIOEventSource(EventSource::from_string(gpio_config["gpio_event_source"].asString()));
        if (gpio_config.isMember("gpio_in_trigger_mode"))
            setGPIOInTriggerMode(State::from_string(gpio_config["gpio_in_trigger_mode"].asString()));
        if (gpio_config.isMember("gpio_out_trigger_mode"))
            setGPIOOutTriggerMode(State::from_string(gpio_config["gpio_out_trigger_mode"].asString()));
        if (gpio_config.isMember("gpio_trigger_blocks"))
            setGPIOTriggerBlocks(gpio_config["gpio_trigger_blocks"].asUInt());
        return true;
    } catch (...) {
        ERROR_LOG("Error parse json. Invalid file")
        return false;
    }
    return false;
}

auto CStreamSettings::toString() const -> std::string {
    auto hex = [](uint32_t value) -> std::string {
        std::ostringstream os;
        os << "0x" << std::hex << value;
        return os.str();
    };
    auto blocks = [](uint32_t value) -> std::string { return value == 0 ? std::string("Continuous") : std::to_string(value); };

    std::string str = "******************** ADC streaming ********************\n";

    std::string channels = "";
    for (auto i = 1; i <= 4; i++) {
        channels += "\tCh " + std::to_string(i) + ": " + getADCChannels(i).to_string() + " Attenuator: " + getADCAttenuator(i).to_string() +
                    " AD/DC (250-12 only): " + getADCAC_DC(i).to_string() + "\n";
    }

    str += "Channels:\n" + channels;
    str += "Decimation:\t\t" + std::to_string(getADCDecimation()) + "\n";
    str += "Resolution:\t\t" + std::string(getADCResolution().to_string()) + "\n";
    str += "Use calibration:\t\t" + std::string(getADCCalibration().to_string()) + "\n";
    str += "Pass mode:\t\t" + std::string(getADCPassMode().to_string()) + "\n";
    str += "Samples:\t\t" + (getADCSamples() == 0 ? "Unlimited" : std::to_string(getADCSamples())) + " (In file mode)\n";
    str += "Data format:\t\t" + std::string(getADCFormat().to_string()) + " (In file mode)\n";
    str += "Data type:\t\t" + std::string(getADCType().to_string()) + " (In file mode)\n";
    str += "Save capture time:\t\t" + std::string(getADCCaptureTime().to_string()) + "\n";
    str += "Trigger mode:\t\t" + std::string(getADCTriggerMode().to_string()) + "\n";
    str += "Trigger sources:\t\t" + hex(getADCTriggerSources()) + "\n";
    str += "Trigger level:\t\t" + std::to_string(getADCTriggerLevelLow()) + " .. " + std::to_string(getADCTriggerLevelHigh()) + " (ADC codes)\n";
    str += "Trigger edge:\t\t" + std::string(getADCTriggerEdge().to_string()) + "\n";
    str += "Event source:\t\t" + std::string(getADCEventSource().to_string()) + "\n";
    str += "Trigger blocks:\t\t" + blocks(getADCTriggerBlocks()) + "\n";

    str += "\n******************** DAC streaming ********************\n";
    channels = "";
    for (auto i = 1; i <= 2; i++) {
        channels += "\tCh " + std::to_string(i) + " Gain (250-12 only): " + getDACGain(i).to_string() + "\n";
    }
    str += "Channels:\n" + channels;
    str += "Rate:\t\t" + std::to_string(getDACSpeed()) + "\n";
    str += "Pass mode:\t\t" + std::string(getDACPassMode().to_string()) + "\n";
    str += "Path to local file on SD:\t\t" + getDACFile() + "\n";
    str += "Data format:\t\t" + std::string(getDACFileType().to_string()) + " (In file mode)\n";
    str += "Repeat:\t\t" + std::string(getDACRepeat().to_string()) + " (In file mode)\n";
    str += "Repeat count:\t" + std::to_string(getDACRepeatCount()) + " (In file mode)\n";
    str += "Trigger mode:\t\tCh 1: " + std::string(getDACTriggerMode(1).to_string()) + " Ch 2: " + getDACTriggerMode(2).to_string() + "\n";
    str += "Trigger sources:\t\t" + hex(getDACTriggerSources()) + "\n";
    str += "Event source:\t\t" + std::string(getDACEventSource().to_string()) + "\n";

    str += "\n******************** Memory Manager *******************\n";
    str += "Block size:\t\t" + std::to_string(getMemoryBlockSize()) + "\n";
    str += "ADC size:\t\t" + std::to_string(getADCSize()) + "\n";
    str += "DAC size:\t\t" + std::to_string(getDACSize()) + "\n";
    str += "GPIO size:\t\t" + std::to_string(getGPIOSize()) + "\n";

    str += "\n******************* External trigger ******************\n";
    str += "Conditioner:\t\t" + std::string(getExtTrigConditioner().to_string()) + "\n";
    str += "Edge:\t\t" + std::string(getExtTrigEdge().to_string()) + "\n";
    str += "Debounce:\t\t" + (getExtTrigDebounce() == 0 ? std::string("Off") : std::to_string(getExtTrigDebounce()) + " clocks") + "\n";
    str += "DIO0_P input:\t\t" + std::string(getExtTrigDio0().to_string()) + "\n";

    str += "\n******************** GPIO streaming *******************\n";
    str += "Input streaming:\t\t" + std::string(getGPIOInStreaming().to_string()) + "\n";
    str += "Output streaming:\t\t" + std::string(getGPIOOutStreaming().to_string()) + "\n";
    str += "Outputs P/N:\t\t" + hex(getGPIOOutputsP()) + " / " + hex(getGPIOOutputsN()) + "\n";
    str += "Input polarity:\t\t" + hex(getGPIOPolarity()) + "\n";
    str += "Decimation:\t\t" + std::to_string(getGPIODecimation()) + "\n";
    str += "RLE:\t\t" + std::string(getGPIORLE().to_string()) + "\n";
    str += "Output step:\t\t" + std::to_string(getGPIOOutStep()) + "\n";
    str += "Pattern mask/value:\t\t" + hex(getGPIOPatternMask()) + " / " + hex(getGPIOPatternValue()) + "\n";
    str += "Pattern edges pos/neg:\t\t" + hex(getGPIOPatternPosEdge()) + " / " + hex(getGPIOPatternNegEdge()) + "\n";
    str += "Trigger sources:\t\t" + hex(getGPIOTriggerSources()) + "\n";
    str += "Event source:\t\t" + std::string(getGPIOEventSource().to_string()) + "\n";
    str += "Trigger mode in/out:\t\t" + std::string(getGPIOInTriggerMode().to_string()) + " / " + getGPIOOutTriggerMode().to_string() + "\n";
    str += "Trigger blocks:\t\t" + blocks(getGPIOTriggerBlocks()) + "\n";

    return str;
}

auto CStreamSettings::readFromFile(std::string _filename) -> bool {
    std::ifstream file(_filename, std::ios::in);
    if (!file.is_open()) {
        ERROR_LOG("File %s read failed: %s", _filename.c_str(), std::strerror(errno))
        return false;
    }
    resetDefault();
    std::stringstream strStream;
    strStream << file.rdbuf();
    return parseJson(strStream.str());
}

auto CStreamSettings::setADCSamples(uint64_t _samples) -> void {
    m_adcsettings.m_samples = _samples;
}

auto CStreamSettings::getADCSamples() const -> uint64_t {
    return m_adcsettings.m_samples;
}

auto CStreamSettings::setADCFormat(CStreamSettings::DataFormat _format) -> void {
    m_adcsettings.m_format = _format;
}

auto CStreamSettings::getADCFormat() const -> CStreamSettings::DataFormat {
    return m_adcsettings.m_format;
}

auto CStreamSettings::setADCType(DataType _type) -> void {
    m_adcsettings.m_dataType = _type;
}

auto CStreamSettings::getADCType() const -> CStreamSettings::DataType {
    return m_adcsettings.m_dataType;
}

auto CStreamSettings::setADCCaptureTime(ADCCaptureTime _time) -> void {
    m_adcsettings.m_captureTime = _time;
}

auto CStreamSettings::getADCCaptureTime() const -> ADCCaptureTime {
    return m_adcsettings.m_captureTime;
}

auto CStreamSettings::setADCPassMode(CStreamSettings::PassMode _type) -> void {
    m_adcsettings.m_passMode = _type;
}

auto CStreamSettings::getADCPassMode() const -> CStreamSettings::PassMode {
    return m_adcsettings.m_passMode;
}

auto CStreamSettings::setADCChannels(uint8_t _channel, CStreamSettings::State _state) -> bool {
    if (_channel > 0 && _channel <= 4) {
        m_adcsettings.m_channels[_channel - 1] = _state;
        return true;
    }
    return false;
}

auto CStreamSettings::getADCChannels(uint8_t _channel) const -> CStreamSettings::State {
    if (_channel > 0 && _channel <= 4) {
        return m_adcsettings.m_channels[_channel - 1];
    }
    return CStreamSettings::State::OFF;
}

auto CStreamSettings::setADCResolution(CStreamSettings::Resolution _value) -> void {
    m_adcsettings.m_resolution = _value;
}

auto CStreamSettings::getADCResolution() const -> CStreamSettings::Resolution {
    return m_adcsettings.m_resolution;
}

auto CStreamSettings::setADCDecimation(uint32_t _decimation) -> bool {
    // 1 to 65536. Zero used to be accepted although it is not a usable
    // decimation factor, and getHelp() advertised a third range, 1-65535.
    if (_decimation == 0 || _decimation > 1024 * 64)
        return false;
    m_adcsettings.m_decimation = _decimation;
    return true;
}

auto CStreamSettings::getADCDecimation() const -> uint32_t {
    return m_adcsettings.m_decimation;
}

auto CStreamSettings::setADCAttenuator(uint8_t _channel, Attenuator _state) -> bool {
    if (_channel > 0 && _channel <= 4) {
        m_adcsettings.m_attenuator[_channel - 1] = _state;
        return true;
    }
    return false;
}

auto CStreamSettings::getADCAttenuator(uint8_t _channel) const -> Attenuator {
    if (_channel > 0 && _channel <= 4) {
        return m_adcsettings.m_attenuator[_channel - 1];
    }
    return CStreamSettings::Attenuator::A_1_1;
}

auto CStreamSettings::setADCAC_DC(uint8_t _channel, AC_DC _state) -> bool {
    if (_channel > 0 && _channel <= 4) {
        m_adcsettings.m_ac_dc[_channel - 1] = _state;
        return true;
    }
    return false;
}

auto CStreamSettings::getADCAC_DC(uint8_t _channel) const -> AC_DC {
    if (_channel > 0 && _channel <= 4) {
        return m_adcsettings.m_ac_dc[_channel - 1];
    }
    return CStreamSettings::AC_DC::DC;
}

auto CStreamSettings::setADCCalibration(State _calibration) -> void {
    m_adcsettings.m_useCalib = _calibration;
}

auto CStreamSettings::getADCCalibration() const -> State {
    return m_adcsettings.m_useCalib;
}

auto CStreamSettings::setDACFile(std::string _value) -> void {
    m_dacsettings.m_file = _value;
}

auto CStreamSettings::getDACFile() const -> std::string {
    return m_dacsettings.m_file;
}

auto CStreamSettings::setDACFileType(CStreamSettings::DataFormat _value) -> bool {
    if (_value.value != DataFormat::BIN) {
        m_dacsettings.m_file_type = _value;
        return true;
    }
    return false;
}

auto CStreamSettings::getDACFileType() const -> CStreamSettings::DataFormat {
    return m_dacsettings.m_file_type;
}

auto CStreamSettings::setDACGain(uint8_t _channel, DACGain _value) -> bool {
    if (_channel > 0 && _channel <= 2) {
        m_dacsettings.m_gain[_channel - 1] = _value;
        return true;
    }
    return false;
}

auto CStreamSettings::getDACGain(uint8_t _channel) const -> DACGain {
    if (_channel > 0 && _channel <= 2) {
        return m_dacsettings.m_gain[_channel - 1];
    }
    return CStreamSettings::DACGain::X1;
}

auto CStreamSettings::setDACPassMode(CStreamSettings::DACPassMode _value) -> void {
    m_dacsettings.m_passMode = _value;
}

auto CStreamSettings::getDACPassMode() const -> CStreamSettings::DACPassMode {
    return m_dacsettings.m_passMode;
}

auto CStreamSettings::setDACRepeat(DACRepeat _value) -> void {
    m_dacsettings.m_repeat = _value;
}

auto CStreamSettings::getDACRepeat() const -> DACRepeat {
    return m_dacsettings.m_repeat;
}

auto CStreamSettings::setDACSpeed(uint32_t _value) -> bool {
    m_dacsettings.m_rate = _value;
    return true;
}

auto CStreamSettings::getDACSpeed() const -> uint32_t {
    return m_dacsettings.m_rate;
}

auto CStreamSettings::setDACRepeatCount(uint32_t _value) -> void {
    m_dacsettings.m_repeatCount = _value;
}

auto CStreamSettings::getDACRepeatCount() const -> uint32_t {
    return m_dacsettings.m_repeatCount;
}

auto CStreamSettings::setADCTriggerMode(State _value) -> void {
    m_adcsettings.m_trigMode = _value;
}

auto CStreamSettings::getADCTriggerMode() const -> State {
    return m_adcsettings.m_trigMode;
}

auto CStreamSettings::setADCTriggerSources(uint32_t _mask) -> bool {
    if (_mask & ~TRIGGER_SOURCES_ALL)
        return false;
    m_adcsettings.m_trigSources = _mask;
    return true;
}

auto CStreamSettings::getADCTriggerSources() const -> uint32_t {
    return m_adcsettings.m_trigSources;
}

// The order of low and high is not checked here, so that they can be set one by one;
// the consumer has to check low <= high.
auto CStreamSettings::setADCTriggerLevelLow(int32_t _value) -> bool {
    if (_value < std::numeric_limits<int16_t>::min() || _value > std::numeric_limits<int16_t>::max())
        return false;
    m_adcsettings.m_trigLevelLow = _value;
    return true;
}

auto CStreamSettings::getADCTriggerLevelLow() const -> int32_t {
    return m_adcsettings.m_trigLevelLow;
}

auto CStreamSettings::setADCTriggerLevelHigh(int32_t _value) -> bool {
    if (_value < std::numeric_limits<int16_t>::min() || _value > std::numeric_limits<int16_t>::max())
        return false;
    m_adcsettings.m_trigLevelHigh = _value;
    return true;
}

auto CStreamSettings::getADCTriggerLevelHigh() const -> int32_t {
    return m_adcsettings.m_trigLevelHigh;
}

auto CStreamSettings::setADCTriggerEdge(TriggerEdge _value) -> void {
    m_adcsettings.m_trigEdge = _value;
}

auto CStreamSettings::getADCTriggerEdge() const -> TriggerEdge {
    return m_adcsettings.m_trigEdge;
}

auto CStreamSettings::setADCEventSource(EventSource _value) -> void {
    m_adcsettings.m_eventSource = _value;
}

auto CStreamSettings::getADCEventSource() const -> EventSource {
    return m_adcsettings.m_eventSource;
}

auto CStreamSettings::setADCTriggerBlocks(uint32_t _blocks) -> bool {
    if (_blocks > 0xFFFF)
        return false;
    m_adcsettings.m_trigBlocks = _blocks;
    return true;
}

auto CStreamSettings::getADCTriggerBlocks() const -> uint32_t {
    return m_adcsettings.m_trigBlocks;
}

auto CStreamSettings::setDACTriggerMode(uint8_t _channel, State _value) -> bool {
    if (_channel > 0 && _channel <= 2) {
        m_dacsettings.m_trigMode[_channel - 1] = _value;
        return true;
    }
    return false;
}

auto CStreamSettings::getDACTriggerMode(uint8_t _channel) const -> State {
    if (_channel > 0 && _channel <= 2) {
        return m_dacsettings.m_trigMode[_channel - 1];
    }
    return State::OFF;
}

auto CStreamSettings::setDACTriggerSources(uint32_t _mask) -> bool {
    if (_mask & ~TRIGGER_SOURCES_ALL)
        return false;
    m_dacsettings.m_trigSources = _mask;
    return true;
}

auto CStreamSettings::getDACTriggerSources() const -> uint32_t {
    return m_dacsettings.m_trigSources;
}

auto CStreamSettings::setDACEventSource(EventSource _value) -> void {
    m_dacsettings.m_eventSource = _value;
}

auto CStreamSettings::getDACEventSource() const -> EventSource {
    return m_dacsettings.m_eventSource;
}

auto CStreamSettings::setExtTrigConditioner(State _value) -> void {
    m_exttrigsettings.m_conditioner = _value;
}

auto CStreamSettings::getExtTrigConditioner() const -> State {
    return m_exttrigsettings.m_conditioner;
}

auto CStreamSettings::setExtTrigEdge(TriggerEdge _value) -> void {
    m_exttrigsettings.m_edge = _value;
}

auto CStreamSettings::getExtTrigEdge() const -> TriggerEdge {
    return m_exttrigsettings.m_edge;
}

// The widest debounce counter is 16 bits; boards with an 8-bit one are checked by uio_lib.
auto CStreamSettings::setExtTrigDebounce(uint32_t _clocks) -> bool {
    if (_clocks > 0xFFFF)
        return false;
    m_exttrigsettings.m_debounce = _clocks;
    return true;
}

auto CStreamSettings::getExtTrigDebounce() const -> uint32_t {
    return m_exttrigsettings.m_debounce;
}

auto CStreamSettings::setExtTrigDio0(State _value) -> void {
    m_exttrigsettings.m_dio0 = _value;
}

auto CStreamSettings::getExtTrigDio0() const -> State {
    return m_exttrigsettings.m_dio0;
}

auto CStreamSettings::setGPIOInStreaming(State _value) -> void {
    m_gpiosettings.m_inStreaming = _value;
}

auto CStreamSettings::getGPIOInStreaming() const -> State {
    return m_gpiosettings.m_inStreaming;
}

auto CStreamSettings::setGPIOOutStreaming(State _value) -> void {
    m_gpiosettings.m_outStreaming = _value;
}

auto CStreamSettings::getGPIOOutStreaming() const -> State {
    return m_gpiosettings.m_outStreaming;
}

auto CStreamSettings::setGPIOOutputsP(uint32_t _mask) -> bool {
    if (_mask > 0xFF)
        return false;
    m_gpiosettings.m_outputsP = _mask;
    return true;
}

auto CStreamSettings::getGPIOOutputsP() const -> uint32_t {
    return m_gpiosettings.m_outputsP;
}

auto CStreamSettings::setGPIOOutputsN(uint32_t _mask) -> bool {
    if (_mask > 0xFF)
        return false;
    m_gpiosettings.m_outputsN = _mask;
    return true;
}

auto CStreamSettings::getGPIOOutputsN() const -> uint32_t {
    return m_gpiosettings.m_outputsN;
}

auto CStreamSettings::setGPIOPolarity(uint32_t _mask) -> bool {
    if (_mask > 0xFFFF)
        return false;
    m_gpiosettings.m_polarity = _mask;
    return true;
}

auto CStreamSettings::getGPIOPolarity() const -> uint32_t {
    return m_gpiosettings.m_polarity;
}

auto CStreamSettings::setGPIODecimation(uint32_t _decimation) -> bool {
    if (_decimation == 0 || _decimation > 1024 * 128)
        return false;
    m_gpiosettings.m_decimation = _decimation;
    return true;
}

auto CStreamSettings::getGPIODecimation() const -> uint32_t {
    return m_gpiosettings.m_decimation;
}

auto CStreamSettings::setGPIORLE(State _value) -> void {
    m_gpiosettings.m_rle = _value;
}

auto CStreamSettings::getGPIORLE() const -> State {
    return m_gpiosettings.m_rle;
}

auto CStreamSettings::setGPIOOutStep(uint32_t _step) -> bool {
    if (_step == 0)
        return false;
    m_gpiosettings.m_outStep = _step;
    return true;
}

auto CStreamSettings::getGPIOOutStep() const -> uint32_t {
    return m_gpiosettings.m_outStep;
}

auto CStreamSettings::setGPIOPatternMask(uint32_t _mask) -> bool {
    if (_mask > 0xFFFF)
        return false;
    m_gpiosettings.m_patternMask = _mask;
    return true;
}

auto CStreamSettings::getGPIOPatternMask() const -> uint32_t {
    return m_gpiosettings.m_patternMask;
}

auto CStreamSettings::setGPIOPatternValue(uint32_t _value) -> bool {
    if (_value > 0xFFFF)
        return false;
    m_gpiosettings.m_patternValue = _value;
    return true;
}

auto CStreamSettings::getGPIOPatternValue() const -> uint32_t {
    return m_gpiosettings.m_patternValue;
}

auto CStreamSettings::setGPIOPatternPosEdge(uint32_t _mask) -> bool {
    if (_mask > 0xFFFF)
        return false;
    m_gpiosettings.m_patternPosEdge = _mask;
    return true;
}

auto CStreamSettings::getGPIOPatternPosEdge() const -> uint32_t {
    return m_gpiosettings.m_patternPosEdge;
}

auto CStreamSettings::setGPIOPatternNegEdge(uint32_t _mask) -> bool {
    if (_mask > 0xFFFF)
        return false;
    m_gpiosettings.m_patternNegEdge = _mask;
    return true;
}

auto CStreamSettings::getGPIOPatternNegEdge() const -> uint32_t {
    return m_gpiosettings.m_patternNegEdge;
}

auto CStreamSettings::setGPIOTriggerSources(uint32_t _mask) -> bool {
    if (_mask & ~TRIGGER_SOURCES_ALL)
        return false;
    m_gpiosettings.m_trigSources = _mask;
    return true;
}

auto CStreamSettings::getGPIOTriggerSources() const -> uint32_t {
    return m_gpiosettings.m_trigSources;
}

auto CStreamSettings::setGPIOEventSource(EventSource _value) -> void {
    m_gpiosettings.m_eventSource = _value;
}

auto CStreamSettings::getGPIOEventSource() const -> EventSource {
    return m_gpiosettings.m_eventSource;
}

auto CStreamSettings::setGPIOInTriggerMode(State _value) -> void {
    m_gpiosettings.m_inTrigMode = _value;
}

auto CStreamSettings::getGPIOInTriggerMode() const -> State {
    return m_gpiosettings.m_inTrigMode;
}

auto CStreamSettings::setGPIOOutTriggerMode(State _value) -> void {
    m_gpiosettings.m_outTrigMode = _value;
}

auto CStreamSettings::getGPIOOutTriggerMode() const -> State {
    return m_gpiosettings.m_outTrigMode;
}

auto CStreamSettings::setGPIOTriggerBlocks(uint32_t _blocks) -> bool {
    if (_blocks > 0xFFFF)
        return false;
    m_gpiosettings.m_trigBlocks = _blocks;
    return true;
}

auto CStreamSettings::getGPIOTriggerBlocks() const -> uint32_t {
    return m_gpiosettings.m_trigBlocks;
}

auto CStreamSettings::setMemoryBlockSize(uint32_t size) -> void {
    m_memorysettings.m_memoryBlock = size;
}

auto CStreamSettings::getMemoryBlockSize() const -> uint32_t {
    return m_memorysettings.m_memoryBlock;
}

auto CStreamSettings::setADCSize(uint32_t count) -> void {
    m_memorysettings.m_ADCSize = count;
}

auto CStreamSettings::getADCSize() const -> uint32_t {
    return m_memorysettings.m_ADCSize;
}

auto CStreamSettings::setDACSize(uint32_t count) -> void {
    m_memorysettings.m_DACSize = count;
}

auto CStreamSettings::getDACSize() const -> uint32_t {
    return m_memorysettings.m_DACSize;
}

auto CStreamSettings::setGPIOSize(uint32_t count) -> void {
    m_memorysettings.m_GPIOSize = count;
}

auto CStreamSettings::getGPIOSize() const -> uint32_t {
    return m_memorysettings.m_GPIOSize;
}

auto CStreamSettings::setValue(std::string key, std::string value) -> bool {
    try {
        if (key == "format_sd") {
            setADCFormat(DataFormat::from_string(value));
            return true;
        }

        if (key == "data_type_sd") {
            setADCType(DataType::from_string(value));
            return true;
        }

        if (key == "adc_capture_time") {
            setADCCaptureTime(ADCCaptureTime::from_string(value));
            return true;
        }

        if (key == "samples_limit_sd") {
            setADCSamples(to_uint64(value.c_str()));
            return true;
        }

        if (key == "adc_pass_mode") {
            setADCPassMode(PassMode::from_string(value));
            return true;
        }

        if (key == "resolution") {
            setADCResolution(Resolution::from_string(value));
            return true;
        }

        if (key == "adc_decimation") {
            return setADCDecimation(to_uint(value.c_str()));
        }

        if (key == "use_calib") {
            setADCCalibration(State::from_string(value));
            return true;
        }

        for (auto i = 1u; i <= 4; i++) {
            if (key == "channel_state_" + std::to_string(i)) {
                return setADCChannels(i, State::from_string(value));
            }

            if (key == "channel_attenuator_" + std::to_string(i)) {
                return setADCAttenuator(i, Attenuator::from_string(value));
            }

            if (key == "channel_ac_dc_" + std::to_string(i)) {
                return setADCAC_DC(i, AC_DC::from_string(value));
            }
        }

        if (key == "adc_trigger_mode") {
            setADCTriggerMode(State::from_string(value));
            return true;
        }

        if (key == "adc_trigger_sources") {
            return setADCTriggerSources(to_uint(value.c_str()));
        }

        if (key == "adc_trigger_level_low") {
            return setADCTriggerLevelLow(to_int(value.c_str()));
        }

        if (key == "adc_trigger_level_high") {
            return setADCTriggerLevelHigh(to_int(value.c_str()));
        }

        if (key == "adc_trigger_edge") {
            setADCTriggerEdge(TriggerEdge::from_string(value));
            return true;
        }

        if (key == "adc_event_source") {
            setADCEventSource(EventSource::from_string(value));
            return true;
        }

        if (key == "adc_trigger_blocks") {
            return setADCTriggerBlocks(to_uint(value.c_str()));
        }

        if (key == "dac_rate") {
            return setDACSpeed(to_uint(value.c_str()));
        }

        if (key == "file_sd") {
            setDACFile((value));
            return true;
        }

        if (key == "file_type_sd") {
            // Report the setter's answer, the way adc_decimation does: BIN is
            // not a valid DAC playback format and the write is refused, which
            // this used to hide behind an unconditional true.
            return setDACFileType(DataFormat::from_string(value));
        }

        if (key == "dac_pass_mode") {
            setDACPassMode(DACPassMode::from_string(value));
            return true;
        }

        if (key == "repeat") {
            setDACRepeat(DACRepeat::from_string(value));
            return true;
        }

        if (key == "repeatCount") {
            setDACRepeatCount(to_uint(value.c_str()));
            return true;
        }

        for (auto i = 1u; i <= 2; i++) {
            if (key == "channel_gain_" + std::to_string(i)) {
                return setDACGain(i, DACGain::from_string(value));
            }

            if (key == "dac_trigger_mode_" + std::to_string(i)) {
                return setDACTriggerMode(i, State::from_string(value));
            }
        }

        if (key == "dac_trigger_sources") {
            return setDACTriggerSources(to_uint(value.c_str()));
        }

        if (key == "dac_event_source") {
            setDACEventSource(EventSource::from_string(value));
            return true;
        }

        if (key == "block_size") {
            setMemoryBlockSize(to_uint(value.c_str()));
            return true;
        }

        if (key == "adc_size") {
            setADCSize(to_uint(value.c_str()));
            return true;
        }

        if (key == "dac_size") {
            setDACSize(to_uint(value.c_str()));
            return true;
        }

        if (key == "gpio_size") {
            setGPIOSize(to_uint(value.c_str()));
            return true;
        }

        if (key == "ext_trig_conditioner") {
            setExtTrigConditioner(State::from_string(value));
            return true;
        }

        if (key == "ext_trig_edge") {
            setExtTrigEdge(TriggerEdge::from_string(value));
            return true;
        }

        if (key == "ext_trig_debounce") {
            return setExtTrigDebounce(to_uint(value.c_str()));
        }

        if (key == "ext_trig_dio0") {
            setExtTrigDio0(State::from_string(value));
            return true;
        }

        if (key == "gpio_in_streaming") {
            setGPIOInStreaming(State::from_string(value));
            return true;
        }

        if (key == "gpio_out_streaming") {
            setGPIOOutStreaming(State::from_string(value));
            return true;
        }

        if (key == "gpio_outputs_p") {
            return setGPIOOutputsP(to_uint(value.c_str()));
        }

        if (key == "gpio_outputs_n") {
            return setGPIOOutputsN(to_uint(value.c_str()));
        }

        if (key == "gpio_polarity") {
            return setGPIOPolarity(to_uint(value.c_str()));
        }

        if (key == "gpio_decimation") {
            return setGPIODecimation(to_uint(value.c_str()));
        }

        if (key == "gpio_rle") {
            setGPIORLE(State::from_string(value));
            return true;
        }

        if (key == "gpio_out_step") {
            return setGPIOOutStep(to_uint(value.c_str()));
        }

        if (key == "gpio_pattern_mask") {
            return setGPIOPatternMask(to_uint(value.c_str()));
        }

        if (key == "gpio_pattern_value") {
            return setGPIOPatternValue(to_uint(value.c_str()));
        }

        if (key == "gpio_pattern_pos_edge") {
            return setGPIOPatternPosEdge(to_uint(value.c_str()));
        }

        if (key == "gpio_pattern_neg_edge") {
            return setGPIOPatternNegEdge(to_uint(value.c_str()));
        }

        if (key == "gpio_trigger_sources") {
            return setGPIOTriggerSources(to_uint(value.c_str()));
        }

        if (key == "gpio_event_source") {
            setGPIOEventSource(EventSource::from_string(value));
            return true;
        }

        if (key == "gpio_in_trigger_mode") {
            setGPIOInTriggerMode(State::from_string(value));
            return true;
        }

        if (key == "gpio_out_trigger_mode") {
            setGPIOOutTriggerMode(State::from_string(value));
            return true;
        }

        if (key == "gpio_trigger_blocks") {
            return setGPIOTriggerBlocks(to_uint(value.c_str()));
        }

    } catch (...) {
        return false;
    }
    return false;
}

auto CStreamSettings::getValue(std::string key) -> std::string {
    try {
        if (key == "format_sd") {
            return getADCFormat().name();
        }

        if (key == "data_type_sd") {
            return getADCType().name();
        }

        if (key == "adc_capture_time") {
            return getADCCaptureTime().name();
        }

        if (key == "samples_limit_sd") {
            return std::to_string(getADCSamples());
        }

        if (key == "adc_pass_mode") {
            return getADCPassMode().name();
        }

        if (key == "resolution") {
            return getADCResolution().name();
        }

        if (key == "adc_decimation") {
            return std::to_string(getADCDecimation());
        }

        if (key == "use_calib") {
            return getADCCalibration().name();
        }

        for (auto i = 1u; i <= 4; i++) {
            if (key == "channel_state_" + std::to_string(i)) {
                return getADCChannels(i).name();
            }

            if (key == "channel_attenuator_" + std::to_string(i)) {
                return getADCAttenuator(i).name();
            }

            if (key == "channel_ac_dc_" + std::to_string(i)) {
                return getADCAC_DC(i).name();
            }
        }

        if (key == "adc_trigger_mode") {
            return getADCTriggerMode().name();
        }

        if (key == "adc_trigger_sources") {
            return std::to_string(getADCTriggerSources());
        }

        if (key == "adc_trigger_level_low") {
            return std::to_string(getADCTriggerLevelLow());
        }

        if (key == "adc_trigger_level_high") {
            return std::to_string(getADCTriggerLevelHigh());
        }

        if (key == "adc_trigger_edge") {
            return getADCTriggerEdge().name();
        }

        if (key == "adc_event_source") {
            return getADCEventSource().name();
        }

        if (key == "adc_trigger_blocks") {
            return std::to_string(getADCTriggerBlocks());
        }

        if (key == "dac_rate") {
            return std::to_string(getDACSpeed());
        }

        if (key == "file_sd") {
            return getDACFile();
        }

        if (key == "file_type_sd") {
            return getDACFileType().name();
        }

        if (key == "dac_pass_mode") {
            return getDACPassMode().name();
        }

        if (key == "repeat") {
            return getDACRepeat().name();
        }

        if (key == "repeatCount") {
            return std::to_string(getDACRepeatCount());
        }

        for (auto i = 1u; i <= 2; i++) {
            if (key == "channel_gain_" + std::to_string(i)) {
                return getDACGain(i).name();
            }

            if (key == "dac_trigger_mode_" + std::to_string(i)) {
                return getDACTriggerMode(i).name();
            }
        }

        if (key == "dac_trigger_sources") {
            return std::to_string(getDACTriggerSources());
        }

        if (key == "dac_event_source") {
            return getDACEventSource().name();
        }

        if (key == "block_size") {
            return std::to_string(getMemoryBlockSize());
        }

        if (key == "adc_size") {
            return std::to_string(getADCSize());
            ;
        }

        if (key == "dac_size") {
            return std::to_string(getDACSize());
            ;
        }

        if (key == "gpio_size") {
            return std::to_string(getGPIOSize());
            ;
        }

        if (key == "ext_trig_conditioner") {
            return getExtTrigConditioner().name();
        }

        if (key == "ext_trig_edge") {
            return getExtTrigEdge().name();
        }

        if (key == "ext_trig_debounce") {
            return std::to_string(getExtTrigDebounce());
        }

        if (key == "ext_trig_dio0") {
            return getExtTrigDio0().name();
        }

        if (key == "gpio_in_streaming") {
            return getGPIOInStreaming().name();
        }

        if (key == "gpio_out_streaming") {
            return getGPIOOutStreaming().name();
        }

        if (key == "gpio_outputs_p") {
            return std::to_string(getGPIOOutputsP());
        }

        if (key == "gpio_outputs_n") {
            return std::to_string(getGPIOOutputsN());
        }

        if (key == "gpio_polarity") {
            return std::to_string(getGPIOPolarity());
        }

        if (key == "gpio_decimation") {
            return std::to_string(getGPIODecimation());
        }

        if (key == "gpio_rle") {
            return getGPIORLE().name();
        }

        if (key == "gpio_out_step") {
            return std::to_string(getGPIOOutStep());
        }

        if (key == "gpio_pattern_mask") {
            return std::to_string(getGPIOPatternMask());
        }

        if (key == "gpio_pattern_value") {
            return std::to_string(getGPIOPatternValue());
        }

        if (key == "gpio_pattern_pos_edge") {
            return std::to_string(getGPIOPatternPosEdge());
        }

        if (key == "gpio_pattern_neg_edge") {
            return std::to_string(getGPIOPatternNegEdge());
        }

        if (key == "gpio_trigger_sources") {
            return std::to_string(getGPIOTriggerSources());
        }

        if (key == "gpio_event_source") {
            return getGPIOEventSource().name();
        }

        if (key == "gpio_in_trigger_mode") {
            return getGPIOInTriggerMode().name();
        }

        if (key == "gpio_out_trigger_mode") {
            return getGPIOOutTriggerMode().name();
        }

        if (key == "gpio_trigger_blocks") {
            return std::to_string(getGPIOTriggerBlocks());
        }

    } catch (...) {
        return "ERROR";
    }
    return "ERROR";
}

auto CStreamSettings::getHelp() -> std::string {
    auto concat = [](const char* const* name, int count) -> std::string {
        std::string s = "";
        bool first = true;
        for (int i = 0; i < count; i++) {
            if (!first)
                s += ",";
            s += std::string(name[i]);
            first = false;
        }
        return s;
    };

    const std::string states = concat(CStreamSettings::State::names(), CStreamSettings::State::count);
    const std::string events = concat(CStreamSettings::EventSource::names(), CStreamSettings::EventSource::count) +
                               ". The block whose start/stop/trigger events are followed, OWN - its own.\n";
    const std::string sources = "An unsigned integer bit mask: 0-63. 1 DAC A, 2 DAC B, 4 ADC CH1, 8 ADC CH2, 16 GPIO, 32 external.\n";

    std::string s = "";
    s += "format_sd\t\t: " + concat(CStreamSettings::DataFormat::names(), CStreamSettings::DataFormat::count) + "\n";
    s += "data_type_sd\t\t: " + concat(CStreamSettings::DataType::names(), CStreamSettings::DataType::count) + "\n";
    s += "samples_limit_sd\t: An unsigned integer value. 0 - Disables write limit.\n";
    s += "adc_pass_mode\t\t: " + concat(CStreamSettings::PassMode::names(), CStreamSettings::PassMode::count) + "\n";
    s += "resolution\t\t: " + concat(CStreamSettings::Resolution::names(), CStreamSettings::Resolution::count) + "\n";
    s += "adc_decimation\t\t: An unsigned integer value: 1-65536.\n";
    s += "use_calib\t\t: " + concat(CStreamSettings::State::names(), CStreamSettings::State::count) + "\n";
    s += "adc_capture_time\t\t: " + concat(CStreamSettings::ADCCaptureTime::names(), CStreamSettings::ADCCaptureTime::count) + "\n";
    for (auto i = 1u; i <= 4; i++) {
        s += "channel_state_" + std::to_string(i) + "\t\t: " + concat(CStreamSettings::State::names(), CStreamSettings::State::count) + "\n";
        s += "channel_attenuator_" + std::to_string(i) + "\t: " + concat(CStreamSettings::Attenuator::names(), CStreamSettings::Attenuator::count) + "\n";
        s += "channel_ac_dc_" + std::to_string(i) + "\t\t: " + concat(CStreamSettings::AC_DC::names(), CStreamSettings::AC_DC::count) + "\n";
    }
    s += "adc_trigger_mode\t: " + states + ". ON - the ADC waits for a trigger from adc_trigger_sources.\n";
    s += "adc_trigger_sources\t: " + sources;
    s += "adc_trigger_level_low\t: A signed integer value: -32768-32767, ADC codes. Must not be greater than adc_trigger_level_high.\n";
    s += "adc_trigger_level_high\t: A signed integer value: -32768-32767, ADC codes.\n";
    s += "adc_trigger_edge\t: " + concat(CStreamSettings::TriggerEdge::names(), CStreamSettings::TriggerEdge::count) + "\n";
    s += "adc_event_source\t: " + events;
    s += "adc_trigger_blocks\t: An unsigned integer value: 0-65535. Blocks captured after a trigger, 0 - continuous.\n";
    s += "dac_rate\t\t: An unsigned integer value. Indicates the rate for signal generation. The maximum value should not be greater than "
         "the base frequency of the FPGA.\n";
    s += "file_sd\t\t\t: Path to the file on the memory card in RP that will be used to generate the signal.\n";
    s += "file_type_sd\t\t: " + concat(CStreamSettings::DataFormat::names(), CStreamSettings::DataFormat::count) + "\n";
    s += "dac_pass_mode\t\t: " + concat(CStreamSettings::DACPassMode::names(), CStreamSettings::DACPassMode::count) + "\n";
    s += "repeat\t\t\t: " + concat(CStreamSettings::DACRepeat::names(), CStreamSettings::DACRepeat::count) + "\n";
    s += "repeatCount\t\t: An unsigned integer value.\n";
    for (auto i = 1u; i <= 2; i++) {
        s += "channel_gain_" + std::to_string(i) + "\t\t: " + concat(CStreamSettings::DACGain::names(), CStreamSettings::DACGain::count) + "\n";
        s += "dac_trigger_mode_" + std::to_string(i) + "\t: " + states + ". ON - the channel waits for a trigger from dac_trigger_sources.\n";
    }
    s += "dac_trigger_sources\t: " + sources;
    s += "dac_event_source\t: " + events;

    s += "block_size\t\t: An unsigned integer value. The value must be less than the reserved memory in the system divided by 16. By "
         "default, 32MB is allocated, the allowed maximum value is 2MB.\n";
    s += "adc_size\t\t: Minimum value 12 * (block_size + " + std::to_string(DataLib::sizeHeader()) + ")\n";
    s += "dac_size\t\t: Minimum value 12 * (block_size + " + std::to_string(DataLib::sizeHeader()) + ")\n";
    s += "gpio_size\t\t: Minimum value 12 * (block_size + " + std::to_string(DataLib::sizeHeader()) + ")\n";

    s += "ext_trig_conditioner\t: " + states + ". ON - the external trigger goes through the edge detector and the debounce.\n";
    s += "ext_trig_edge\t\t: " + concat(CStreamSettings::TriggerEdge::names(), CStreamSettings::TriggerEdge::count) + "\n";
    s += "ext_trig_debounce\t: An unsigned integer value: 0-65535 clocks, 0 - off. Some boards allow up to 255.\n";
    s += "ext_trig_dio0\t\t: " + states + ". ON - DIO0_P drives the external trigger.\n";

    s += "gpio_in_streaming\t: " + states + "\n";
    s += "gpio_out_streaming\t: " + states + "\n";
    s += "gpio_outputs_p\t\t: An unsigned integer value: 0-255. Bit mask of DIO_P lines driven by the output stream.\n";
    s += "gpio_outputs_n\t\t: An unsigned integer value: 0-255. Bit mask of DIO_N lines driven by the output stream.\n";
    s += "gpio_polarity\t\t: An unsigned integer value: 0-65535. Inverted input lines, bits 7:0 P, 15:8 N.\n";
    s += "gpio_decimation\t\t: An unsigned integer value: 1-131072.\n";
    s += "gpio_rle\t\t: " + states + "\n";
    s += "gpio_out_step\t\t: An unsigned integer value, not 0.\n";
    s += "gpio_pattern_mask\t: An unsigned integer value: 0-65535. Lines of the pattern trigger, bits 7:0 P, 15:8 N.\n";
    s += "gpio_pattern_value\t: An unsigned integer value: 0-65535. Levels of the masked lines.\n";
    s += "gpio_pattern_pos_edge\t: An unsigned integer value: 0-65535. Lines that must rise.\n";
    s += "gpio_pattern_neg_edge\t: An unsigned integer value: 0-65535. Lines that must fall.\n";
    s += "gpio_trigger_sources\t: " + sources;
    s += "gpio_event_source\t: " + events;
    s += "gpio_in_trigger_mode\t: " + states + "\n";
    s += "gpio_out_trigger_mode\t: " + states + "\n";
    s += "gpio_trigger_blocks\t: An unsigned integer value: 0-65535. Blocks captured after a trigger, 0 - continuous.\n";

    return s;
}

auto CStreamSettings::setDACDirPath(std::string& _path) -> void {
    g_dirPath = _path;
}

auto CStreamSettings::getDACDirPath() -> std::string {
#ifdef RP_PLATFORM
    if (g_dirPath.empty()) {
        g_dirPath = FILE_PATH;
    }

    std::error_code ec;
    std::filesystem::create_directories(g_dirPath, ec);
    if (ec) {
        return FILE_PATH; 
    }

    return g_dirPath;
#else
    return ".";
#endif
}

auto CStreamSettings::getDACFiles() -> std::string {
    std::string s = "";
    for (const auto& entry : std::filesystem::directory_iterator(getDACDirPath()))
        s += entry.path().filename().generic_string() + "\n";
    return s;
}
