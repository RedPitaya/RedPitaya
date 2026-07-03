/**
 * $Id$
 *
 * @brief Red Pitaya data formatter.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_FORMATTER_H__
#define __RP_FORMATTER_H__

#include <stdint.h>
#include <memory>

namespace rp_formatter_api {

typedef enum {
    RP_F_WAV = 0,
    RP_F_TDMS = 1,
    RP_F_CSV = 2,
} rp_mode_t;

typedef enum { RP_F_LittleEndian = 0, RP_F_BigEndian = 1 } rp_endianness_t;

typedef enum {
    RP_F_CH1 = 0,
    RP_F_CH2 = 1,
    RP_F_CH3 = 2,
    RP_F_CH4 = 3,
    RP_F_CH5 = 4,
    RP_F_CH6 = 5,
    RP_F_CH7 = 6,
    RP_F_CH8 = 7,
    RP_F_CH9 = 8,
    RP_F_CH10 = 9,
    RP_F_TIME = 10,
    RP_F_INDEX = 11
} rp_channel_t;

class CFormatter {

   public:
    CFormatter(rp_mode_t _mode, uint32_t _oscRate);
    ~CFormatter();

    auto setEndiannes(rp_endianness_t _endiannes) -> bool;
    auto resetWriter() -> void;

    auto clearBuffer() -> void;
    auto setChannel(rp_channel_t _channel, uint8_t* _buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannel(rp_channel_t _channel, uint16_t* _buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannel(rp_channel_t _channel, int32_t* _buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannel(rp_channel_t _channel, uint32_t* _buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannel(rp_channel_t _channel, int64_t* _buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannel(rp_channel_t _channel, uint64_t* _buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannel(rp_channel_t _channel, float* _buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannel(rp_channel_t _channel, double* _buffer, int _samplesCount, std::string _name = "") -> void;

    auto setChannelUI8NP(rp_channel_t _channel, uint8_t* _np_buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannelUI16NP(rp_channel_t _channel, uint16_t* _np_buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannelI32NP(rp_channel_t _channel, int32_t* _np_buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannelUI32NP(rp_channel_t _channel, uint32_t* _np_buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannelI64NP(rp_channel_t _channel, int64_t* _np_buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannelUI64NP(rp_channel_t _channel, uint64_t* _np_buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannelFNP(rp_channel_t _channel, float* _np_buffer, int _samplesCount, std::string _name = "") -> void;
    auto setChannelDNP(rp_channel_t _channel, double* _np_buffer, int _samplesCount, std::string _name = "") -> void;

    auto openFile(std::string _path) -> bool;
    auto closeFile() -> bool;
    auto isOpenFile() -> bool;

    auto writeToFile() -> bool;
    auto writeToStream(std::iostream* _memory) -> bool;

    auto getMaxSamples() -> size_t;

   private:
    CFormatter(const CFormatter&) = delete;
    CFormatter(CFormatter&&) = delete;
    CFormatter& operator=(const CFormatter&) = delete;
    CFormatter& operator=(CFormatter&&) = delete;

    struct Impl;
    // Pointer to the internal implementation
    Impl* m_pimpl;
};

}  // namespace rp_formatter_api

#endif
