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

    enum LOOPBACKChannels{
        ONE = 0,
        TWO = 1
    };

    enum LOOPBACKMode{
        DD  = 0
    };

    enum RPBoardMode{
        MASTER = 0,
        SLAVE = 1
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
    auto StringStreaming()-> std::string;

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
    auto setChannels(Channel _channels) -> void;
    auto getChannels() const -> Channel;
    auto setResolution(Resolution _resolution) -> void;
    auto getResolution() const -> Resolution;
    auto setDecimation(uint32_t _decimation) -> void;
    auto getDecimation() const -> uint32_t;
    auto setBoardMode(RPBoardMode _mode) -> void;
    auto getBoardMode() const -> RPBoardMode;

    auto setAttenuator(Attenuator _attenuator) -> void;
    auto getAttenuator() const -> Attenuator;
    auto setCalibration(bool _calibration) -> void;
    auto getCalibration() const -> bool;
    auto setAC_DC(AC_DC _value) -> void;
    auto getAC_DC() const -> AC_DC;

    auto setDACFile(std::string _value) -> void;
    auto getDACFile() const -> std::string;
    auto setDACFileType(DataFormat _value) -> void;
    auto getDACFileType() const -> DataFormat;
    auto setDACGain(DACGain _value) -> void;
    auto getDACGain() const -> DACGain;
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
    Channel         m_channels;
    Resolution      m_res;
    uint32_t        m_decimation;
    Attenuator      m_attenuator;
    bool            m_calib;
    AC_DC           m_ac_dc;

    DACGain         m_dac_gain;
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

    RPBoardMode      m_rp_board_mode;

    std::map<std::string, bool> m_var_changed;
};

#endif
