#pragma once

#include <CustomParameters.h>
#include <DataManager.h>
#include <cmath>
#include "rp.h"

#define CH_SIGNAL_SIZE_DEFAULT 1024

struct GenChannelSettings {
    // Basic Signal Parameters
    std::string waveform;  // Signal shape type (Sine, Square, etc.)
    float frequency;       // Main signal frequency in Hz
    float phase;           // Phase shift in radians
    float amplitude;       // Normalized amplitude (V)
    float offset;          // Normalized DC offset (V)
    float showOff;         // Scaled offset for UI display
    float duty_cycle;      // PWM duty cycle (0.0 to 1.0)
    uint32_t arb_size;

    // Frequency Sweep Settings
    float freqSweepStart;            // Sweep start frequency in Hz
    float freqSweepEnd;              // Sweep end frequency in Hz
    rp_gen_sweep_mode_t sweep_mode;  // Linear or Logarithmic mode
    rp_gen_sweep_dir_t sweep_dir;    // Up, Down, or Up-Down direction

    // Burst Mode Configuration
    rp_gen_mode_t gen_mode;  // Continuous or Burst generation mode
    int burstCount;          // Cycles per burst
    double burstPeriod;      // Delay/Period between bursts
    int burstReps;           // Number of burst repetitions

    // Waveform Transition & Sample Control
    float riseTime;      // Signal rise time
    float fallTime;      // Signal fall time
    float initValue;     // Initial voltage level for burst
    float lastValue;     // Final voltage level for burst
    bool useLastSample;  // Maintain last sample level after burst

    // System Integration
    float timeOffset;  // Oscilloscope time synchronization offset
    float tscale;
    /**
     * @brief Constructor that accepts direct values.
     */
    GenChannelSettings(std::string _waveform, float _frequency, float _phase, float _amplitude, float _offset, float _showOff, float _duty_cycle, float _freqSweepStart,
                       float _freqSweepEnd, rp_gen_sweep_mode_t _sweep_mode, rp_gen_sweep_dir_t _sweep_dir, rp_gen_mode_t _gen_mode, int _burstCount, double _burstPeriod,
                       int _burstReps, float _riseTime, float _fallTime, float _initValue, float _lastValue, bool _useLastSample, float _timeOffset, float _tscale)
        : waveform(_waveform),
          frequency(_frequency),
          phase(_phase),
          amplitude(_amplitude),
          offset(_offset),
          showOff(_showOff),
          duty_cycle(_duty_cycle),
          arb_size(0),
          freqSweepStart(_freqSweepStart),
          freqSweepEnd(_freqSweepEnd),
          sweep_mode(_sweep_mode),
          sweep_dir(_sweep_dir),
          gen_mode(_gen_mode),
          burstCount(_burstCount),
          burstPeriod(_burstPeriod),
          burstReps(_burstReps),
          riseTime(_riseTime),
          fallTime(_fallTime),
          initValue(_initValue),
          lastValue(_lastValue),
          useLastSample(_useLastSample),
          timeOffset(_timeOffset),
          tscale(_tscale) {}

    GenChannelSettings(){};
    /**
     * @brief Comprehensive equality operator.
     * Note: Uses direct comparison for floats. If precision jitter is an issue,
     * consider using an epsilon-based comparison.
     */
    bool operator==(const GenChannelSettings& o) const {
        return (waveform == o.waveform) && (frequency == o.frequency) && (phase == o.phase) && (amplitude == o.amplitude) && (offset == o.offset) && (showOff == o.showOff) &&
               (duty_cycle == o.duty_cycle) && (freqSweepStart == o.freqSweepStart) && (freqSweepEnd == o.freqSweepEnd) && (sweep_mode == o.sweep_mode) &&
               (sweep_dir == o.sweep_dir) && (gen_mode == o.gen_mode) && (burstCount == o.burstCount) && (burstPeriod == o.burstPeriod) && (burstReps == o.burstReps) &&
               (riseTime == o.riseTime) && (fallTime == o.fallTime) && (initValue == o.initValue) && (lastValue == o.lastValue) && (useLastSample == o.useLastSample) &&
               (timeOffset == o.timeOffset) && (tscale == o.tscale) && (arb_size == o.arb_size);
    }

    bool operator!=(const GenChannelSettings& other) const { return !(*this == other); }
};

auto synthesis_arb(CFloatBinarySignal* signal, const float* data, const GenChannelSettings& settings) -> void;
auto synthesis_arb_burst(CFloatBinarySignal* signal, const float* data, const GenChannelSettings& settings) -> void;
auto synthesis_noise(CFloatBinarySignal* signal, const GenChannelSettings& settings) -> void;
