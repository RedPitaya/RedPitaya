#pragma once
#include <string>

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
        CSV   =  2
    };

    enum DataType{
        RAW  = 1,
        VOLT = 2
    };

    enum Channel{
        CH1  = 1,
        CH2  = 2,
        BOTH = 3
    };

    enum Resolution{
        BIT_8 = 1,
        BIT_16 = 2
    };

    enum Attenuator{
        A_1_1  = 1,
        A_1_20 = 2
    };

    enum AC_DC{
        AC = 1,
        DC = 2
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


    CStreamSettings();
    auto reset() -> void;
    auto isSetted() -> bool;
    auto setValue(std::string key,std::string value) -> bool;
    auto setValue(std::string key,int64_t value) -> bool;
    auto setValue(std::string key,double value) -> bool;

    auto writeToFile(std::string _filename) -> bool;
    auto readFromFile(std::string _filename) -> bool;
    auto getJson()-> std::string;
    auto String()-> std::string;

    auto setPort(std::string _port) -> void;
    auto getPort() -> std::string;
    auto setProtocol(Protocol _port) -> void;
    auto getProtocol() -> Protocol;
    auto setSamples(int32_t _samples) -> void;
    auto getSamples() -> int32_t;
    auto setFormat(DataFormat _format) -> void;
    auto getFormat() -> DataFormat;
    auto setType(DataType _type) -> void;
    auto getType() -> DataType;
    auto setSaveType(SaveType _type) -> void;
    auto getSaveType() -> SaveType;
    auto setChannels(Channel _channels) -> void;
    auto getChannels() -> Channel;
    auto setResolution(Resolution _resolution) -> void;
    auto getResolution() -> Resolution;
    auto setDecimation(uint32_t _decimation) -> void;
    auto getDecimation() -> uint32_t;

    auto setAttenuator(Attenuator _attenuator) -> void;
    auto getAttenuator() -> Attenuator;
    auto setCalibration(bool _calibration) -> void;
    auto getCalibration() -> bool;
    auto setAC_DC(AC_DC _value) -> void;
    auto getAC_DC() -> AC_DC;

    auto setDACFile(std::string _value) -> void;
    auto getDACFile() -> std::string;
    auto setDACFileType(DataFormat _value) -> void;
    auto getDACFileType() -> DataFormat;
    auto setDACGain(DACGain _value) -> void;
    auto getDACGain() -> DACGain;
    auto setDACMode(DACType _value) -> void;
    auto getDACMode() -> DACType;
    auto setDACRepeat(DACRepeat _value) -> void;
    auto getDACRepeat() -> DACRepeat;
    auto setDACRepeatCount(uint32_t _value) -> void;
    auto getDACRepeatCount() -> uint32_t;
    auto getDACPort() -> std::string;
    auto setDACPort(std::string _port) -> void;
    auto getDACMemoryUsage() -> int64_t;
    auto setDACMemoryUsage(int64_t _value) -> void;


private:
    bool m_Bport;
    bool m_Bdac_file;
    bool m_Bprotocol;
    bool m_Bsamples;
    bool m_Bformat;
    bool m_Btype;
    bool m_BsaveType;
    bool m_Bchannels;
    bool m_Bres;
    bool m_Bdecimation;
    bool m_calib;
    bool m_Battenuator;
    bool m_Bcalib;
    bool m_Bac_dc;
    bool m_Bdac_gain;
    bool m_Bdac_file_type;
    bool m_Bdac_mode;
    bool m_Bdac_repeat;
    bool m_Bdac_port;
    bool m_Bdac_memoryUsage;
    bool m_Bdac_repeatCount;

    std::string     m_port;
    std::string     m_dac_file;
    Protocol        m_protocol;
    uint32_t        m_samples;
    DataFormat      m_format;
    DataType        m_type;
    SaveType        m_saveType;
    Channel         m_channels;
    Resolution      m_res;
    uint32_t        m_decimation;
    Attenuator      m_attenuator;
    AC_DC           m_ac_dc;
    DACGain         m_dac_gain;
    DataFormat      m_dac_file_type;
    DACType         m_dac_mode;
    DACRepeat       m_dac_repeat;
    std::string     m_dac_port;
    int64_t         m_dac_memoryUsage;
    uint32_t        m_dac_repeatCount;
};

