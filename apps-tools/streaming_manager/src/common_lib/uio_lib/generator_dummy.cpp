#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "generator.h"

using namespace uio_lib;

auto CGenerator::create(const UioT&, bool _channel1Enable, bool _channel2Enable, uint32_t dacHz, uint32_t maxDacHz) -> CGenerator::Ptr {
    return std::make_shared<CGenerator>(_channel1Enable, _channel2Enable, 0, nullptr, 0, dacHz, maxDacHz);
}

CGenerator::CGenerator(bool _channel1Enable, bool _channel2Enable, int _fd, void* _regset, size_t _regsetSize, uint32_t dacHz, uint32_t maxDacHz)
    : m_Channel1(_channel1Enable),
      m_Channel2(_channel2Enable),
      m_Fd(_fd),
      m_Regset(_regset),
      m_RegsetSize(_regsetSize),
      m_Map(nullptr),
      m_Buffer1(nullptr),
      m_Buffer2(nullptr),
      m_waitLock(),
      m_maxDacSpeedHz(maxDacHz),
      m_dacSpeedHz(dacHz) {}

CGenerator::~CGenerator() {}

auto CGenerator::getDacHz() -> uint32_t {
    return m_dacSpeedHz;
}

auto CGenerator::setDacHz(uint32_t hz) -> bool {
    if (((double)hz / (double)m_maxDacSpeedHz) * (1 << 16) < 1)
        return false;
    m_dacSpeedHz = hz;
    return true;
}

auto CGenerator::setReg(volatile GeneratorMapT*) -> void {}

auto CGenerator::prepare() -> void {}

auto CGenerator::setCalibration(int32_t, float, int32_t, float) -> void {}

auto CGenerator::setDataAddress(uint8_t, uint32_t, uint32_t, uint32_t) -> bool {
    return true;
}

auto CGenerator::setDataBits(bool, bool) -> void {}

auto CGenerator::setDataSize(uint32_t) -> void {}

auto CGenerator::start(bool, bool) -> void {}

auto CGenerator::stop() -> void {}

auto CGenerator::printReg() -> void {}
