#include "can_decoder.h"

#include <math.h>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <initializer_list>
#include <limits>
#include <list>
#include <string>

#include "bit_decoder/bit_decoder_one_line_rle.h"
#include "common/profiler.h"
#include "rp.h"

using namespace can;

class CANDecoder::Impl {
    enum States {
        S_START_OF_FRAME,
        S_ID_1,
        S_ID_2,
        S_RR_12_BIT,  // Remote request
        S_RR_32_BIT,  // Remote request Bit 32 for Ext frame
        S_IDE_BIT,    // Identifier extension bit
        S_R0,         // Reserved bit. Must be dominant (0), but accepted as either dominant or recessive.
        S_R1,
        S_R1_EXT,
        S_DLC,
        S_BRS,
        S_ESI,
        S_DATA,
        S_CRC,
        S_CRC_15,
        S_CRC_17,
        S_CRC_21,
        S_CRC_DEL,
        S_STUFF_BITS,
        S_ACK,
        S_ACK_DEL,
        S_END
    };

   public:
    Impl();

    CANParameters m_options;

    States m_state;
    uint32_t m_samplerate;
    float m_samplePoint;
    uint16_t m_frame_limit;
    float m_bitwidth;

    OutputPacket m_ssBit12;  // RTR or SRR bit
    bool m_isFlexibleData;   // Bit 14: FDF (Flexible data format)

    uint32_t m_id;
    uint32_t m_crc;
    double m_startBlockSamplePoint;
    uint32_t m_bitsArray;
    uint32_t m_bitsArrayCount;
    uint32_t m_bitsArrayCountSaved;
    uint32_t m_bitsArrayWithStuff;
    uint32_t m_bitsArrayCountWithStuff;
    uint32_t m_curByte;
    uint32_t m_fsbCounter;
    uint32_t m_stuffBitCounter;
    uint8_t m_dlc;
    uint32_t m_fullFDCrc;
    uint32_t m_fullFDCrcBits;
    double m_fullFDCrcStart;

    FrameFormat m_frameType;

    std::vector<OutputPacket> m_result;

    bit_decoder::BitDecoderOneLine m_bitDecoder;

    auto resetDecoder() -> void;
    auto resetState() -> void;
    auto decode(const uint8_t* _input, uint32_t _size) -> void;
    auto setNominalBitrate() -> void;
    auto setFastBitrate() -> void;
    auto getMemoryUsage() -> uint64_t;
    auto handleBit(bit_decoder::Bit& bit) -> bool;
    auto decodeStandardFrame(bit_decoder::Bit& bit) -> bool;
    auto decodeExtendedFrame(bit_decoder::Bit& bit) -> bool;
    auto decodeEndFrame(bit_decoder::Bit& bit) -> bool;
    auto dlc2Len(uint8_t _dlc) -> uint8_t;
    inline auto isStuffBit(bit_decoder::Bit& bit) -> std::tuple<bool, bool>;
};

CANDecoder::Impl::Impl() {}

CANDecoder::CANDecoder(int decoderType, const std::string& _name) {
    m_decoderType = decoderType;
    m_name = _name;
    m_impl = new Impl();
    m_impl->resetDecoder();
    setParameters(can::CANParameters());
}

CANDecoder::~CANDecoder() {
    delete m_impl;
}

auto CANDecoder::getMemoryUsage() -> uint64_t {
    return m_impl->getMemoryUsage();
}

auto CANDecoder::Impl::getMemoryUsage() -> uint64_t {
    uint64_t size = sizeof(Impl);
    size += m_result.size() * sizeof(OutputPacket);
    return size;
}

auto CANDecoder::setParameters(const CANParameters& _new_params) -> void {
    m_impl->m_options = _new_params;
}

auto CANDecoder::getSignal() -> std::vector<OutputPacket> {
    return m_impl->m_result;
}

auto CANDecoder::Impl::resetDecoder() -> void {
    m_result.clear();
    m_bitDecoder.reset();
    resetState();
}

auto CANDecoder::Impl::resetState() -> void {
    m_isFlexibleData = false;
    m_frameType = None;
    m_crc = 0;
    m_id = 0;
    m_bitsArray = 0;
    m_bitsArrayCount = 0;
    m_bitsArrayCountSaved = 0;
    m_bitsArrayWithStuff = 0;
    m_bitsArrayCountWithStuff = 0;
    m_state = S_START_OF_FRAME;
    m_startBlockSamplePoint = -1;
    m_dlc = 0;
    m_curByte = 0;
    m_stuffBitCounter = 0;
    m_fullFDCrc = 0;
    m_fullFDCrcStart = 0;
    m_fullFDCrcBits = 0;
    setNominalBitrate();
}

auto CANDecoder::reset() -> void {
    m_impl->resetDecoder();
}

auto CANDecoder::getParametersInJSON() -> std::string {
    return m_impl->m_options.toJson();
}

auto CANDecoder::setParametersInJSON(const std::string& parameter) -> void {
    CANParameters param;
    if (param.fromJson(parameter)) {
        setParameters(param);
    } else {
        ERROR_LOG("Error set parameters %s", parameter.c_str())
    }
}

auto CANDecoder::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool {
    auto opt = m_impl->m_options;
    if (opt.setDecoderSettingsUInt(key, value)) {
        setParameters(opt);
        return true;
    }
    return false;
}

auto CANDecoder::setDecoderSettingsFloat(std::string& key, float value) -> bool {
    auto opt = m_impl->m_options;
    if (opt.setDecoderSettingsFloat(key, value)) {
        setParameters(opt);
        return true;
    }
    return false;
}

auto CANDecoder::getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool {
    return m_impl->m_options.getDecoderSettingsUInt(key, value);
}

auto CANDecoder::getDecoderSettingsFloat(std::string& key, float* value) -> bool {
    return m_impl->m_options.getDecoderSettingsFloat(key, value);
}

auto CANDecoder::decode(const uint8_t* _input, uint32_t _size) -> void {
    m_impl->decode(_input, _size);
}

auto CANDecoder::Impl::decode(const uint8_t* _input, uint32_t _size) -> void {
    if (!_input)
        FATAL("Input value is null")
    if (_size == 0)
        FATAL("Input value size == 0")
    if (_size & 0x1)
        FATAL("Input value is odd")
    if (m_options.m_can_rx == 0 || m_options.m_can_rx > 8) {
        ERROR_LOG("RX not specified. Valid values from 1 to 8")
        return;
    }

    TRACE_SHORT("Input data size %d", _size)

    resetDecoder();

    // profiler::clearHistory("1");

    uint8_t line_rx = m_options.m_can_rx - 1;

    m_bitDecoder.setData(_input, _size);
    m_bitDecoder.setBitIndex(line_rx);
    m_bitDecoder.setInvertMode(m_options.m_invert_bit);
    m_bitDecoder.setSampleRate(m_options.m_acq_speed);
    m_bitDecoder.setBoundRate(m_options.m_nominal_bitrate);
    m_bitDecoder.setSamplePoint(m_options.m_sample_point / 100.0);
    m_bitDecoder.setStartMode(bit_decoder::LOW);

    bit_decoder::Bit bit;
    while (m_bitDecoder.getNextBit(&bit)) {
        handleBit(bit);
    }

    if (bit.valid) {
        handleBit(bit);
    }

    TRACE_SHORT("Frames found: %d\n", m_result.size());
}

auto CANDecoder::Impl::setNominalBitrate() -> void {
    m_bitDecoder.setBoundRate(m_options.m_nominal_bitrate);
}

auto CANDecoder::Impl::setFastBitrate() -> void {
    m_bitDecoder.setBoundRate(m_options.m_fast_bitrate);
}

auto CANDecoder::Impl::handleBit(bit_decoder::Bit& bit) -> bool {

    // Check stuff bit
    auto [isStuff, isStuffError] = isStuffBit(bit);

    m_bitsArrayWithStuff = m_bitsArrayWithStuff << 1;
    m_bitsArrayWithStuff |= bit.bitValue;
    m_bitsArrayCountWithStuff++;

    if (isStuff && m_state != S_END) {
        if (isStuffError) {
            m_result.push_back(OutputPacket{"rx", STUFF_BIT_ERROR, 0, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
            resetState();
            m_bitDecoder.setIdle();
            m_bitDecoder.detectBitChange();
            return false;
        } else {
            m_result.push_back(OutputPacket{"rx", STUFF_BIT, 0, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        }
        m_stuffBitCounter++;
        return false;
    }

    m_bitsArray = m_bitsArray << 1;
    m_bitsArray |= bit.bitValue;
    m_bitsArrayCount++;

    // Bit 0: Start of frame (SOF) bit
    if (m_state == S_START_OF_FRAME && bit.bitValue == 0) {
        m_result.push_back(OutputPacket{"rx", START_OF_FRAME, 0, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        m_state = S_ID_1;
        return true;
    }

    // Bits 1-11: Identifier (ID[10..0])
    // The bits ID[10..4] must NOT be all recessive.
    if (m_state == S_ID_1) {
        if (m_startBlockSamplePoint == -1) {
            m_startBlockSamplePoint = bit.bitSampleStart;
            m_bitsArrayCountSaved = m_bitsArrayCount - 1;
            m_stuffBitCounter = 0;
        }

        if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 11) {
            auto id_1 = m_bitsArray & 0x7FF;
            m_id = id_1;
            m_result.push_back(OutputPacket{"rx", ID, id_1, (float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter, m_startBlockSamplePoint,
                                            bit.bitSampleEnd - m_startBlockSamplePoint});
            if ((id_1 & 0x7F0) == 0x7F0) {
                m_result.push_back(OutputPacket{"rx", WARNING_1, id_1, (float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
                                                m_startBlockSamplePoint, bit.bitSampleEnd - m_startBlockSamplePoint});
            }
            m_state = S_RR_12_BIT;
            m_startBlockSamplePoint = -1;
        }
        return true;
    }

    if (m_state == S_RR_12_BIT) {
        m_ssBit12 = OutputPacket{"rx", RTR, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart};
        m_state = S_IDE_BIT;
        return true;
    }

    // Bit 13: Identifier extension (IDE) bit
    // Standard frame: dominant, extended frame: recessive
    if (m_state == S_IDE_BIT) {
        m_frameType = bit.bitValue ? Extended : Standart;
        m_result.push_back(OutputPacket{"rx", IDE, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        m_state = bit.bitValue ? S_ID_2 : S_R0;
        return true;
    }

    if (m_frameType == Standart) {
        return decodeStandardFrame(bit);
    }

    if (m_frameType == Extended) {
        return decodeExtendedFrame(bit);
    }

    return false;
}

auto CANDecoder::Impl::decodeStandardFrame(bit_decoder::Bit& bit) -> bool {
    // Bit 14: Reserve bit r0
    // FDF (Flexible data format)
    // Has to be sent dominant when FD frame, has to be sent recessive when classic CAN frame.

    if (m_state == S_R0) {
        m_isFlexibleData = bit.bitValue;
        if (bit.bitValue) {
            m_result.push_back(OutputPacket{"rx", RESERV_BIT_FLEX, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
            auto b12 = m_ssBit12;
            b12.control = SRR;
            m_result.push_back(b12);
            m_state = S_R1;
        } else {
            m_result.push_back(m_ssBit12);
            m_state = S_DLC;
        }
        m_result.push_back(OutputPacket{"rx", RESERV_BIT, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        return true;
    }

    if (m_state == S_R1) {
        m_result.push_back(OutputPacket{"rx", RESERV_BIT, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        m_state = S_BRS;
        m_bitDecoder.syncLastBit();
        return true;
    }

    if (m_state == S_BRS) {
        m_result.push_back(OutputPacket{"rx", BRS, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        if (bit.bitValue)
            setFastBitrate();
        m_state = S_ESI;
        return true;
    }

    if (m_state == S_ESI) {
        m_result.push_back(OutputPacket{"rx", ESI, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        m_state = S_DLC;
        return true;
    }

    if (m_state == S_DLC) {
        if (m_startBlockSamplePoint == -1) {
            m_startBlockSamplePoint = bit.bitSampleStart;
            m_bitsArrayCountSaved = m_bitsArrayCount - 1;
            m_stuffBitCounter = 0;
        }

        if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 4) {
            m_dlc = m_bitsArray & 0xF;
            // The values 0-8 indicate 0-8 bytes like classic CAN. The values 9-15 are translated to a value
            // between 12-64 which is the actual length of the data field:
            // 9→12   10→16   11→20   12→24   13→32   14→48   15→64
            if (m_isFlexibleData) {
                m_dlc = dlc2Len(m_dlc);
            }

            m_result.push_back(OutputPacket{"rx", DLC, m_dlc, (float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter, m_startBlockSamplePoint,
                                            bit.bitSampleEnd - m_startBlockSamplePoint});
            m_state = m_dlc != 0 ? S_DATA : S_CRC;
            m_curByte = 0;
            m_startBlockSamplePoint = -1;
        }
        return true;
    }

    if (m_state == S_DATA) {
        if (m_startBlockSamplePoint == -1) {
            m_startBlockSamplePoint = bit.bitSampleStart;
            m_bitsArrayCountSaved = m_bitsArrayCount - 1;
            m_stuffBitCounter = 0;
        }

        if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 8) {
            auto data = m_bitsArray & 0xFF;

            m_result.push_back(OutputPacket{"rx", PAYLOAD_DATA, data, (float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
                                            m_startBlockSamplePoint, bit.bitSampleEnd - m_startBlockSamplePoint});
            m_curByte++;
            if (m_curByte == m_dlc) {
                m_state = S_CRC;
                m_fullFDCrc = 0;
                m_fullFDCrcBits = 0;
            }
            m_startBlockSamplePoint = -1;
        }
        return true;
    }

    return decodeEndFrame(bit);
}

auto CANDecoder::Impl::decodeExtendedFrame(bit_decoder::Bit& bit) -> bool {

    // Bits 14-31: Extended identifier (EID[17..0])
    if (m_state == S_ID_2) {
        if (m_startBlockSamplePoint == -1) {
            m_startBlockSamplePoint = bit.bitSampleStart;
            m_bitsArrayCountSaved = m_bitsArrayCount - 1;
            m_stuffBitCounter = 0;
        }

        if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 18) {
            auto id_2 = m_bitsArray & 0x3FFFF;
            m_result.push_back(OutputPacket{"rx", EXT_ID, id_2, (float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter, m_startBlockSamplePoint,
                                            bit.bitSampleEnd - m_startBlockSamplePoint});
            auto f_id = m_id << 18 | id_2;
            m_result.push_back(OutputPacket{"rx", FULL_ID, f_id, (float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter, m_startBlockSamplePoint,
                                            bit.bitSampleEnd - m_startBlockSamplePoint});
            m_state = S_RR_32_BIT;
            m_startBlockSamplePoint = -1;
        }
        return true;
    }

    if (m_state == S_RR_32_BIT) {
        m_result.push_back(OutputPacket{"rx", RTR, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        m_state = S_R0;
        return true;
    }

    // Bit 33: RB1 (reserved bit)
    if (m_state == S_R0) {
        m_isFlexibleData = bit.bitValue;
        if (bit.bitValue) {
            m_result.push_back(OutputPacket{"rx", RESERV_BIT_FLEX, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
            m_state = S_R1;
        } else {
            m_state = S_R1_EXT;
        }
        auto b12 = m_ssBit12;
        b12.control = SRR;
        m_result.push_back(b12);
        m_result.push_back(OutputPacket{"rx", RESERV_BIT, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        return true;
    }

    if (m_state == S_R1_EXT) {
        m_result.push_back(OutputPacket{"rx", RESERV_BIT, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        m_state = S_DLC;
        return true;
    }

    if (m_state == S_R1) {
        m_result.push_back(OutputPacket{"rx", RESERV_BIT, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        m_state = S_BRS;
        m_bitDecoder.syncLastBit();
        return true;
    }

    if (m_state == S_BRS) {
        m_result.push_back(OutputPacket{"rx", BRS, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        if (bit.bitValue)
            setFastBitrate();
        m_state = S_ESI;
        return true;
    }

    if (m_state == S_ESI) {
        m_result.push_back(OutputPacket{"rx", ESI, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        m_state = S_DLC;
        return true;
    }

    if (m_state == S_DLC) {
        if (m_startBlockSamplePoint == -1) {
            m_startBlockSamplePoint = bit.bitSampleStart;
            m_bitsArrayCountSaved = m_bitsArrayCount - 1;
            m_stuffBitCounter = 0;
        }

        if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 4) {
            m_dlc = m_bitsArray & 0xF;
            // The values 0-8 indicate 0-8 bytes like classic CAN. The values 9-15 are translated to a value
            // between 12-64 which is the actual length of the data field:
            // 9→12   10→16   11→20   12→24   13→32   14→48   15→64
            if (m_isFlexibleData) {
                m_dlc = dlc2Len(m_dlc);
            }

            m_result.push_back(OutputPacket{"rx", DLC, m_dlc, (float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter, m_startBlockSamplePoint,
                                            bit.bitSampleEnd - m_startBlockSamplePoint});
            m_state = m_dlc != 0 ? S_DATA : S_CRC;
            m_curByte = 0;
            m_startBlockSamplePoint = -1;
        }
        return true;
    }

    if (m_state == S_DATA) {
        if (m_startBlockSamplePoint == -1) {
            m_startBlockSamplePoint = bit.bitSampleStart;
            m_bitsArrayCountSaved = m_bitsArrayCount - 1;
            m_stuffBitCounter = 0;
        }

        if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 8) {
            auto data = m_bitsArray & 0xFF;

            m_result.push_back(OutputPacket{"rx", PAYLOAD_DATA, data, (float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
                                            m_startBlockSamplePoint, bit.bitSampleEnd - m_startBlockSamplePoint});
            m_curByte++;
            if (m_curByte == m_dlc) {
                m_state = S_CRC;
                m_fullFDCrc = 0;
                m_fullFDCrcBits = 0;
            }
            m_startBlockSamplePoint = -1;
        }
        return true;
    }

    return decodeEndFrame(bit);
}

auto CANDecoder::Impl::decodeEndFrame(bit_decoder::Bit& bit) -> bool {
    // Remember start of CRC sequence (see below).

    if (m_state == S_CRC) {
        if (m_isFlexibleData) {
            m_result.push_back(OutputPacket{"rx", FSB, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
            m_state = S_STUFF_BITS;
            m_fullFDCrc = m_fullFDCrc << 1 | bit.bitValue;
            m_fullFDCrcStart = bit.bitSampleStart;
            m_fullFDCrcBits++;
            return true;
        } else {
            m_state = S_CRC_15;
        }
    }

    if (m_state == S_CRC_15) {
        if (m_startBlockSamplePoint == -1) {
            m_startBlockSamplePoint = bit.bitSampleStart;
            m_bitsArrayCountSaved = m_bitsArrayCount - 1;
            m_stuffBitCounter = 0;
        }

        if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 15) {
            m_crc = m_bitsArray & 0x7FFF;
            m_result.push_back(OutputPacket{"rx", CRC_15_VAL, m_crc, (float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter,
                                            m_startBlockSamplePoint, bit.bitSampleEnd - m_startBlockSamplePoint});
            m_state = S_CRC_DEL;
            m_startBlockSamplePoint = -1;
        }
        return true;
    }

    if (m_state == S_STUFF_BITS) {
        if (m_startBlockSamplePoint == -1) {
            m_startBlockSamplePoint = bit.bitSampleStart;
            m_bitsArrayCountSaved = m_bitsArrayCount - 1;
            m_stuffBitCounter = 0;
        }
        m_fullFDCrc = m_fullFDCrc << 1 | bit.bitValue;
        m_fullFDCrcBits++;
        if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 4) {
            auto sb = m_bitsArray & 0xF;
            m_result.push_back(OutputPacket{"rx", SBC, sb, (float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter, m_startBlockSamplePoint,
                                            bit.bitSampleEnd - m_startBlockSamplePoint});
            m_state = m_dlc < 16 ? S_CRC_17 : S_CRC_21;
            m_startBlockSamplePoint = -1;
        }
        return true;
    }

    if (m_state == S_CRC_17 || m_state == S_CRC_21) {
        if (m_startBlockSamplePoint == -1) {
            m_startBlockSamplePoint = bit.bitSampleStart;
            m_bitsArrayCountSaved = m_bitsArrayCount - 1;
            m_crc = 0;
            m_fsbCounter = 0;
            m_stuffBitCounter = 0;
        }
        m_fullFDCrc = m_fullFDCrc << 1 | bit.bitValue;
        m_fullFDCrcBits++;
        if ((m_bitsArrayCount - m_bitsArrayCountSaved) == 1 + (m_fsbCounter * 5)) {
            m_result.push_back(OutputPacket{"rx", FSB, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
            m_fsbCounter++;
        } else {
            m_crc = m_crc << 1;
            m_crc |= bit.bitValue;
        }
        uint32_t l = m_state == S_CRC_17 ? 22 : 27;
        if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= l) {
            m_result.push_back(OutputPacket{"rx", (m_state == S_CRC_17 ? CRC_17_VAL : CRC_21_VAL), m_crc,
                                            (float)(m_bitsArrayCount - m_bitsArrayCountSaved) + m_stuffBitCounter, m_startBlockSamplePoint,
                                            bit.bitSampleEnd - m_startBlockSamplePoint});
            m_result.push_back(
                OutputPacket{"rx", CRC_FSB_SBC, m_fullFDCrc, (float)(m_fullFDCrcBits), m_fullFDCrcStart, bit.bitSampleEnd - m_startBlockSamplePoint});
            m_state = S_CRC_DEL;
            m_startBlockSamplePoint = -1;
        }
        return true;
    }

    // CRC delimiter bit (recessive)
    if (m_state == S_CRC_DEL) {
        m_result.push_back(OutputPacket{"rx", CRC_DELIMITER, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        if (!bit.bitValue) {
            m_result.push_back(OutputPacket{"rx", WARNING_2, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        }
        if (m_isFlexibleData) {
            setNominalBitrate();
        }
        m_state = S_ACK;
        return true;
    }

    // ACK slot bit (dominant: ACK, recessive: NACK)
    if (m_state == S_ACK) {
        m_result.push_back(OutputPacket{"rx", ACK_SLOT, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        m_state = S_ACK_DEL;
        return true;
    }

    // ACK delimiter bit (recessive)
    if (m_state == S_ACK_DEL) {
        m_result.push_back(OutputPacket{"rx", ACK_DELIMITER, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        if (!bit.bitValue) {
            m_result.push_back(OutputPacket{"rx", WARNING_3, bit.bitValue, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        }
        m_state = S_END;
        return true;
    }

    if (m_state == S_END) {
        if (m_startBlockSamplePoint == -1) {
            m_startBlockSamplePoint = bit.bitSampleStart;
            m_bitsArrayCountSaved = m_bitsArrayCount - 1;
        }
        if ((m_bitsArrayCount - m_bitsArrayCountSaved) >= 7) {
            auto end = m_bitsArray & 0x7F;
            m_result.push_back(OutputPacket{"rx", END_OF_FRAME, end, (float)(m_bitsArrayCount - m_bitsArrayCountSaved), m_startBlockSamplePoint,
                                            bit.bitSampleEnd - m_startBlockSamplePoint});
            if ((end & 0x7F) != 0x7F) {
                m_result.push_back(OutputPacket{"rx", ERROR_3, end, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
            }
            resetState();
            m_bitDecoder.setIdle();
            m_bitDecoder.detectBitChange();
            // m_bitDetect = false;
            m_startBlockSamplePoint = -1;
        }
        return true;
    }
    return false;
}

auto CANDecoder::Impl::isStuffBit(bit_decoder::Bit& bit) -> std::tuple<bool, bool> {
    if (m_bitsArrayCountWithStuff >= 5) {
        if ((m_bitsArrayWithStuff & 0x1F) == 0) {
            if (bit.bitValue)
                return std::make_tuple(true, false);
            else
                return std::make_tuple(true, true);
        }

        if ((m_bitsArrayWithStuff & 0x1F) == 0x1F) {
            if (!bit.bitValue)
                return std::make_tuple(true, false);
            else
                return std::make_tuple(true, true);
        }
    }
    return std::make_tuple(false, false);
}

auto CANDecoder::Impl::dlc2Len(uint8_t _dlc) -> uint8_t {
    uint8_t code_page[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
    return code_page[_dlc];
}