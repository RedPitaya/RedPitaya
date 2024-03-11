#ifndef SETTINGS_LIB_STREAMSETTINGS_H
#define SETTINGS_LIB_STREAMSETTINGS_H

#include <string>
#include <map>

class CStreamSettings {

public:
    enum Protocol{
        TCP = 0,
        UDP = 1
    };

    enum DataFormat{
        UNDEF = -1,
        WAV   =  0,
        TDMS  =  1,
        BIN   =  2
    };

    enum DataType{
        RAW  = 0,
        VOLT = 1
    };

    enum Channel{
        CH1  = 0x1,
        CH2  = 0x2,
        CH3  = 0x4,
        CH4  = 0x8
    };

    enum Resolution{
        BIT_8 = 0,
        BIT_16 = 1
    };

    enum Attenuator{
        A_1_1  = 0,
        A_1_20 = 1
    };

    enum AC_DC{
        AC = 0,
        DC = 1
    };

    enum SaveType{
        NET = 0,
        FILE = 1
    };

    enum DACType{
        DAC_NET = 0,
        DAC_FILE = 1
    };

    enum DACRepeat{
        DAC_REP_OFF = -1,
        DAC_REP_INF = -2,
        DAC_REP_ON  =  0
    };

    enum DACGain{
        X1 = 0,
        X5 = 1
    };

    enum LOOPBACKChannels{
        ONE = 0,
        TWO = 1
    };

    enum LOOPBACKMode{
        DD  = 0
    };

    CStreamSettings();
    ~CStreamSettings();
    CStreamSettings (const CStreamSettings&);
    CStreamSettings& operator= (const CStreamSettings&);
    auto resetDefault() -> void;
    auto reset() -> void;
    auto isSetted() -> bool;
    auto copy(const CStreamSettings &) -> void;
    auto setValue(std::string key,std::string value) -> bool;
    auto setValue(std::string key,int64_t value) -> bool;
    auto setValue(std::string key,double value) -> bool;

    auto writeToFile(std::string _filename) -> bool;
    auto readFromFile(std::string _filename) -> bool;
    auto getJson()-> std::string;
    auto String()-> std::string;
    // auto StringStreaming()-> std::string;

    auto setPort(std::string _port) -> void;
    auto getPort() const -> std::string;
    auto setProtocol(Protocol _port) -> void;
    auto getProtocol() const -> Protocol;
    auto setSamples(int32_t _samples) -> void;
    auto getSamples() const -> int32_t;
    auto setFormat(DataFormat _format) -> void;
    auto getFormat() const -> DataFormat;
    auto setType(DataType _type) -> void;
    auto getType() const -> DataType;
    auto setSaveType(SaveType _type) -> void;
    auto getSaveType() const -> SaveType;

    auto setChannels(uint8_t _value) -> void;
    auto setChannels(Channel _channel,bool _enable) -> void;
    auto getChannels() const -> uint8_t;
    auto getChannels(Channel _channel) const -> bool;

    auto setResolution(Resolution _resolution) -> void;
    auto getResolution() const -> Resolution;

    auto setDecimation(uint32_t _decimation) -> void;
    auto getDecimation() const -> uint32_t;

    auto setAttenuator(uint8_t _value) -> void;
    auto setAttenuator(Channel _channel,Attenuator _attenuator) -> void;
    auto getAttenuator() const -> uint8_t;
    auto getAttenuator(Channel _channel) const -> Attenuator;

    auto setCalibration(bool _calibration) -> void;
    auto getCalibration() const -> bool;

    auto setAC_DC(uint8_t _value) -> void;
    auto setAC_DC(Channel _channel,AC_DC _value) -> void;
    auto getAC_DC() const -> uint8_t;
    auto getAC_DC(Channel _channel) const -> AC_DC;

    auto setDACFile(std::string _value) -> void;
    auto getDACFile() const -> std::string;
    auto setDACFileType(DataFormat _value) -> void;
    auto getDACFileType() const -> DataFormat;

    auto setDACGain(uint8_t _value) -> void;
    auto setDACGain(Channel _channel,DACGain _value) -> void;
    auto getDACGain() const -> uint8_t;
    auto getDACGain(Channel _channel) const -> DACGain;

    auto setDACMode(DACType _value) -> void;
    auto getDACMode() const -> DACType;
    auto setDACRepeat(DACRepeat _value) -> void;
    auto getDACRepeat() const -> DACRepeat;
    auto setDACHz(uint32_t _value) -> void;
    auto getDACHz() const -> uint32_t;
    auto setDACRepeatCount(uint32_t _value) -> void;
    auto getDACRepeatCount() const -> uint32_t;
    auto getDACPort() const -> std::string;
    auto setDACPort(std::string _port) -> void;
    auto getDACMemoryUsage() const -> int64_t;
    auto setDACMemoryUsage(int64_t _value) -> void;

    auto getLoopbackTimeout() const -> uint32_t;
    auto setLoopbackTimeout(uint32_t value) -> void;
    auto getLoopbackSpeed() const -> int32_t;
    auto setLoopbackSpeed(int32_t value) -> void;
    auto getLoopbackMode() const -> LOOPBACKMode;
    auto setLoopbackMode(LOOPBACKMode mode) -> void;
    auto getLoopbackChannels() const -> LOOPBACKChannels;
    auto setLoopbackChannels(LOOPBACKChannels channels) -> void;

    static auto checkChannel(uint32_t _value, uint32_t _channel_index) -> bool;

private:

    CStreamSettings(CStreamSettings&&) = delete;
    CStreamSettings& operator=(CStreamSettings&&) = delete;

    std::string     m_port;
    std::string     m_dac_file;
    Protocol        m_protocol;
    uint32_t        m_samples;
    DataFormat      m_format;
    DataType        m_type;
    SaveType        m_saveType;
    uint8_t         m_channels;
    Resolution      m_res;
    uint32_t        m_decimation;
    uint8_t         m_attenuator;
    bool            m_calib;
    uint8_t         m_ac_dc;

    uint8_t         m_dac_gain;
    DataFormat      m_dac_file_type;
    DACType         m_dac_mode;
    DACRepeat       m_dac_repeat;
    std::string     m_dac_port;
    int64_t         m_dac_memoryUsage;
    uint32_t        m_dac_repeatCount;
    uint32_t        m_dac_speed_Hz;

    uint32_t         m_loopback_timeout;
    int32_t          m_loopback_speed_Hz;
    LOOPBACKMode     m_loopback_mode;
    LOOPBACKChannels m_loopback_channels;

    std::map<std::string, bool> m_var_changed;
};

#endif
