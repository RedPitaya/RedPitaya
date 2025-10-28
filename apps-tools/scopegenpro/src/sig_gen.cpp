#include "sig_gen.h"
#include <math.h>
#include "rp_hw-profiles.h"

/***************************************************************************************
*                            SIGNAL GENERATING TEMPORARY                                *
****************************************************************************************/

void synthesis_arb(CFloatBinarySignal* signal, const float* data, uint32_t _size, float freq, float amp, float off, float showOff, float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;
    float rate = (float)_size / (float)DAC_BUFFER_SIZE;
    int period = (int)sigSize * 1000 / (freq * tscale * 10) * rate;
    if (period == 0)
        period = 1;
    for (size_t i = 0; i < sigSize; ++i) {
        auto x = i % period;

        auto t = (float)x / (float)period;
        int z = ((int)(t * _size) % _size);
        (*signal)[i] = data[z] * amp + off + showOff;
    }
}

void synthesis_arb_burst(CFloatBinarySignal* signal, const float* data, uint32_t _size, float freq, float amp, float off, float showOff, int burstCount, int periodBrust, int reps,
                         float tscale, float timeOffset, float initV, float lastV) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    float point_time = (tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    // Calculate burst duration in seconds
    float burst_duration = (float)burstCount / freq;

    float period_duration = (float)periodBrust / 1000000.0;

    int offsetInPoints = std::round(-(timeOffset / 1000.0) / point_time);
    int position = sigSize / 2 + offsetInPoints;

    if (position < 0) {
        position = 0;
    } else if (position >= sigSize) {
        position = sigSize - 1;
    }

    // Fill beginning with offset + showOff
    for (int i = 0; i < position; i++) {
        (*signal)[i] = initV + off + showOff;
    }

    float current_time = 0;
    int rep = 0;
    bool in_burst = false;

    for (int i = position; i < sigSize; i++) {
        if (rep >= reps) {
            // All repetitions done - fill with last value
            (*signal)[i] = data[_size - 1] * amp + off + showOff;
            continue;
        }

        if (current_time <= burst_duration) {
            // In burst - generate arbitrary signal
            in_burst = true;
            float z = burst_duration / burstCount;
            float t = std::fmod(current_time, z) / z;
            int data_index = (int)(t * _size) % _size;
            (*signal)[i] = data[data_index] * amp + off + showOff;
        } else if (current_time <= period_duration) {
            // In pause - fill with last value
            in_burst = false;
            (*signal)[i] = lastV + off + showOff;
        } else {
            // Period completed - move to next repetition
            rep++;
            if (rep < reps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = lastV + off + showOff;
            }
        }

        current_time += point_time;
    }
}

void synthesis_sin(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    // float tscale = atof(inTimeScale.Value().c_str());
    for (int i = 0; i < sigSize; i++) {
        (*signal)[i] = (float)(sin(2 * M_PI * (float)i / (float)sigSize * (freq * tscale / 1000) * 10 + phase) * amp + off + showOff);
    }
}

void synthesis_sin_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                         float timeOffset, float initV, float lastV) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    phase *= -1;

    // Calculate time per point in seconds
    float point_time = (tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    // Calculate phase increment per point
    float delta_phase = 2 * M_PI * freq * point_time;

    // Calculate exact number of points for burstCount sine periods
    float burst_duration = (float)burstCount / freq;  // burst duration in seconds

    // Calculate number of points for full period (burst + pause)
    float period_duration = (float)period / 1000000.0;  // period duration in seconds

    int offsetInPoints = std::round(-(timeOffset / 1000.0) / point_time);

    int position = sigSize / 2 + offsetInPoints;

    if (position < 0) {
        position = 0;
    } else if (position >= sigSize) {
        position = sigSize - 1;
    }

    float current_phase = phase;
    float current_time = 0;
    int x = 0;
    int rep = 0;
    bool pause = false;

    for (int i = 0; i < position; i++) {
        (*signal)[i] = initV + off + showOff;
    }

    for (int i = position; i < sigSize; i++) {
        if (current_time <= burst_duration) {
            (*signal)[i] = sin(current_phase) * amp + off + showOff;
        } else if (current_time <= period_duration) {
            (*signal)[i] = lastV + off + showOff;
            pause = true;
        } else {
            if (rep >= reps - 1) {
                (*signal)[i] = lastV + off + showOff;
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

void synthesis_triangle(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    // float tscale = atof(inTimeScale.Value().c_str());
    for (int i = 0; i < sigSize; i++) {
        (*signal)[i] = (float)((asin(sin(2 * M_PI * (float)i / (float)sigSize * (freq * tscale / 1000) * 10 + phase))) / M_PI * 2 * amp + off + showOff);
    }
}

void synthesis_triangle_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                              float timeOffset, float initV, float lastV) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    phase *= -1;

    // Calculate time per point in seconds
    float point_time = (tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    // Calculate phase increment per point
    float delta_phase = 2 * M_PI * freq * point_time;

    // Calculate exact number of points for burstCount sine periods
    float burst_duration = (float)burstCount / freq;  // burst duration in seconds

    // Calculate number of points for full period (burst + pause)
    float period_duration = (float)period / 1000000.0;  // period duration in seconds

    int offsetInPoints = std::round(-(timeOffset / 1000.0) / point_time);

    int position = sigSize / 2 + offsetInPoints;

    if (position < 0) {
        position = 0;
    } else if (position >= sigSize) {
        position = sigSize - 1;
    }

    float current_phase = phase;
    float current_time = 0;
    int x = 0;
    int rep = 0;
    bool pause = false;

    for (int i = 0; i < position; i++) {
        (*signal)[i] = initV + off + showOff;
    }

    for (int i = position; i < sigSize; i++) {
        if (current_time <= burst_duration) {
            (*signal)[i] = (asin(sin(current_phase))) / M_PI * 2 * amp + off + showOff;
        } else if (current_time <= period_duration) {
            (*signal)[i] = lastV + off + showOff;
            pause = true;
        } else {
            if (rep >= reps - 1) {
                (*signal)[i] = lastV + off + showOff;
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

void synthesis_square(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float tscale, float riseTime, float fallTime) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    int period = (int)sigSize * 1000 / (freq * tscale * 10);
    int riseCount = riseTime * sigSize / (tscale * 10 * 1000);
    if (riseCount == 0)
        riseCount = 1;
    int fallCount = fallTime * sigSize / (tscale * 10 * 1000);
    if (fallCount == 0)
        fallCount = 1;
    if (period == 0)
        period = 1;

    int phaseCount = phase * period / (2 * M_PI) + period;  // + period so that there is no error with a negative phase value.
    int t = 0;
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
        (*signal)[i] = off + showOff + amp * z;
    }
}

void synthesis_square_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                            float riseTime, float fallTime, float timeOffset, float initV, float lastV) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    phase *= -1;

    float point_time = (tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    float burst_duration = (float)burstCount / freq;

    float period_duration = (float)period / 1000000.0;

    int offsetInPoints = std::round(-(timeOffset / 1000.0) / point_time);
    int position = sigSize / 2 + offsetInPoints;

    if (position < 0) {
        position = 0;
    } else if (position >= sigSize) {
        position = sigSize - 1;
    }

    // Calculate rise and fall times in points
    int risePoints = std::round(riseTime / 1000000.0 / point_time);
    int fallPoints = std::round(fallTime / 1000000.0 / point_time);

    if (risePoints == 0)
        risePoints = 1;
    if (fallPoints == 0)
        fallPoints = 1;

    // Fill beginning with offset + showOff
    for (int i = 0; i < position; i++) {
        (*signal)[i] = initV + off + showOff;
    }

    float current_time = 0;
    int rep = 0;
    bool in_burst = false;

    for (int i = position; i < sigSize; i++) {
        if (rep >= reps) {
            // All repetitions done - fill with offset
            (*signal)[i] = lastV + off + showOff;
            continue;
        }

        if (current_time <= burst_duration) {
            // In burst - generate square wave
            in_burst = true;

            // Calculate position within square wave period
            float square_period = 1.0f / freq;
            float time_in_period = std::fmod(current_time, square_period);
            float normalized_time = time_in_period / square_period;

            // Apply phase shift
            normalized_time = std::fmod(normalized_time + phase / (2 * M_PI), 1.0f);

            // Generate square wave with rise/fall times
            float value = 0.0f;
            if (normalized_time < 0)
                normalized_time += 1;
            if (normalized_time > 1)
                normalized_time -= 1;

            if (normalized_time < 0.5f) {
                // First half period - positive pulse
                float pulse_time = normalized_time * 2.0f;  // 0 to 1 within positive half

                if (pulse_time < (float)risePoints * point_time / square_period) {
                    // Rise edge
                    value = pulse_time / ((float)risePoints * point_time / square_period);
                } else if (pulse_time > 1.0f - (float)fallPoints * point_time / square_period) {
                    // Fall edge
                    value = 1.0f - (pulse_time - (1.0f - (float)fallPoints * point_time / square_period)) / ((float)fallPoints * point_time / square_period);
                } else {
                    // Flat top
                    value = 1.0f;
                }
            } else {
                // Second half period - negative pulse
                float pulse_time = (normalized_time - 0.5f) * 2.0f;  // 0 to 1 within negative half

                if (pulse_time < (float)fallPoints * point_time / square_period) {
                    // Fall edge (from 0 to -1)
                    value = -pulse_time / ((float)fallPoints * point_time / square_period);
                } else if (pulse_time > 1.0f - (float)risePoints * point_time / square_period) {
                    // Rise edge (from -1 to 0)
                    value = -1.0f + (pulse_time - (1.0f - (float)risePoints * point_time / square_period)) / ((float)risePoints * point_time / square_period);
                } else {
                    // Flat bottom
                    value = -1.0f;
                }
            }

            (*signal)[i] = off + showOff + amp * value;

        } else if (current_time <= period_duration) {
            // In pause - fill with offset
            in_burst = false;
            (*signal)[i] = lastV + off + showOff;
        } else {
            // Period completed - move to next repetition
            rep++;
            if (rep < reps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = lastV + off + showOff;
            }
        }

        current_time += point_time;
    }
}

int synthesis_rampUp(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    // float tscale = atof(inTimeScale.Value().c_str());
    float shift = 0;
    for (int unsigned i = 0; i < sigSize; i++) {
        float angle = M_PI * (float)i / (float)sigSize * (freq * tscale / 1000) * 10 + phase - shift;
        if (angle > M_PI) {
            angle -= M_PI;
            shift += M_PI;
        }
        (*signal)[sigSize - i - 1] = (float)(-1.0 * (acos(cos(angle)) / M_PI - 1)) * amp + off + showOff;
    }
    return RP_OK;
}
void synthesis_rampUp_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                            float timeOffset, float initV, float lastV) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    phase *= -1;

    float point_time = (tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    float burst_duration = (float)burstCount / freq;

    float period_duration = (float)period / 1000000.0;

    int offsetInPoints = std::round(-(timeOffset / 1000.0) / point_time);
    int position = sigSize / 2 + offsetInPoints;

    if (position < 0) {
        position = 0;
    } else if (position >= sigSize) {
        position = sigSize - 1;
    }

    for (int i = 0; i < position; i++) {
        (*signal)[i] = initV + off + showOff;
    }

    float current_time = 0;
    int rep = 0;
    float global_phase_offset = 0;

    for (int i = position; i < sigSize; i++) {
        if (rep >= reps) {
            // All repetitions done - fill with offset
            (*signal)[i] = lastV + off + showOff;
            continue;
        }

        if (current_time <= burst_duration) {
            float ramp_period = 1.0f / freq;

            float total_burst_time = current_time + global_phase_offset;
            float time_in_period = std::fmod(total_burst_time + phase / (2 * M_PI * freq), ramp_period);

            if (time_in_period < 0) {
                time_in_period += ramp_period;
            }

            // Generate ramp signal: 0 to 1 over each period
            float normalized_ramp = time_in_period / ramp_period;
            float value = normalized_ramp;  // Linear ramp from 0 to 1

            (*signal)[i] = off + showOff + amp * value;

        } else if (current_time <= period_duration) {
            // In pause - fill with offset
            (*signal)[i] = lastV + off + showOff;
        } else {
            // Period completed - move to next repetition
            global_phase_offset += burst_duration;
            rep++;
            if (rep < reps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = lastV + off + showOff;
            }
        }

        current_time += point_time;
    }
}

int synthesis_rampDown(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    // float tscale = atof(inTimeScale.Value().c_str());
    float shift = 0;
    for (int unsigned i = 0; i < sigSize; i++) {
        float angle = M_PI * (float)i / (float)sigSize * (freq * tscale / 1000) * 10 + phase - shift;
        if (angle > M_PI) {
            angle -= M_PI;
            shift += M_PI;
        }
        (*signal)[i] = (float)(-1.0 * (acos(cos(angle)) / M_PI - 1)) * amp + off + showOff;
    }
    return RP_OK;
}

void synthesis_rampDown_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                              float timeOffset, float initV, float lastV) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    phase *= -1;

    float point_time = (tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    // Calculate burst duration in seconds
    float burst_duration = (float)burstCount / freq;

    float period_duration = (float)period / 1000000.0;

    int offsetInPoints = std::round(-(timeOffset / 1000.0) / point_time);
    int position = sigSize / 2 + offsetInPoints;

    if (position < 0) {
        position = 0;
    } else if (position >= sigSize) {
        position = sigSize - 1;
    }

    // Fill beginning with offset + showOff
    for (int i = 0; i < position; i++) {
        (*signal)[i] = initV + off + showOff;
    }

    float current_time = 0;
    int rep = 0;
    float global_phase_offset = 0;

    for (int i = position; i < sigSize; i++) {
        if (rep >= reps) {
            // All repetitions done - fill with offset
            (*signal)[i] = lastV + off + showOff;
            continue;
        }

        if (current_time <= burst_duration) {
            float ramp_period = 1.0f / freq;

            float total_burst_time = current_time + global_phase_offset;
            float time_in_period = std::fmod(total_burst_time + phase / (2 * M_PI * freq), ramp_period);

            if (time_in_period < 0) {
                time_in_period += ramp_period;
            }

            // Generate ramp down signal: 1 to 0 over each period
            float normalized_ramp = time_in_period / ramp_period;
            float value = 1.0f - normalized_ramp;  // Linear ramp from 1 to 0

            (*signal)[i] = off + showOff + amp * value;

        } else if (current_time <= period_duration) {
            // In pause - fill with offset
            (*signal)[i] = lastV + off + showOff;
        } else {
            // Period completed - move to next repetition
            global_phase_offset += burst_duration;
            rep++;
            if (rep < reps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = lastV + off + showOff;
            }
        }

        current_time += point_time;
    }
}

int synthesis_DC(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    for (int i = 0; i < signal->GetSize(); i++) {
        (*signal)[i] = 1.0 * amp + off + showOff;
    }
    return RP_OK;
}

void synthesis_DC_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                        float timeOffset, float initV, float lastV) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    float point_time = (tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    // Calculate burst duration in seconds
    float burst_duration = (float)burstCount / freq;

    float period_duration = (float)period / 1000000.0;

    int offsetInPoints = std::round(-(timeOffset / 1000.0) / point_time);
    int position = sigSize / 2 + offsetInPoints;

    if (position < 0) {
        position = 0;
    } else if (position >= sigSize) {
        position = sigSize - 1;
    }

    // Fill beginning with offset + showOff
    for (int i = 0; i < position; i++) {
        (*signal)[i] = initV + off + showOff;
    }

    float current_time = 0;
    int rep = 0;

    for (int i = position; i < sigSize; i++) {
        if (rep >= reps) {
            // All repetitions done - fill with offset
            (*signal)[i] = lastV + off + showOff;
            continue;
        }

        if (current_time <= burst_duration) {
            // In burst - generate DC signal (constant high level)
            (*signal)[i] = amp + off + showOff;
        } else if (current_time <= period_duration) {
            // In pause - fill with offset
            (*signal)[i] = lastV + off + showOff;
        } else {
            // Period completed - move to next repetition
            rep++;
            if (rep < reps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = lastV + off + showOff;
            }
        }

        current_time += point_time;
    }
}

int synthesis_DC_NEG(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    for (int i = 0; i < signal->GetSize(); i++) {
        (*signal)[i] = -1.0 * amp + off + showOff;
    }
    return RP_OK;
}

void synthesis_DC_NEG_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, int burstCount, int period, int reps, float tscale,
                            float timeOffset, float initV, float lastV) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    float point_time = (tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;  // seconds

    // Calculate burst duration in seconds
    float burst_duration = (float)burstCount / freq;

    float period_duration = (float)period / 1000000.0;

    int offsetInPoints = std::round(-(timeOffset / 1000.0) / point_time);
    int position = sigSize / 2 + offsetInPoints;

    if (position < 0) {
        position = 0;
    } else if (position >= sigSize) {
        position = sigSize - 1;
    }

    // Fill beginning with offset + showOff
    for (int i = 0; i < position; i++) {
        (*signal)[i] = initV + off + showOff;
    }

    float current_time = 0;
    int rep = 0;

    for (int i = position; i < sigSize; i++) {
        if (rep >= reps) {
            // All repetitions done - fill with offset
            (*signal)[i] = lastV + off + showOff;
            continue;
        }

        if (current_time <= burst_duration) {
            // In burst - generate DC signal (constant high level)
            (*signal)[i] = -1.0 * amp + off + showOff;
        } else if (current_time <= period_duration) {
            // In pause - fill with offset
            (*signal)[i] = lastV + off + showOff;
        } else {
            // Period completed - move to next repetition
            rep++;
            if (rep < reps) {
                current_time -= period_duration;
                i--;  // Reprocess this point in the next burst
                continue;
            } else {
                (*signal)[i] = lastV + off + showOff;
            }
        }

        current_time += point_time;
    }
}

int synthesis_PWM(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float ratio /*duty cycle*/, float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    // float tscale = atof(inTimeScale.Value().c_str());
    float period = (float)sigSize / (freq * tscale * 10.f / 1000.f);
    float duty = period * ratio;
    float fphase = period * phase / (2.f * M_PI);

    float shift = 0;
    for (int i = 0; i < sigSize; i++) {
        float value = (float)i + fphase - shift;
        if (value > period) {
            value -= period;
            shift += period;
        }
        (*signal)[i] = (value > duty) ? (-amp + off + showOff) : (amp + off + showOff);
    }
    return RP_OK;
}

void synthesis_PWM_burst(CFloatBinarySignal* signal, float freq, float phase, float amp, float off, float showOff, float ratio, int burstCount, int burst_period, int reps,
                         float tscale, float timeOffset, float initV, float lastV) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return;

    phase *= -1;

    // Calculate time parameters
    float point_time = (tscale * 10.0) / (float)CH_SIGNAL_SIZE_DEFAULT / 1000.0;
    float burst_duration = (float)burstCount / freq;
    float period_duration = (float)burst_period / 1000000.0;

    // Calculate PWM period in points
    float pwm_period_points = 1.0f / (freq * point_time);  // Period in points
    float duty_points = pwm_period_points * ratio;         // Duty cycle in points

    // Convert phase to points with support for negative phase
    float phase_points = pwm_period_points * phase / (2 * M_PI);

    // Calculate start position
    int offsetInPoints = std::round(-(timeOffset / 1000.0) / point_time);
    int position = sigSize / 2 + offsetInPoints;

    if (position < 0)
        position = 0;
    else if (position >= sigSize)
        position = sigSize - 1;

    // Signal values
    float high_value = amp + off + showOff;
    float low_value = -amp + off + showOff;
    float pause_value = off + showOff;

    // Fill beginning
    for (int i = 0; i < position; i++) {
        (*signal)[i] = initV + pause_value;
    }

    float current_time = 0;
    int rep = 0;
    float accumulated_phase = 0;

    for (int i = position; i < sigSize; i++) {
        if (rep >= reps) {
            (*signal)[i] = lastV + pause_value;
        } else if (current_time < burst_duration) {
            float absolute_point = current_time / point_time;  // Absolute point from burst start
            float point_in_period = std::fmod(absolute_point + phase_points + accumulated_phase, pwm_period_points);

            // Handle negative values for negative phase
            if (point_in_period < 0) {
                point_in_period += pwm_period_points;
            }

            // PWM decision
            if (point_in_period <= duty_points) {
                (*signal)[i] = high_value;
            } else {
                (*signal)[i] = low_value;
            }

        } else if (current_time < period_duration) {
            (*signal)[i] = lastV + pause_value;
        } else {
            accumulated_phase = std::fmod((burst_duration + phase / (2 * M_PI * freq)) / point_time, pwm_period_points);
            rep++;
            current_time = 0;
            i--;  // Process same point again in next burst
            continue;
        }

        current_time += point_time;
    }
}

int synthesis_sweep(CFloatBinarySignal* signal, float frequency, float frequency_start, float frequency_end, rp_gen_sweep_mode_t mode, rp_gen_sweep_dir_t dir, float phase,
                    float amp, float off, float showOff, float tscale) {

    auto sigSize = (*signal).GetSize();
    if (sigSize == 0)
        return RP_OK;
    bool inverDir = false;
    if (frequency_end < frequency_start) {
        inverDir = true;
    }
    // double tscale = (double)atof(inTimeScale.Value().c_str());
    float period = (float)sigSize / (frequency * tscale * 10.f / 1000.f);
    for (auto i = 0; i < sigSize; i++) {
        float sign = 1;
        double x = ((double)i - ((float)sigSize / 2.0)) / period;
        x = x - floor(x);
        if (dir == RP_GEN_SWEEP_DIR_UP_DOWN) {
            x = x * 2;
            if (x > 1) {
                x = 2 - x;
                sign = -1;
            }
        }
        double freq = 0;
        if (mode == RP_GEN_SWEEP_MODE_LINEAR) {
            freq = ((frequency_end - frequency_start) * x + frequency_start) * 2;
        }
        if (mode == RP_GEN_SWEEP_MODE_LOG) {
            freq = frequency_start * exp(x * log(frequency_end / frequency_start));
        }
        if (inverDir)
            x = 1 - x;
        (*signal)[i] = sin(freq * 2 * M_PI * (x) / frequency + phase) * sign * amp + off + showOff;
    }
    return RP_OK;
}

auto synthesis_noise(CFloatBinarySignal* signal, float amp, float off, float showOff, float tscale) -> void {
    // auto clockRate = rp_HPGetBaseFastDACSpeedHzOrDefault();
    auto sigSize = (*signal).GetSize();
    for (int i = 0; i < signal->GetSize(); i++) {
        auto r = ((double)rand() / (RAND_MAX)) * 2.0 - 1.0;
        (*signal)[i] = r * amp + off + showOff;
    }
}