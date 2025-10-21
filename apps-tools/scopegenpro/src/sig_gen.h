#pragma once

#include <CustomParameters.h>
#include <DataManager.h>
#include "rp.h"

#define CH_SIGNAL_SIZE_DEFAULT 1024

auto synthesis_arb(CFloatBase64Signal* signal, const float* data, uint32_t _size, float freq, float amp, float off, float showOff, float tscale) -> void;
auto synthesis_arb_burst(CFloatBase64Signal* signal, const float* data, uint32_t _size, float freq, float amp, float off, float showOff, int burstCount, int periodBrust, int reps,
                         float tscale, float timeOffset, float initV, float lastV) -> void;
auto synthesis_sin(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) -> void;
auto synthesis_sin_burst(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                         float timeOffset, float initV, float lastV) -> void;
auto synthesis_triangle(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) -> void;
auto synthesis_triangle_burst(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                              float timeOffset, float initV, float lastV) -> void;
auto synthesis_square(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, float tscale, float riseTime, float fallTime) -> void;
auto synthesis_square_burst(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                            float riseTime, float fallTime, float timeOffset, float initV, float lastV) -> void;
auto synthesis_rampUp(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) -> int;
auto synthesis_rampUp_burst(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                            float timeOffset, float initV, float lastV) -> void;
auto synthesis_rampDown(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) -> int;
auto synthesis_rampDown_burst(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                              float timeOffset, float initV, float lastV) -> void;
auto synthesis_DC(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff) -> int;
auto synthesis_DC_burst(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                        float timeOffset, float initV, float lastV) -> void;
auto synthesis_DC_NEG(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff) -> int;
auto synthesis_DC_NEG_burst(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                            float timeOffset, float initV, float lastV) -> void;
auto synthesis_PWM(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, float ratio, float tscale) -> int;
auto synthesis_PWM_burst(CFloatBase64Signal* signal, float freq, float phase, float amp, float off, float showOff, float ratio, int burstCount, int burst_period, int reps,
                         float tscale, float timeOffset, float initV, float lastV) -> void;
auto synthesis_sweep(CFloatBase64Signal* signal, float frequency, float frequency_start, float frequency_end, rp_gen_sweep_mode_t mode, rp_gen_sweep_dir_t dir, float phase,
                     float amp, float off, float showOff, float tscale) -> int;
auto synthesis_noise(CFloatBase64Signal* signal, float amp, float off, float showOff, float tscale) -> void;
