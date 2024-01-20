/**
* $Id: $
*
* @brief Red Pitaya application Impedance analyzer module interface
*
* @Author Luka Golinar <luka.golinar@gmail.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <atomic>
#include <vector>

#include "lcr_generator.h"
#include "utils.h"

CLCRGenerator::CLCRGenerator(){
    setDefault();
}

CLCRGenerator::~CLCRGenerator(){

}

auto CLCRGenerator::start() -> int{
    rp_channel_t channel = RP_CH_1;
    std::lock_guard<std::mutex> lock(m_mutex);
    ECHECK_APP(rp_GenReset());
    ECHECK_APP(rp_GenAmp(channel, m_amplitude));
    ECHECK_APP(rp_GenOffset(channel, m_offset));
    ECHECK_APP(rp_GenWaveform(channel, RP_WAVEFORM_SINE));
    ECHECK_APP(rp_GenFreq(channel, m_freq));
    ECHECK_APP(rp_GenOutEnable(channel));
    ECHECK_APP(rp_GenResetTrigger(channel));
    return RP_LCR_OK;
}

auto CLCRGenerator::stop() -> int{
    std::lock_guard<std::mutex> lock(m_mutex);
    ECHECK_APP(rp_GenOutDisable(RP_CH_1))
    ECHECK_APP(rp_GenOutDisable(RP_CH_2))
    return RP_LCR_OK;
}

auto CLCRGenerator::setDefault() -> void{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_freq = 10;
}

auto CLCRGenerator::setFreq(uint32_t _freq) -> void{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_freq = _freq;
}

auto CLCRGenerator::getFreq() -> uint32_t{
    return m_freq;
}