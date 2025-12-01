#pragma once

#include <CustomParameters.h>
#include <DataManager.h>
#include "rp.h"

#define CH_SIGNAL_SIZE_DEFAULT 1024

auto synthesis_arb(CFloatBinarySignal* signal, const float* data, uint32_t _size, float freq, float amp, float off, float showOff, float tscale) -> void;
auto synthesis_arb_burst(CFloatBinarySignal* signal, const float* data, uint32_t _size, float freq, float amp, float off, float showOff, int burstCount, int periodBrust, int reps,
                         float tscale, float timeOffset, float initV, float lastV) -> void;
auto synthesis_sin(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) -> void;
auto synthesis_sin_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                         float timeOffset, float initV, float lastV) -> void;
auto synthesis_triangle(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) -> void;
auto synthesis_triangle_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                              float timeOffset, float initV, float lastV) -> void;
auto synthesis_square(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float tscale, float riseTime, float fallTime) -> void;
auto synthesis_square_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                            float riseTime, float fallTime, float timeOffset, float initV, float lastV) -> void;
auto synthesis_rampUp(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) -> int;
auto synthesis_rampUp_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                            float timeOffset, float initV, float lastV) -> void;
auto synthesis_rampDown(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) -> int;
auto synthesis_rampDown_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                              float timeOffset, float initV, float lastV) -> void;
auto synthesis_DC(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff) -> int;
auto synthesis_DC_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                        float timeOffset, float initV, float lastV) -> void;
auto synthesis_DC_NEG(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff) -> int;
auto synthesis_DC_NEG_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                            float timeOffset, float initV, float lastV) -> void;
auto synthesis_PWM(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float ratio, float tscale) -> int;
auto synthesis_PWM_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float ratio, int burstCount, int burst_period, int reps,
                         float tscale, float timeOffset, float initV, float lastV) -> void;
auto synthesis_sweep(CFloatBinarySignal* signal, float frequency, float frequency_start, float frequency_end, rp_gen_sweep_mode_t mode, rp_gen_sweep_dir_t dir, float phase,
                     float amp, float off, float showOff, float tscale) -> int;
auto synthesis_noise(CFloatBinarySignal* signal, float amp, float off, float showOff, float tscale) -> void;
