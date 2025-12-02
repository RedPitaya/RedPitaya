/**
* $Id: $
*
* @brief Red Pitaya application library Impedance Analyzer module interface
*
* @Author Luka Golinar <luka.golinar@gmail.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*/

#ifndef __LCRGENERATOR_H
#define __LCRGENERATOR_H
#include <stdbool.h>
#include <stdint.h>
#include <mutex>
#include <string>

#include "lcrApp.h"
#include "rp.h"

class CLCRGenerator {

   public:
    CLCRGenerator();
    ~CLCRGenerator();

    CLCRGenerator(CLCRGenerator&) = delete;
    CLCRGenerator(CLCRGenerator&&) = delete;

    auto start() -> int;
    auto setSettings() -> int;
    auto stop() -> int;
    auto setDefault() -> void;
    auto setFreq(uint32_t _freq) -> void;
    auto getFreq() -> uint32_t;
    auto setAmplitude(float _ampl) -> void;
    auto getAmplitude() -> float;
    auto setOffset(float _offset) -> void;
    auto getOffset() -> float;

   private:
    std::mutex m_mutex;
    uint32_t m_freq = 10;
    float m_amplitude = 0.5;
    float m_offset = 0;
};

#endif  //__LCRGENERATOR_H
