#pragma once

#include "bit_decoder.h"

namespace bit_decoder {

enum BitDecoderOneLineMode { LOW = 0, HIGH = 1 };

class BitDecoderOneLine {
    class Impl;

   public:
    BitDecoderOneLine();
    ~BitDecoderOneLine();

    auto setData(const uint8_t* _input, size_t _size) -> void;
    auto reset() -> void;
    auto setIdle() -> void;
    auto syncLastBit() -> void;
    auto detectBitChange() -> void;
    auto getNextBit(Bit* value) -> bool;
    auto setSampleRate(uint64_t rate) -> void;
    auto setBoundRate(uint64_t rate) -> void;
    auto setSamplePoint(float value) -> void;
    auto setBitIndex(uint8_t bit) -> void;
    auto setStartMode(BitDecoderOneLineMode mode) -> void;
    auto setInvertMode(bool invert) -> void;

   private:
    Impl* m_impl;
};

}  // namespace bit_decoder