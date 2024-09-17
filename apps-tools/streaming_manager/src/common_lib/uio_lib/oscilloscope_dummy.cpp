#include "oscilloscope.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

using namespace uio_lib;

auto COscilloscope::create(const UioT &,
						   uint32_t _dec_factor,
						   bool _isMaster,
						   uint32_t _adcMaxSpeed,
						   bool _isADCFilterPresent,
						   uint8_t _fpgaBits,
						   uint8_t _maxChannels) -> COscilloscope::Ptr
{
	return std::make_shared<COscilloscope>(0, nullptr, 0, _dec_factor, _isMaster, _adcMaxSpeed, _isADCFilterPresent, _fpgaBits, _maxChannels);
}

COscilloscope::COscilloscope(int _fd,
							 void *_regset,
							 size_t _regsetSize,
							 uint32_t _dec_factor,
							 bool _isMaster,
							 uint32_t _adcMaxSpeed,
							 bool _isADCFilterPresent,
							 uint8_t _fpgaBits,
							 uint8_t _maxChannels)
	: m_Fd(_fd)
	, m_Regset(_regset)
	, m_RegsetSize(_regsetSize)
	, m_OscMap(nullptr)
	, m_dec_factor(_dec_factor)
	, m_filterBypass(true)
	, m_isMaster(_isMaster)
	, m_adcMaxSpeed(_adcMaxSpeed)
	, m_isADCFilterPresent(_isADCFilterPresent)
	, m_fpgaBits(_fpgaBits)
	, m_maxChannels(_maxChannels)
{}

COscilloscope::~COscilloscope() {}

auto COscilloscope::setSkipDataAddress(uio_lib::MemoryRegionT) -> void {}

auto COscilloscope::setDataAddress(uint8_t, uint32_t, uint32_t, uint32_t, uint32_t) -> void {}

auto COscilloscope::setDataSize(uint32_t) -> void {}

void COscilloscope::setReg(volatile OscilloscopeMapT *) {}

void COscilloscope::setFilterCalibration(uint8_t, int32_t, int32_t, int32_t, int32_t) {}

void COscilloscope::setFilterBypass(bool) {}

auto COscilloscope::prepare() -> void {}

auto COscilloscope::setCalibration(uint8_t, int32_t, float) -> void {}

auto COscilloscope::getFPGALost(uint8_t, uint32_t &_overFlow) -> bool
{
	_overFlow = 0;
	return true;
}

auto COscilloscope::getDecimation() -> uint32_t
{
	return m_dec_factor;
}

auto COscilloscope::getOSCRate() -> uint32_t
{
	return m_dec_factor;
}

auto COscilloscope::clearBuffer(uint8_t) -> bool
{
	return true;
}

auto COscilloscope::wait() -> bool
{
	return true;
}

auto COscilloscope::clearInterrupt() -> bool
{
	return true;
}

auto isMaster() -> BoardMode
{
	return BoardMode::UNKNOWN;
}

auto COscilloscope::stop() -> void {}

auto COscilloscope::printReg() -> void {}

auto COscilloscope::set8BitMode(bool) -> void {}
