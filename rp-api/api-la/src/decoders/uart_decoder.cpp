#include "uart_decoder.h"

#include <math.h>
#include <cassert>
#include <limits>
#include "bit_decoder/bit_decoder_one_line_rle.h"
#include "rp.h"

using namespace uart;

class UARTDecoder::Impl {

    enum State { WAIT_FOR_START_BIT, GET_START_BIT, GET_DATA_BITS, GET_PARITY_BIT, GET_STOP_BITS };

   public:
    Impl();

    bool m_isEnd;
    bool m_oldBit;
    // uint32_t    m_prevBitStart;
    // uint32_t    m_startDataBits;
    bool m_initStartFirst;
    bool m_initStopFirst;
    bit_decoder::Bit m_startDataBit;
    bit_decoder::Bit m_startStopDataBit;
    uint8_t m_curDataBit;
    uint8_t m_curStopBit;
    uint16_t m_dataByte;
    uint8_t m_parityBit;
    bool m_parityOk;
    uint32_t m_samplenum;
    uint32_t m_silenceLength;
    uint16_t m_bitAccumulate;

    double m_bitWidth;

    State m_state;

    UARTParameters m_options;
    bit_decoder::BitDecoderOneLine m_bitdecoder;

    std::string m_line;

    std::string m_name;
    std::vector<OutputPacket> m_result;

    void resetDecoder();
    void decode(const uint8_t* _input, uint32_t _size);

    // void waitForStartBit(bool bit, uint32_t sampleNum);
    // void getStartBit(bool bit, uint32_t sampleNum);
    void getStartBit(bit_decoder::Bit& bit);
    // void getDataBits(bool bit, uint32_t sampleNum);
    void getDataBits(bit_decoder::Bit& bit);
    // void getParityBit(bool bit, uint32_t sampleNum);
    void getParityBit(bit_decoder::Bit& bit);
    // void getStopBits(bool bit, uint32_t sampleNum);
    void getStopBits(bit_decoder::Bit& bit);
    bool parityOk();
};

UARTDecoder::Impl::Impl() {}

UARTDecoder::UARTDecoder(int decoderType, const std::string& _name) {
    m_decoderType = decoderType;
    m_name = _name;
    m_impl_rx = new Impl();
    m_impl_tx = new Impl();
    m_impl_rx->m_line = "rx";
    m_impl_tx->m_line = "tx";
    m_impl_rx->resetDecoder();
    m_impl_tx->resetDecoder();
    setParameters(uart::UARTParameters());
}

UARTDecoder::~UARTDecoder() {
    delete m_impl_rx;
    delete m_impl_tx;
}

auto UARTDecoder::reset() -> void {
    m_impl_tx->resetDecoder();
    m_impl_rx->resetDecoder();
}

auto UARTDecoder::getMemoryUsage() -> uint64_t {
    uint64_t size = sizeof(Impl);
    size += m_impl_rx->m_result.size() * sizeof(OutputPacket);
    size += m_impl_tx->m_result.size() * sizeof(OutputPacket);
    return size;
}

void UARTDecoder::setParameters(const UARTParameters& _new_params) {
    if (_new_params.m_baudrate == 0)
        FATAL("baudrate should not be equal to 0")
    m_impl_rx->m_options = _new_params;
    m_impl_tx->m_options = _new_params;
    m_impl_rx->m_bitWidth = (double)_new_params.m_samplerate / (double)_new_params.m_baudrate;
    m_impl_tx->m_bitWidth = (double)_new_params.m_samplerate / (double)_new_params.m_baudrate;
}

auto UARTDecoder::setDecoderSettingsUInt(std::string& key, uint32_t value) -> bool {
    auto opt_tx = m_impl_tx->m_options;
    auto opt_rx = m_impl_rx->m_options;
    if (opt_tx.setDecoderSettingsUInt(key, value) && opt_rx.setDecoderSettingsUInt(key, value)) {
        setParameters(opt_tx);
        return true;
    }
    return false;
}

auto UARTDecoder::getDecoderSettingsUInt(std::string& key, uint32_t* value) -> bool {
    return m_impl_tx->m_options.getDecoderSettingsUInt(key, value);
}

auto UARTDecoder::getParametersInJSON() -> std::string {
    return m_impl_rx->m_options.toJson();
}

auto UARTDecoder::setParametersInJSON(const std::string& parameter) -> void {
    UARTParameters param;
    if (param.fromJson(parameter)) {
        setParameters(param);
    } else {
        ERROR_LOG("Error set parameters %s", parameter.c_str())
    }
}

std::vector<OutputPacket> UARTDecoder::getSignal() {
    auto vec = m_impl_rx->m_result;
    auto vec2 = m_impl_tx->m_result;
    vec.insert(vec.end(), vec2.begin(), vec2.end());
    return vec;
}

void UARTDecoder::Impl::resetDecoder() {
    m_samplenum = 0;
    m_state = GET_START_BIT;
    // m_startDataBits = 0;
    // m_prevBitStart = 0;
    m_curDataBit = 0;
    m_curStopBit = 0;
    m_dataByte = 0;
    m_parityBit = 0;
    m_parityOk = false;
    m_oldBit = 1;
    m_silenceLength = 0;
    m_bitAccumulate = 0;
    m_isEnd = 0;
    m_bitdecoder.reset();
    m_result.clear();
}

void UARTDecoder::decode(const uint8_t* _input, uint32_t _size) {
    if (m_impl_rx->m_options.m_rx != 0)
        m_impl_rx->decode(_input, _size);
    if (m_impl_tx->m_options.m_tx != 0)
        m_impl_tx->decode(_input, _size);
}

void UARTDecoder::Impl::decode(const uint8_t* _input, uint32_t _size) {
    auto parseBit = [&](bit_decoder::Bit& bit) {
        // TODO Add IDLE and BREAK frame support
        // https://deepbluembedded.com/stm32-usart-uart-tutorial/#:~:text=STM32%20UART%20Data%20Packet
        // https://sigrok.org/gitweb/?p=libsigrokdecode.git;a=blob_plain;f=decoders/uart/pd.py;hb=HEAD#:~:text=def%20handle_break(self%2C%20rxtx%2C%20ss%2C%20es)%3A

        if (m_state == GET_START_BIT)
            getStartBit(bit);
        else if (m_state == GET_DATA_BITS)
            getDataBits(bit);
        else if (m_state == GET_PARITY_BIT)
            getParityBit(bit);
        else if (m_state == GET_STOP_BITS)
            getStopBits(bit);

        if (m_state == WAIT_FOR_START_BIT) {
            m_bitdecoder.setIdle();
            m_state = GET_START_BIT;
        }
    };

    if (!_input)
        FATAL("Input value is null")
    if (_size == 0)
        FATAL("Input value size == 0")
    if (_size & 0x1)
        FATAL("Input value is odd")
    if (m_options.m_rx > 8) {
        ERROR_LOG("RX not specified. Valid values from 0 to 8")
        return;
    }

    if (m_options.m_tx > 8) {
        ERROR_LOG("TX not specified. Valid values from 0 to 8")
        return;
    }

    if (m_options.m_rx == 0 && m_options.m_tx == 0) {
        ERROR_LOG("RX or TX not specified. Valid values from 0 to 8")
        return;
    }

    resetDecoder();

    uint8_t rx_line = 0;

    if (m_line == "rx")
        rx_line = m_options.m_rx;

    if (m_line == "tx")
        rx_line = m_options.m_tx;

    rx_line--;

    m_bitdecoder.setData(_input, _size);
    m_bitdecoder.setBitIndex(rx_line);
    m_bitdecoder.setSamplePoint(0.5);
    m_bitdecoder.setInvertMode(m_options.m_invert);
    m_bitdecoder.setStartMode(bit_decoder::LOW);
    m_bitdecoder.setSampleRate(m_options.m_samplerate);
    m_bitdecoder.setBoundRate(m_options.m_baudrate);
    bit_decoder::Bit bit;
    while (m_bitdecoder.getNextBit(&bit)) {
        // bit_decoder::Bit::print(bit);
        parseBit(bit);
    }
    if (bit.valid) {
        // bit_decoder::Bit::print(bit);
        parseBit(bit);
    }
}

void UARTDecoder::Impl::getStartBit(bit_decoder::Bit& bit) {
    // The startbit must be 0. If not, we report an error.
    if (bit.bitValue != 0) {
        // START-bit error
        m_result.push_back(OutputPacket{m_line, START_BIT_ERR, 0, 0, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
        m_bitdecoder.setIdle();
        m_state = WAIT_FOR_START_BIT;
        return;
    }

    m_result.push_back(OutputPacket{m_line, START_BIT, 0, 0, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
    m_curDataBit = 0;
    m_dataByte = 0;
    m_bitAccumulate = 0;
    m_initStartFirst = false;
    m_state = GET_DATA_BITS;
}

void UARTDecoder::Impl::getDataBits(bit_decoder::Bit& bit) {

    if (!m_initStartFirst) {
        m_startDataBit = bit;
        m_initStartFirst = true;
    }

    if (m_options.m_bitOrder == MSB_FIRST) {
        m_dataByte <<= 1;
        m_dataByte |= (bit.bitValue << 0);
    } else if (m_options.m_bitOrder == LSB_FIRST) {
        m_dataByte >>= 1;
        m_dataByte |= (bit.bitValue << (m_options.m_num_data_bits - 1));
    }

    // Return here, unless we already received all data bits.
    if (m_curDataBit < (m_options.m_num_data_bits - 1)) {
        m_bitAccumulate = 0;
        m_curDataBit++;
        return;
    }

    m_result.push_back({m_line, DATA, m_dataByte, (float)m_curDataBit + 1, m_startDataBit.bitSampleStart, bit.bitSampleEnd - m_startDataBit.bitSampleStart});

    if (m_options.m_parity == NONE) {
        m_state = GET_STOP_BITS;
        m_curStopBit = 0;
        if (m_options.m_num_stop_bits == STOP_BITS_05 || m_options.m_num_stop_bits == STOP_BITS_15) {
            m_bitdecoder.setBoundRate(m_options.m_baudrate * 2);
        } else {
            m_bitdecoder.setBoundRate(m_options.m_baudrate);
        }
        m_initStopFirst = false;
        m_bitdecoder.syncLastBit();
    } else
        m_state = GET_PARITY_BIT;
}

void UARTDecoder::Impl::getParityBit(bit_decoder::Bit& bit) {
    m_parityBit = bit.bitValue;
    m_parityOk = parityOk();
    if (m_parityOk)
        m_result.push_back(OutputPacket{m_line, PARITY_BIT, 0, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
    else {
        m_result.push_back(OutputPacket{m_line, PARITY_ERR, 0, 1, bit.bitSampleStart, bit.bitSampleEnd - bit.bitSampleStart});
    }
    m_bitAccumulate = 0;
    m_state = GET_STOP_BITS;
    m_curStopBit = 0;
    if (m_options.m_num_stop_bits == STOP_BITS_05 || m_options.m_num_stop_bits == STOP_BITS_15) {
        m_bitdecoder.setBoundRate(m_options.m_baudrate * 2);
    } else {
        m_bitdecoder.setBoundRate(m_options.m_baudrate);
    }
    m_initStopFirst = false;
    m_bitdecoder.syncLastBit();
}

void UARTDecoder::Impl::getStopBits(bit_decoder::Bit& bit) {

    if (!m_initStopFirst) {
        m_startStopDataBit = bit;
        m_initStopFirst = true;
    }

    float bitWait = 1;
    uint8_t bits = 0;

    switch (m_options.m_num_stop_bits) {
        case STOP_BITS_05:
            bitWait = 0.5;
            bits = 1;
            break;

        case STOP_BITS_10:
            bitWait = 1;
            bits = 1;
            break;

        case STOP_BITS_15:
            bitWait = 1.5;
            bits = 3;  // 3 * 0.5
            break;

        case STOP_BITS_20:
            bitWait = 2;
            bits = 2;  // 2 * 1
            break;

        case STOP_BITS_NO:
            m_state = GET_START_BIT;
            m_bitdecoder.setBoundRate(m_options.m_baudrate);
            m_bitdecoder.setIdle();
            m_bitdecoder.syncLastBit();
            return;

        default:
            FATAL("Unknown mode %d", m_options.m_num_stop_bits)
    }

    if (bit.bitValue == false) {
        // STOP-bit error
        m_result.push_back({m_line, STOP_BIT_ERR, 0, 1, m_startStopDataBit.bitSampleStart, bit.bitSampleEnd - m_startStopDataBit.bitSampleStart});
        m_state = GET_START_BIT;
        m_bitdecoder.setBoundRate(m_options.m_baudrate);
        m_bitdecoder.setIdle();
        m_bitdecoder.detectBitChange();
        return;
    }

    if (m_curStopBit < bits - 1) {
        m_curStopBit++;
        return;
    }

    m_result.push_back({m_line, STOP_BIT, 0, bitWait, m_startStopDataBit.bitSampleStart, bit.bitSampleEnd - m_startStopDataBit.bitSampleStart});
    m_state = GET_START_BIT;
    m_bitdecoder.setBoundRate(m_options.m_baudrate);
    m_bitdecoder.setIdle();
}

bool UARTDecoder::Impl::parityOk() {
    int sumOfBits = 0;

    for (int i = 0; i < m_options.m_num_data_bits; i++)
        sumOfBits += (int)((m_dataByte >> i) & 0x01);

    sumOfBits += m_parityBit;

    switch (m_options.m_parity) {
        case ODD:
            return (sumOfBits % 2) == 1;
        case EVEN:
            return (sumOfBits % 2) == 0;
        case ALWAYS_0:
            return m_parityBit == 0;
        case ALWAYS_1:
            return m_parityBit == 1;
        default:
            ERROR_LOG("Unknown mode parity %d", m_options.m_parity)
            break;
    }
    return false;
}
