#pragma once

#include <DataManager.h>
#include <CustomParameters.h>
#include "rp.h"

#define CH_SIGNAL_SIZE_DEFAULT		1024

auto synthesis_arb(CFloatSignal *signal, const float *data, uint32_t _size, float freq, float amp, float off, float showOff,float tscale) -> void;
auto synthesis_sin(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,float tscale) -> void;
auto synthesis_sin_burst(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) -> void;
auto synthesis_triangle(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,float tscale) -> void;
auto synthesis_triangle_burst(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) -> void;
auto synthesis_square(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,float tscale, float riseTime, float fallTime) -> void;
auto synthesis_square_burst(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale, float riseTime, float fallTime) -> void;
auto synthesis_rampUp(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,float tscale) -> int;
auto synthesis_rampUp_burst(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) -> void;
auto synthesis_rampDown(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,float tscale) -> int;
auto synthesis_rampDown_burst(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) -> void;
auto synthesis_DC(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff) -> int;
auto synthesis_DC_burst(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) -> void;
auto synthesis_DC_NEG(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff) -> int;
auto synthesis_DC_NEG_burst(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) -> void;
auto synthesis_PWM(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff, float ratio,float tscale) -> int;
auto synthesis_PWM_burst(CFloatSignal *signal, float freq, float phase, float amp, float off, float showOff, float ratio ,int burstCount, int burst_period, int reps,float tscale) -> void;
auto synthesis_sweep(CFloatSignal *signal,float frequency,float frequency_start,float frequency_end,rp_gen_sweep_mode_t mode,rp_gen_sweep_dir_t dir,float phase, float amp, float off, float showOff,float tscale) -> int;
