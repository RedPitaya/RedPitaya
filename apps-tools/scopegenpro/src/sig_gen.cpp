#include "sig_gen.h"
#include <math.h>
#include <algorithm>
#include "rp_hw-profiles.h"

/***************************************************************************************
*                            SIGNAL GENERATING TEMPORARY                                *
****************************************************************************************/

void synthesis_arb(CFloatBinarySignal* signal, const float* data, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0 || settings.arb_size <= 0)
        return;

    double phaseStep = (double)settings.frequency * settings.tscale * 0.01 * settings.arb_size / sigSize;

    double currentPhase = 0.0;
    for (int i = 0; i < sigSize; ++i) {
        int64_t z = (int64_t)currentPhase % (int64_t)settings.arb_size;

        (*signal)[i] = data[z] * settings.amplitude + settings.offset + settings.showOff;

        currentPhase += phaseStep;
    }
}

void synthesis_arb_burst(CFloatBinarySignal* signal, const float* data, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    float point_time = (settings.tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    // Calculate burst duration in seconds
    float burst_duration = (float)settings.burstCount / settings.frequency;

    float period_duration = (float)settings.burstPeriod / 1000000.0;
    auto lastV = settings.lastValue;

    period_duration = std::max(burst_duration, period_duration);

    int offsetInPoints = std::round(-(settings.timeOffset / 1000.0) / point_time);
    int position = sigSize / 2 + offsetInPoints;

    if (position < 0) {
        position = 0;
    } else if (position >= sigSize) {
        position = sigSize - 1;
    }

    // Fill beginning with offset + showOff
    for (int i = 0; i < position; i++) {
        (*signal)[i] = settings.initValue + settings.offset + settings.showOff;
    }

    if (settings.useLastSample) {
        lastV = data[settings.arb_size - 1] * settings.amplitude;
    }

    float current_time = 0;
    int rep = 0;
    int i = 0;
    for (i = position; i < sigSize; i++) {
        if (rep >= settings.burstReps) {
            // All repetitions done - fill with last value
            (*signal)[i] = lastV + settings.offset + settings.showOff;
            continue;
        }

        if (current_time <= burst_duration) {
            // In burst - generate arbitrary signal
            float z = burst_duration / settings.burstCount;
            float t = std::fmod(current_time, z) / z;
            int data_index = (int)(t * settings.arb_size) % settings.arb_size;
            (*signal)[i] = data[data_index] * settings.amplitude + settings.offset + settings.showOff;
        } else if (current_time <= period_duration) {
            // In pause - fill with last value
            (*signal)[i] = lastV + settings.offset + settings.showOff;
        } else {
            // Period completed - move to next repetition
            rep++;
            if (rep < settings.burstReps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = lastV + settings.offset + settings.showOff;
            }
        }

        current_time += point_time;
    }
}

auto synthesis_noise(CFloatBinarySignal* signal, const GenChannelSettings& settings) -> void {
    // auto clockRate = rp_HPGetBaseFastDACSpeedHzOrDefault();
    for (int i = 0; i < signal->GetSize(); i++) {
        auto r = ((double)rand() / (RAND_MAX)) * 2.0 - 1.0;
        (*signal)[i] = r * settings.amplitude + settings.offset + settings.showOff;
    }
}