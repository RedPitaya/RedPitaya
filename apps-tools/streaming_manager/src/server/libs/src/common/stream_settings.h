#pragma once
#include <string>

class CStreamSettings {

public:
    enum Protocol{
        TCP = 0,
        UDP = 1
    };

    enum DataFormat{
        WAV =  0,
        TDMS = 1,
        CSV =  2
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


    CStreamSettings();
    auto reset() -> void;
    auto isSetted() -> bool;
    auto setValue(std::string key,std::string value) -> bool;
    auto setValue(std::string key,uint32_t value) -> bool;
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

private:
    bool m_Bport;
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
    std::string      m_port;
    Protocol    m_protocol;
    uint32_t    m_samples;
    DataFormat  m_format;
    DataType    m_type;
    SaveType    m_saveType;
    Channel     m_channels;
    Resolution  m_res;
    uint32_t    m_decimation;
    Attenuator  m_attenuator;
    AC_DC       m_ac_dc;
    bool        m_Bac_dc;

};

