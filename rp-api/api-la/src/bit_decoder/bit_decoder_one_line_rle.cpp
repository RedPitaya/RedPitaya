#include "bit_decoder_one_line_rle.h"

#include <math.h>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <initializer_list>
#include <limits>
#include <list>
#include <string>

#include "common/profiler.h"
#include "rp_log.h"

using namespace bit_decoder;

class BitDecoderOneLine::Impl {

   public:
    auto reset() -> void;
    auto calculateBitWidth() -> void;
    auto findBit(const uint8_t* _input, size_t _size, Bit* value) -> bool;
    auto handleBit(bool bit) -> bool;

    uint64_t m_sampleRate = 0;
    uint64_t m_boundRate = 0;
    double m_bitWidth = 0;
    double m_samplePointWidth = 0;
    float m_samplePoint = 0.5;
    BitDecoderOneLineMode m_mode = LOW;
    uint8_t m_index = 0;
    bool m_invert = false;

    uint64_t m_startBitSample = 0;
    double m_startSample = 0;
    uint64_t m_sampleBit = 0;
    size_t m_i = 0;
    size_t m_j = 0;
    bool m_detectBit = false;
    bool m_initFirstBit = true;
    bool m_oldSampleBit = false;
    bool m_foundSamplePointValue = false;
    bool m_samplePointValue = false;
    bool m_detectEdge = false;
    double m_oldSampleBitEnd = 0;
    bool m_skipActiveStateAtFirst = true;
    bool m_reqBitChangeState = false;

    const uint8_t* m_data = nullptr;
    size_t m_size = 0;
};

BitDecoderOneLine::BitDecoderOneLine() {
    m_impl = new Impl();
    m_impl->reset();
}

BitDecoderOneLine::~BitDecoderOneLine() {
    delete m_impl;
}

auto BitDecoderOneLine::Impl::reset() -> void {
    m_startBitSample = 0;
    m_startSample = 0;
    m_sampleBit = 0;
    m_oldSampleBitEnd = -1;
    m_detectBit = false;
    m_initFirstBit = false;
    m_oldSampleBit = false;
    m_detectEdge = false;
    m_foundSamplePointValue = false;
    m_samplePointValue = false;
    m_skipActiveStateAtFirst = true;
    m_reqBitChangeState = false;
    m_i = 0;
    m_j = 0;
}

auto BitDecoderOneLine::Impl::calculateBitWidth() -> void {
    if (m_boundRate == 0)
        return;
    m_bitWidth = (double)m_sampleRate / (double)m_boundRate;
    m_samplePointWidth = m_bitWidth * m_samplePoint;
}

auto BitDecoderOneLine::setData(const uint8_t* _input, size_t _size) -> void {
    m_impl->m_data = _input;
    m_impl->m_size = _size;
}

auto BitDecoderOneLine::setSampleRate(uint64_t rate) -> void {
    m_impl->m_sampleRate = rate;
    m_impl->calculateBitWidth();
}

auto BitDecoderOneLine::setBoundRate(uint64_t rate) -> void {
    m_impl->m_boundRate = rate;
    m_impl->calculateBitWidth();
}

auto BitDecoderOneLine::setSamplePoint(float value) -> void {
    if (value < 0 || value > 1) {
        ERROR_LOG("The sample point value (%f) must be in the range from 0 to 1", value)
        return;
    }
    m_impl->m_samplePoint = value;
    m_impl->calculateBitWidth();
}

auto BitDecoderOneLine::setBitIndex(uint8_t bit) -> void {
    if (bit >= 8) {
        ERROR_LOG("The bit number (%d) must be in the range from 0 to 7", bit)
        return;
    }
    m_impl->m_index = bit;
}

auto BitDecoderOneLine::setStartMode(BitDecoderOneLineMode mode) -> void {
    m_impl->m_mode = mode;
}

auto BitDecoderOneLine::reset() -> void {
    m_impl->reset();
}

auto BitDecoderOneLine::setIdle() -> void {
    m_impl->m_detectBit = false;
}

auto BitDecoderOneLine::detectBitChange() -> void {
    m_impl->m_reqBitChangeState = true;
}

auto BitDecoderOneLine::syncLastBit() -> void {
    m_impl->m_detectEdge = true;
}

auto BitDecoderOneLine::setInvertMode(bool invert) -> void {
    m_impl->m_invert = invert;
}

auto BitDecoderOneLine::Impl::handleBit(bool bit) -> bool {
    if (m_sampleBit <= m_startSample + m_samplePointWidth) {
        m_foundSamplePointValue = true;
        m_samplePointValue = bit;
    }
    if (m_detectEdge && !bit && m_oldSampleBit) {
        return true;
    }
    auto f = floor(m_startSample + m_bitWidth);
    // auto c = ceil(m_startSample + m_bitWidth);
    if (m_sampleBit >= f /* && m_sampleBit < c */) {
        return true;
    }

    return false;
}

auto BitDecoderOneLine::Impl::findBit(const uint8_t* _input, size_t _size, Bit* value) -> bool {
    value->valid = false;
    auto detectBit = false;
    for (; m_i < _size; m_i += 2) {
        // Read count and data for decode RLE
        const size_t count = _input[m_i];
        const uint8_t data = _input[m_i + 1];

        for (; m_j < count + 1; m_j++) {
            bool bit = data & (1 << (m_index));
            bit = m_invert ? !bit : bit;

            if (!m_initFirstBit) {
                m_oldSampleBit = bit;
                m_initFirstBit = true;
            }

            if (m_skipActiveStateAtFirst && m_mode == bit) {
                m_oldSampleBit = bit;
                m_sampleBit++;
                continue;
            } else {
                m_skipActiveStateAtFirst = false;
            }

            if (m_detectBit) {
                detectBit = handleBit(bit);
            } else {
                auto checkError = false;
                if (m_reqBitChangeState) {
                    if (m_oldSampleBit != bit && m_mode == bit) {
                        checkError = true;
                        m_reqBitChangeState = false;
                    }
                } else {
                    checkError = true;
                }

                if (m_mode == bit && checkError) {
                    m_detectBit = true;
                    m_startSample = m_sampleBit - (m_oldSampleBit == bit && m_sampleBit > 0 ? 1 : 0);
                    m_startBitSample = m_sampleBit - (m_oldSampleBit == bit && m_sampleBit > 0 ? 1 : 0);
                    detectBit = handleBit(bit);
                }
            }
            m_oldSampleBit = bit;
            m_sampleBit++;
            if (detectBit) {
                m_j++;
                value->bitSampleStart = m_startSample;
                value->bitSampleEnd = !m_detectEdge ? m_startSample + m_bitWidth : m_sampleBit - 1;
                value->bitValue = m_samplePointValue;
                value->valid = true;
                m_startSample += !m_detectEdge ? m_bitWidth : m_sampleBit - 1 - m_startSample;  //m_bitWidth;
                m_foundSamplePointValue = false;
                m_oldSampleBitEnd = value->bitSampleEnd;
                m_detectEdge = false;
                return true;
            }
        }
        m_j = 0;
    }
    m_i = 0;

    if (m_foundSamplePointValue) {
        value->bitSampleStart = m_startSample;
        value->bitSampleEnd = m_sampleBit - 1;
        value->bitValue = m_samplePointValue;
        value->valid = true;
        m_foundSamplePointValue = false;
        return false;
    }

    return false;
}

auto BitDecoderOneLine::getNextBit(Bit* value) -> bool {
    if (value == nullptr) {
        ERROR_LOG("The structure must not have a null pointer")
        return false;
    }

    if (m_impl->m_bitWidth < 1) {
        ERROR_LOG("Bit width must be equal to or greater than 1")
        return false;
    }

    if (m_impl->m_data == nullptr) {
        ERROR_LOG("The pointer to the data buffer is null")
        return false;
    }

    if (m_impl->m_size % 2 != 0) {
        ERROR_LOG("The data size must be a multiple of 2 and not equal to zero.")
        return false;
    }

    return m_impl->findBit(m_impl->m_data, m_impl->m_size, value);
}
