/**
 * $Id$
 *
 * @brief Red Pitaya Sweep controller.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_SWEEP_H__
#define __RP_SWEEP_H__

#include "rp.h"

namespace rp_sweep_api{

class CSweepController
{

public:
    CSweepController();
    ~CSweepController();
    auto run() -> void;
    auto stop() -> void;
    auto pause(bool _state) -> void;
    auto genSweep(rp_channel_t _ch,bool _enable) -> int;
    auto isGen(rp_channel_t _ch,bool *state) -> int;
    auto isAllDisabled() -> bool;
    auto setStartFreq(rp_channel_t _ch,float _freq) -> int;
    auto getStartFreq(rp_channel_t _ch,float *_freq) -> int;
    auto setStopFreq(rp_channel_t _ch,float _freq) -> int;
    auto getStopFreq(rp_channel_t _ch,float *_freq) -> int;
    auto setTime(rp_channel_t _ch,int _us) -> int;
    auto getTime(rp_channel_t _ch,int *_us) -> int;
    auto setMode(rp_channel_t _ch,rp_gen_sweep_mode_t _mode) -> int;
    auto getMode(rp_channel_t _ch,rp_gen_sweep_mode_t *_mode) -> int;
    auto setDir(rp_channel_t _ch,rp_gen_sweep_dir_t _dir) -> int;
    auto getDir(rp_channel_t _ch,rp_gen_sweep_dir_t *_dir) -> int;
    auto resetAll() -> void;
private:

    CSweepController(const CSweepController &) = delete;
    CSweepController(CSweepController &&) = delete;
    CSweepController& operator=(const CSweepController&) = delete;
    CSweepController& operator=(const CSweepController&&) = delete;

    struct Impl;
    // Pointer to the internal implementation
    Impl *m_pimpl;
};

}

#endif