#include "sig_gen.h"
#include <math.h>
#include <algorithm>
#include "rp_hw-profiles.h"

/***************************************************************************************
*                            SIGNAL GENERATING TEMPORARY                                *
****************************************************************************************/

void synthesis_arb(CFloatBinarySignal* signal, const float* data, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;
    float rate = 1;  //(float)_size / (float)DAC_BUFFER_SIZE;
    int period = (int)sigSize * 1000 / (settings.frequency * settings.tscale * 10) * rate;
    if (period == 0)
        period = 1;
    for (int i = 0; i < sigSize; ++i) {
        auto x = i % period;

        auto t = (float)x / (float)period;
        int z = ((int)(t * settings.arb_size) % settings.arb_size);
        (*signal)[i] = data[z] * settings.amplitude + settings.offset + settings.showOff;
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
        (*signal)[i] = settings.initValue * 2.0 + settings.offset + settings.showOff;
    }

    if (settings.useLastSample) {
        lastV = data[settings.arb_size - 1];
    }

    float current_time = 0;
    int rep = 0;
    int i = 0;
    for (i = position; i < sigSize; i++) {
        if (rep >= settings.burstReps) {
            // All repetitions done - fill with last value
            (*signal)[i] = lastV * 2.0 + settings.offset + settings.showOff;
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
            (*signal)[i] = lastV * 2.0 + settings.offset + settings.showOff;
        } else {
            // Period completed - move to next repetition
            rep++;
            if (rep < settings.burstReps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = lastV * 2.0 + settings.offset + settings.showOff;
            }
        }

        current_time += point_time;
    }
}

void synthesis_sin(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    // float tscale = atof(inTimeScale.Value().c_str());
    for (int i = 0; i < sigSize; i++) {
        (*signal)[i] = (float)(sin(2 * M_PI * (float)i / (float)sigSize * (settings.frequency * settings.tscale / 1000.f) * 10.f + settings.phase) * settings.amplitude +
                               settings.offset + settings.showOff);
    }
}

void synthesis_sin_burst(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize <= 0)
        return;

    auto phase = settings.phase * -1.0f;

    float point_time = (settings.tscale * 10.0f) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0f;
    if (point_time <= 0)
        return;

    int pointsPerBurst = std::round((float)settings.burstCount / settings.frequency / point_time);
    int pointsPerPeriod = std::round((settings.burstPeriod / 1000000.0f) / point_time);
    pointsPerPeriod = std::max(pointsPerBurst, pointsPerPeriod);

    int offsetInPoints = std::round(-(settings.timeOffset / 1000.0f) / point_time);
    int position = (sigSize / 2) + offsetInPoints;
    position = std::clamp(position, 0, (int)sigSize);

    float delta_phase = 2.0f * (float)M_PI * settings.frequency * point_time;
    float baseLine = settings.offset + settings.showOff;  // The "zero" level of your signal
    float lastSample = settings.initValue * 2.0f + baseLine;

    // Fill initial buffer
    for (int i = 0; i < position; i++) {
        (*signal)[i] = lastSample;
    }

    for (int i = position; i < sigSize; i++) {
        int relIdx = i - position;
        int currentRep = relIdx / pointsPerPeriod;
        int idxInPeriod = relIdx % pointsPerPeriod;

        if (currentRep < settings.burstReps && idxInPeriod < pointsPerBurst) {
            float p = phase + (delta_phase * idxInPeriod);
            lastSample = std::sin(p) * settings.amplitude + baseLine;
            (*signal)[i] = lastSample;
        } else {
            // If not using last sample, smoothly return to lastV or baseLine
            if (!settings.useLastSample) {
                lastSample = settings.lastValue * 2.0f + baseLine;
            } else {
                lastSample = baseLine;
            }
            (*signal)[i] = lastSample;
        }
    }
}

void synthesis_triangle(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    // float tscale = atof(inTimeScale.Value().c_str());
    for (int i = 0; i < sigSize; i++) {
        (*signal)[i] =
            (float)((asin(sin(2 * M_PI * (float)i / (float)sigSize * (settings.frequency * settings.tscale / 1000) * 10 + settings.phase))) / M_PI * 2 * settings.amplitude +
                    settings.offset + settings.showOff);
    }
}

void synthesis_triangle_burst(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    auto phase = settings.phase * -1;

    // Calculate time per point in seconds
    float point_time = (settings.tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    // Calculate phase increment per point
    float delta_phase = 2 * M_PI * settings.frequency * point_time;

    // Calculate exact number of points for burstCount sine periods
    float burst_duration = (float)settings.burstCount / settings.frequency;  // burst duration in seconds

    // Calculate number of points for full period (burst + pause)
    float period_duration = (float)settings.burstPeriod / 1000000.0;  // period duration in seconds

    period_duration = std::max(burst_duration, period_duration);

    int offsetInPoints = std::round(-(settings.timeOffset / 1000.0) / point_time);

    int position = sigSize / 2 + offsetInPoints;

    if (position < 0) {
        position = 0;
    } else if (position >= sigSize) {
        position = sigSize - 1;
    }

    float current_phase = phase;
    float current_time = 0;
    int rep = 0;
    bool pause = false;
    float lastSample = 0;

    for (int i = 0; i < position; i++) {
        (*signal)[i] = settings.initValue * 2.0 + settings.offset + settings.showOff;
    }

    for (int i = position; i < sigSize; i++) {
        if (current_time <= burst_duration) {
            lastSample = (asin(sin(current_phase))) / M_PI * 2 * settings.amplitude + settings.offset + settings.showOff;
            (*signal)[i] = lastSample;
        } else if (current_time <= period_duration) {
            (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + settings.offset + settings.showOff;
            pause = true;
        } else {
            if (rep >= settings.burstReps - 1) {
                (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + settings.offset + settings.showOff;
            } else {
                rep++;
                if (pause) {
                    current_phase = phase;
                    current_time -= period_duration;
                    i--;
                    continue;
                } else {
                    current_time -= burst_duration;
                    i--;
                    continue;
                }
            }
        }
        current_phase += delta_phase;
        current_time += point_time;
    }
}

void synthesis_square(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    int period = (int)sigSize * 1000 / (settings.frequency * settings.tscale * 10);
    int riseCount = settings.riseTime * sigSize / (settings.tscale * 10 * 1000);
    if (riseCount == 0)
        riseCount = 1;
    int fallCount = settings.fallTime * sigSize / (settings.tscale * 10 * 1000);
    if (fallCount == 0)
        fallCount = 1;
    if (period == 0)
        period = 1;

    int phaseCount = settings.phase * period / (2 * M_PI) + period;  // + period so that there is no error with a negative phase value.
    for (int i = 0; i < sigSize; i++) {
        auto x = (i + phaseCount) % period;
        float z = 0;
        if (x < riseCount / 2) {
            z = (float)x / ((float)riseCount / 2.0f);
        } else if (x < (period - fallCount) / 2) {
            z = 1.0f;
        } else if (x < (period + fallCount) / 2) {
            if (fallCount > 1) {
                z = 1.0f - 2.0f * (float)((float)x - ((float)period - (float)fallCount) / 2.0f) / ((float)fallCount);
                if (z > 1.f)
                    z = 1.f;
            } else
                z = 0;
        } else if (x < period - (riseCount / 2)) {
            z = -1.0f;
        } else {
            z = -1.0f + (float)(x - (period - (riseCount / 2.0f))) / ((float)riseCount / 2.0f);
        }
        (*signal)[i] = settings.offset + settings.showOff + settings.amplitude * z;
    }
}

void synthesis_square_burst(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize <= 0)
        return;

    auto phase = settings.phase * -1.0f;

    // 1. Time and Index Calculations
    float point_time = (settings.tscale * 10.0f) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0f;
    if (point_time <= 0)
        return;

    int pointsPerBurst = std::round((float)settings.burstCount / settings.frequency / point_time);
    int pointsPerPeriod = std::round((settings.burstPeriod / 1000000.0f) / point_time);
    pointsPerPeriod = std::max(pointsPerBurst, pointsPerPeriod);

    float pointsPerSquareCycle = 1.0f / settings.frequency / point_time;
    int risePoints = std::max(1, (int)std::round(settings.riseTime / 1000000.0f / point_time));
    int fallPoints = std::max(1, (int)std::round(settings.fallTime / 1000000.0f / point_time));

    int offsetInPoints = std::round(-(settings.timeOffset / 1000.0f) / point_time);
    int position = std::clamp((int)(sigSize / 2 + offsetInPoints), 0, (int)sigSize);

    float baseLine = settings.offset + settings.showOff;
    float finalValue = settings.lastValue * 2.0f + baseLine;  // Target value after signal ends
    float lastSample = settings.initValue * 2.0f + baseLine;

    // Initial fill
    for (int i = 0; i < position; i++) {
        (*signal)[i] = lastSample;
    }

    // 2. Generation Loop
    for (int i = position; i < (int)sigSize; i++) {
        int relIdx = i - position;
        int currentRep = relIdx / pointsPerPeriod;
        int idxInPeriod = relIdx % pointsPerPeriod;

        // Check if we are still within the requested number of repetitions
        if (currentRep < settings.burstReps) {
            if (idxInPeriod < pointsPerBurst) {
                // ACTIVE BURST
                float phaseOffsetPoints = (phase / (2.0f * (float)M_PI)) * pointsPerSquareCycle;
                float localSquareIdx = std::fmod((float)idxInPeriod + phaseOffsetPoints, pointsPerSquareCycle);
                if (localSquareIdx < 0)
                    localSquareIdx += pointsPerSquareCycle;

                float normTime = localSquareIdx / pointsPerSquareCycle;
                float value = 0.0f;

                if (normTime < 0.5f) {  // Positive half
                    float hIdx = localSquareIdx;
                    float hLimit = pointsPerSquareCycle * 0.5f;
                    if (hIdx < (float)risePoints)
                        value = hIdx / (float)risePoints;
                    else if (hIdx > hLimit - (float)fallPoints)
                        value = 1.0f - (hIdx - (hLimit - (float)fallPoints)) / (float)fallPoints;
                    else
                        value = 1.0f;
                } else {  // Negative half
                    float hIdx = localSquareIdx - (pointsPerSquareCycle * 0.5f);
                    float hLimit = pointsPerSquareCycle * 0.5f;
                    if (hIdx < (float)fallPoints)
                        value = -hIdx / (float)fallPoints;
                    else if (hIdx > hLimit - (float)risePoints)
                        value = -1.0f + (hIdx - (hLimit - (float)risePoints)) / (float)risePoints;
                    else
                        value = -1.0f;
                }

                float currentAmp = settings.amplitude;
                lastSample = baseLine + currentAmp * value;
                (*signal)[i] = lastSample;
            } else {
                // PAUSE BETWEEN BURSTS
                lastSample = settings.useLastSample ? baseLine : finalValue;
                (*signal)[i] = lastSample;
            }
        } else {
            // AFTER ALL REPS (Signal Tail)
            // Ensure the rest of the buffer is filled with the correct end level
            (*signal)[i] = settings.useLastSample ? lastSample : finalValue;
        }
    }
}

int synthesis_rampUp(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    // float tscale = atof(inTimeScale.Value().c_str());
    float shift = 0;
    for (int i = 0; i < sigSize; i++) {
        float angle = M_PI * (float)i / (float)sigSize * (settings.frequency * settings.tscale / 1000) * 10 + settings.phase - shift;
        if (angle > M_PI) {
            angle -= M_PI;
            shift += M_PI;
        }
        (*signal)[sigSize - i - 1] = (float)(-1.0 * (acos(cos(angle)) / M_PI - 1)) * settings.amplitude + settings.offset + settings.showOff;
    }
    return RP_OK;
}
void synthesis_rampUp_burst(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    auto phase = settings.phase - 1;

    float point_time = (settings.tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    float burst_duration = (float)settings.burstCount / settings.frequency;

    float period_duration = (float)settings.burstPeriod / 1000000.0;

    period_duration = std::max(burst_duration, period_duration);

    int offsetInPoints = std::round(-(settings.timeOffset / 1000.0) / point_time);
    int position = sigSize / 2 + offsetInPoints;

    if (position < 0) {
        position = 0;
    } else if (position >= sigSize) {
        position = sigSize - 1;
    }
    auto baseLine = settings.offset + settings.showOff;
    for (int i = 0; i < position; i++) {
        (*signal)[i] = settings.initValue * 2.0 + settings.offset + settings.showOff;
    }

    float current_time = 0;
    int rep = 0;
    float global_phase_offset = 0;
    float lastSample = 0;

    for (int i = position; i < sigSize; i++) {
        if (rep >= settings.burstReps) {
            // All repetitions done - fill with offset
            (*signal)[i] = settings.useLastSample ? baseLine : settings.lastValue * 2.0 + settings.offset + settings.showOff;
            continue;
        }

        if (current_time <= burst_duration) {
            float ramp_period = 1.0f / settings.frequency;

            float total_burst_time = current_time + global_phase_offset;
            float time_in_period = std::fmod(total_burst_time + phase / (2 * M_PI * settings.frequency), ramp_period);

            if (time_in_period < 0) {
                time_in_period += ramp_period;
            }

            // Generate ramp signal: 0 to 1 over each period
            float normalized_ramp = time_in_period / ramp_period;
            float value = normalized_ramp;  // Linear ramp from 0 to 1

            lastSample = settings.offset + settings.showOff + settings.amplitude * value;
            (*signal)[i] = lastSample;

        } else if (current_time <= period_duration) {
            // In pause - fill with offset
            (*signal)[i] = settings.useLastSample ? baseLine : settings.lastValue * 2.0 + settings.offset + settings.showOff;
        } else {
            // Period completed - move to next repetition
            global_phase_offset += burst_duration;
            rep++;
            if (rep < settings.burstReps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = settings.useLastSample ? baseLine : settings.lastValue * 2.0 + settings.offset + settings.showOff;
            }
        }

        current_time += point_time;
    }
}

int synthesis_rampDown(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    // float tscale = atof(inTimeScale.Value().c_str());
    float shift = 0;
    for (int i = 0; i < sigSize; i++) {
        float angle = M_PI * (float)i / (float)sigSize * (settings.frequency * settings.tscale / 1000) * 10 + settings.phase - shift;
        if (angle > M_PI) {
            angle -= M_PI;
            shift += M_PI;
        }
        (*signal)[i] = (float)(-1.0 * (acos(cos(angle)) / M_PI - 1)) * settings.amplitude + settings.offset + settings.showOff;
    }
    return RP_OK;
}

void synthesis_rampDown_burst(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    auto phase = settings.phase * -1;

    float point_time = (settings.tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    // Calculate burst duration in seconds
    float burst_duration = (float)settings.burstCount / settings.frequency;
    float period_duration = (float)settings.burstPeriod / 1000000.0;

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
        (*signal)[i] = settings.initValue * 2.0 + settings.offset + settings.showOff;
    }

    float current_time = 0;
    int rep = 0;
    float global_phase_offset = 0;
    float lastSample = 0;

    for (int i = position; i < sigSize; i++) {
        if (rep >= settings.burstReps) {
            // All repetitions done - fill with offset
            (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + settings.offset + settings.showOff;
            continue;
        }

        if (current_time <= burst_duration) {
            float ramp_period = 1.0f / settings.frequency;

            float total_burst_time = current_time + global_phase_offset;
            float time_in_period = std::fmod(total_burst_time + phase / (2 * M_PI * settings.frequency), ramp_period);

            if (time_in_period < 0) {
                time_in_period += ramp_period;
            }

            // Generate ramp down signal: 1 to 0 over each period
            float normalized_ramp = time_in_period / ramp_period;
            float value = 1.0f - normalized_ramp;  // Linear ramp from 1 to 0

            lastSample = settings.offset + settings.showOff + settings.amplitude * value;
            (*signal)[i] = lastSample;

        } else if (current_time <= period_duration) {
            // In pause - fill with offset
            (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + settings.offset + settings.showOff;
        } else {
            // Period completed - move to next repetition
            global_phase_offset += burst_duration;
            rep++;
            if (rep < settings.burstReps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + settings.offset + settings.showOff;
            }
        }

        current_time += point_time;
    }
}

int synthesis_DC(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    for (int i = 0; i < signal->GetSize(); i++) {
        (*signal)[i] = 1.0 * settings.amplitude + settings.offset + settings.showOff;
    }
    return RP_OK;
}

void synthesis_DC_burst(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    float point_time = (settings.tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    // Calculate burst duration in seconds
    float burst_duration = (float)settings.burstCount / settings.frequency;
    float period_duration = (float)settings.burstPeriod / 1000000.0;

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
        (*signal)[i] = settings.initValue * 2.0 + settings.offset + settings.showOff;
    }

    float current_time = 0;
    int rep = 0;
    float lastSample = 0;

    for (int i = position; i < sigSize; i++) {
        if (rep >= settings.burstReps) {
            // All repetitions done - fill with offset
            (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + settings.offset + settings.showOff;
            continue;
        }

        if (current_time <= burst_duration) {
            // In burst - generate DC signal (constant high level)
            lastSample = settings.amplitude + settings.offset + settings.showOff;
            (*signal)[i] = lastSample;
        } else if (current_time <= period_duration) {
            // In pause - fill with offset
            (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + settings.offset + settings.showOff;
        } else {
            // Period completed - move to next repetition
            rep++;
            if (rep < settings.burstReps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + settings.offset + settings.showOff;
            }
        }

        current_time += point_time;
    }
}

int synthesis_DC_NEG(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    for (int i = 0; i < signal->GetSize(); i++) {
        (*signal)[i] = -1.0 * settings.amplitude + settings.offset + settings.showOff;
    }
    return RP_OK;
}

void synthesis_DC_NEG_burst(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    float point_time = (settings.tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    // Calculate burst duration in seconds
    float burst_duration = (float)settings.burstCount / settings.frequency;
    float period_duration = (float)settings.burstPeriod / 1000000.0;

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
        (*signal)[i] = settings.initValue * 2.0 + settings.offset + settings.showOff;
    }

    float current_time = 0;
    int rep = 0;
    float lastSample = 0;

    for (int i = position; i < sigSize; i++) {
        if (rep >= settings.burstReps) {
            // All repetitions done - fill with offset
            (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + settings.offset + settings.showOff;
            continue;
        }

        if (current_time <= burst_duration) {
            // In burst - generate DC signal (constant high level)
            lastSample = -1.0 * settings.amplitude + settings.offset + settings.showOff;
            (*signal)[i] = lastSample;
        } else if (current_time <= period_duration) {
            // In pause - fill with offset
            (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + settings.offset + settings.showOff;
        } else {
            // Period completed - move to next repetition
            rep++;
            if (rep < settings.burstReps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + settings.offset + settings.showOff;
            }
        }

        current_time += point_time;
    }
}

int synthesis_PWM(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    // float tscale = atof(inTimeScale.Value().c_str());
    float period = (float)sigSize / (settings.frequency * settings.tscale * 10.f / 1000.f);
    float duty = period * settings.duty_cycle;
    float fphase = period * settings.phase / (2.f * M_PI);

    float shift = 0;
    for (int i = 0; i < sigSize; i++) {
        float value = (float)i + fphase - shift;
        if (value > period) {
            value -= period;
            shift += period;
        }
        (*signal)[i] = (value > duty) ? (-settings.amplitude + settings.offset + settings.showOff) : (settings.amplitude + settings.offset + settings.showOff);
    }
    return RP_OK;
}

void synthesis_PWM_burst(CFloatBinarySignal* signal, const GenChannelSettings& settings) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    auto phase = settings.phase * -1;

    // Calculate time parameters
    float point_time = (settings.tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;
    float burst_duration = (float)settings.burstCount / settings.frequency;
    float period_duration = (float)settings.burstPeriod / 1000000.0;

    period_duration = std::max(burst_duration, period_duration);

    // Calculate PWM period in points
    float pwm_period_points = 1.0f / (settings.frequency * point_time);  // Period in points
    float duty_points = pwm_period_points * settings.duty_cycle;         // Duty cycle in points

    // Convert phase to points with support for negative phase
    float phase_points = pwm_period_points * phase / (2 * M_PI);

    // Calculate start position
    int offsetInPoints = std::round(-(settings.timeOffset / 1000.0) / point_time);
    int position = sigSize / 2 + offsetInPoints;

    if (position < 0)
        position = 0;
    else if (position >= sigSize)
        position = sigSize - 1;

    // Signal values
    float high_value = settings.amplitude + settings.offset + settings.showOff;
    float low_value = -settings.amplitude + settings.offset + settings.showOff;
    float pause_value = settings.offset + settings.showOff;

    // Fill beginning
    for (int i = 0; i < position; i++) {
        (*signal)[i] = settings.initValue * 2.0 + pause_value;
    }

    float current_time = 0;
    int rep = 0;
    float accumulated_phase = 0;
    float lastSample = 0;

    for (int i = position; i < sigSize; i++) {
        if (rep >= settings.burstReps) {
            (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + pause_value;
        } else if (current_time < burst_duration) {
            float absolute_point = current_time / point_time;  // Absolute point from burst start
            float point_in_period = std::fmod(absolute_point + phase_points + accumulated_phase, pwm_period_points);

            // Handle negative values for negative phase
            if (point_in_period < 0) {
                point_in_period += pwm_period_points;
            }

            // PWM decision
            if (point_in_period <= duty_points) {
                lastSample = high_value;
                (*signal)[i] = lastSample;
            } else {
                lastSample = low_value;
                (*signal)[i] = lastSample;
            }

        } else if (current_time < period_duration) {
            (*signal)[i] = settings.useLastSample ? lastSample : settings.lastValue * 2.0 + pause_value;
        } else {
            accumulated_phase = std::fmod((burst_duration + phase / (2 * M_PI * settings.frequency)) / point_time, pwm_period_points);
            rep++;
            current_time = 0;
            i--;  // Process same point again in next burst
            continue;
        }

        current_time += point_time;
    }
}

int synthesis_sweep(CFloatBinarySignal* signal, const GenChannelSettings& settings) {

    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    bool inverDir = false;
    if (settings.freqSweepEnd < settings.freqSweepStart) {
        inverDir = true;
    }
    // double tscale = (double)atof(inTimeScale.Value().c_str());
    float period = (float)sigSize / (settings.frequency * settings.tscale * 10.f / 1000.f);
    for (auto i = 0; i < sigSize; i++) {
        float sign = 1;
        double x = ((double)i - ((float)sigSize / 2.0)) / period;
        x = x - floor(x);
        if (settings.sweep_dir == RP_GEN_SWEEP_DIR_UP_DOWN) {
            x = x * 2;
            if (x > 1) {
                x = 2 - x;
                sign = -1;
            }
        }
        double freq = 0;
        if (settings.sweep_mode == RP_GEN_SWEEP_MODE_LINEAR) {
            freq = ((settings.freqSweepEnd - settings.freqSweepStart) * x + settings.freqSweepStart) * 2;
        }
        if (settings.sweep_mode == RP_GEN_SWEEP_MODE_LOG) {
            freq = settings.freqSweepStart * exp(x * log(settings.freqSweepEnd / settings.freqSweepStart));
        }
        if (inverDir)
            x = 1 - x;
        (*signal)[i] = sin(freq * 2 * M_PI * (x) / settings.frequency + settings.phase) * sign * settings.amplitude + settings.offset + settings.showOff;
    }
    return RP_OK;
}

auto synthesis_noise(CFloatBinarySignal* signal, const GenChannelSettings& settings) -> void {
    // auto clockRate = rp_HPGetBaseFastDACSpeedHzOrDefault();
    for (int i = 0; i < signal->GetSize(); i++) {
        auto r = ((double)rand() / (RAND_MAX)) * 2.0 - 1.0;
        (*signal)[i] = r * settings.amplitude + settings.offset + settings.showOff;
    }
}