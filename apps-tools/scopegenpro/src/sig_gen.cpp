#include <math.h>
#include "sig_gen.h"
#include "rp_hw-profiles.h"

/***************************************************************************************
*                            SIGNAL GENERATING TEMPORARY                                *
****************************************************************************************/

void synthesis_arb(CFloatBase64Signal *signal, const float *data, uint32_t _size, float freq, float amp, float off, float showOff,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;
    float rate = (float)_size / (float)DAC_BUFFER_SIZE ;
    int period = (int) sigSize * 1000 / (freq * tscale * 10) * rate;
    if (period == 0) period = 1;
    for (size_t i = 0; i < sigSize; ++i)
	{
        auto x = i % period;

        auto t =  (float)x / (float)period;
        int z = ((int)(t * _size) % _size);
		(*signal)[i] = data[z] * amp + off + showOff;
	}
}

void synthesis_sin(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;

	// float tscale = atof(inTimeScale.Value().c_str());
    for(int i = 0; i < sigSize; i++) {
        (*signal)[i] = (float) (sin(2 * M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase) * amp + off + showOff);
    }
}


void synthesis_sin_burst(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;

	// float tscale = atof(inTimeScale.Value().c_str());
    float burstPointCount  = (CH_SIGNAL_SIZE_DEFAULT * burstCount) /  (freq * tscale / 1000 * 10);
    float burstPerCount  = (CH_SIGNAL_SIZE_DEFAULT * period) /  (tscale * 1000 * 10);
    float point_time = (tscale * 10) / (float)CH_SIGNAL_SIZE_DEFAULT; // millisecons;
    if (burstPointCount >= CH_SIGNAL_SIZE_DEFAULT){
        for(int i = 0; i < sigSize; i++) {
            (*signal)[i] =  (float) (sin(2 * M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase) * amp + off + showOff);
        }
    }
    else{
        int x = 0;
        int rep = 0;
        while(x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize){
            if (rep < reps){
                for(int i = 0 ; i < burstPointCount && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize ; i++,x++){
                    (*signal)[x] = (float) (sin(2 * M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase) * amp + off + showOff);
                }
                for(int i = 0 ; i < (burstPerCount - burstPointCount) && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    (*signal)[x] = (float) (off + showOff);
                }
                rep++;
            }else{
                for(; x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; x++){
                    (*signal)[x] = (float) (off + showOff);
                }
            }
        }
    }
}


void synthesis_triangle(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;

	// float tscale = atof(inTimeScale.Value().c_str());
    for(int i = 0; i < sigSize; i++) {
        (*signal)[i] = (float) ((asin(sin(2 * M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase))) / M_PI * 2  * amp + off + showOff);
    }
}

void synthesis_triangle_burst(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;

	// float tscale = atof(inTimeScale.Value().c_str());
    float burstPointCount  = (CH_SIGNAL_SIZE_DEFAULT * burstCount) /  (freq * tscale / 1000 * 10);
    float burstPerCount  = (CH_SIGNAL_SIZE_DEFAULT * period) /  (tscale * 1000 * 10);
    float point_time = (tscale * 10) / (float)CH_SIGNAL_SIZE_DEFAULT; // millisecons;
    if (burstPointCount >= CH_SIGNAL_SIZE_DEFAULT){
        for(int i = 0; i < sigSize; i++) {
            (*signal)[i] = (float) ((asin(sin(2 * M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase))) / M_PI * 2 * amp + off + showOff);
        }
    }
    else{
        int x = 0;
        int rep = 0;
        while(x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize){
            if (rep < reps){
                for(int i = 0 ; i < burstPointCount && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    (*signal)[x] = (float) ((asin(sin(2 * M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase))) / M_PI * 2 * amp + off + showOff);
                }
                for(int i = 0 ; i < (burstPerCount - burstPointCount) && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    (*signal)[x] = (float) (off + showOff);
                }
                rep++;
            }else{
                for(; x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; x++){
                    (*signal)[x] = (float) (off + showOff);
                }
            }
        }
    }
}

void synthesis_square(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff, float tscale,
                      float riseTime, float fallTime) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;
    int period = (int) sigSize * 1000 / (freq * tscale * 10);
    int riseCount = riseTime *  sigSize / (tscale * 10 * 1000);
    if (riseCount == 0) riseCount = 1;
    int fallCount = fallTime * sigSize / (tscale * 10 * 1000);
    if (fallCount == 0) fallCount = 1;
    if (period == 0) period = 1;

    int phaseCount = phase * period / (2 * M_PI) + period; // + period so that there is no error with a negative phase value.
    int t = 0;
    for (int i = 0; i < sigSize; i++) {
        auto x = (i + phaseCount) % period;
        float z = 0;
        if ( x < riseCount / 2) {
            z = (float) x / ((float) riseCount / 2.0f);
        } else if (x < (period - fallCount) / 2) {
            z = 1.0f;
        } else if (x < (period + fallCount) / 2) {
            if (fallCount > 1){
                z = 1.0f - 2.0f * (float) ((float)x - ((float)period - (float)fallCount) / 2.0f) / ((float) fallCount);
                if (z > 1.f)
                    z = 1.f;
            }
            else
                z = 0;
        }
        else if (x < period - (riseCount / 2)) {
            z = - 1.0f;
        }
        else {
            z = - 1.0f + (float) (x - (period - (riseCount / 2.0f))) / ((float) riseCount / 2.0f);
        }
        (*signal)[i] = off + showOff + amp * z;
    }
}

void synthesis_square_burst(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps, float tscale,
                            float riseTime, float fallTime) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;
	// float tscale = atof(inTimeScale.Value().c_str());
    float burstPointCount  = (CH_SIGNAL_SIZE_DEFAULT * burstCount) /  (freq * tscale / 1000 * 10);
    float burstPerCount  = (CH_SIGNAL_SIZE_DEFAULT * period) / (tscale * 1000 * 10);
    float point_time = (tscale * 10) / (float)CH_SIGNAL_SIZE_DEFAULT; // millisecons;
    if (burstPointCount >= CH_SIGNAL_SIZE_DEFAULT){
        synthesis_square(signal, freq, phase, amp, off, showOff, tscale, riseTime, fallTime);
    }else{
        int x = 0;
        int rep = 0;
        int riseCount = riseTime *  sigSize / (tscale * 10 * 1000);
        if (riseCount == 0) riseCount = 1;
        int fallCount = fallTime * sigSize / (tscale * 10 * 1000);
        if (fallCount == 0) fallCount = 1;
        int periodCount = (int) sigSize * 1000 / (freq * tscale * 10);
        if (periodCount == 0) periodCount = 1;
        int phaseCount = phase * periodCount / (2 * M_PI) + periodCount; // + periodCount so that there is no error with a negative phase value.
        while(x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize){
            if (rep < reps){
                int t = 0;
                for(int i = 0 ; i < periodCount * burstCount && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    t = (i + phaseCount) % periodCount;

                    float z = 0;
                    if ( t < riseCount / 2) { // Rise signal to 1 value
                        z = (float) t / ((float) riseCount / 2.0f);
                    } else if (t < (periodCount - fallCount) / 2) {
                        z = 1.0f;
                    } else if (t < (periodCount + fallCount) / 2) {
                        if (fallCount > 1){
                            z = 1.0f - 2.0f * (float) ((float)t - ((float)periodCount - (float)fallCount) / 2.0f) / ((float) fallCount);
                            if (z > 1.f)
                                z = 1.f;
                        }
                        else
                            z = 0;
                    }
                    else if (t < periodCount - (riseCount / 2)) {
                        z = - 1.0f;
                    }
                    else {
                        z = - 1.0f + (float) (t - (periodCount - (riseCount / 2.0f))) / ((float) riseCount / 2.0f);
                    }
                    (*signal)[x] = off + showOff + amp * z;
                }
                for(int i = 0 ; i < (burstPerCount - burstPointCount) && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    (*signal)[x] = (float) (off + showOff);
                }
                rep++;
            }else{
                for(; x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; x++){
                    (*signal)[x] = (float) (off + showOff);
                }
            }
        }
    }
}

int synthesis_rampUp(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return RP_OK;
	// float tscale = atof(inTimeScale.Value().c_str());
    float shift = 0;
    for(int unsigned i = 0; i < sigSize; i++) {
        float angle = M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase - shift;
        if(angle > M_PI) {
            angle -= M_PI;
            shift += M_PI;
        }
        (*signal)[sigSize - i - 1] = (float) (-1.0 * (acos(cos(angle)) / M_PI - 1)) * amp + off + showOff;
    }
    return RP_OK;
}

void synthesis_rampUp_burst(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;
	// float tscale = atof(inTimeScale.Value().c_str());
    float burstPointCount  = (CH_SIGNAL_SIZE_DEFAULT * burstCount) /  (freq * tscale / 1000 * 10);
    float burstPerCount  = (CH_SIGNAL_SIZE_DEFAULT * period) /  (tscale * 1000 * 10);
    float point_time = (tscale * 10) / (float)CH_SIGNAL_SIZE_DEFAULT; // millisecons;
    if (burstPointCount >= CH_SIGNAL_SIZE_DEFAULT){
        float shift = 0;
        for(int i = 0; i < sigSize; i++) {
            float angle = M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase - shift;
            if(angle > M_PI) {
                angle -= M_PI;
                shift += M_PI;
            }
            (*signal)[sigSize - i - 1] = (float) (-1.0 * (acos(cos(angle)) / M_PI - 1)) * amp + off + showOff;
        }
    }
    else{
        int x = 0;
        int rep = 0;
        while(x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize){
            if (rep < reps){
                float shift = 0;
                for(int i = 0 ; i < burstPointCount && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    float angle = M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase - shift;
                    if(angle > M_PI) {
                        angle -= M_PI;
                        shift += M_PI;
                    }
                    (*signal)[x] = amp - (float) (-1.0 * (acos(cos(angle)) / M_PI - 1)) * amp + off + showOff;
                }
                for(int i = 0 ; i < (burstPerCount - burstPointCount) && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    (*signal)[x] = (float) (off + showOff);
                }
                rep++;
            }else{
                for(; x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; x++){
                    (*signal)[x] = (float) (off + showOff);
                }
            }
        }
    }
}

int synthesis_rampDown(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return RP_OK;
	// float tscale = atof(inTimeScale.Value().c_str());
    float shift = 0;
    for(int unsigned i = 0; i < sigSize; i++) {
        float angle = M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase - shift;
        if(angle > M_PI) {
            angle -= M_PI;
            shift += M_PI;
        }
        (*signal)[i] = (float) (-1.0 * (acos(cos(angle)) / M_PI - 1)) * amp + off + showOff;
    }
    return RP_OK;
}

void synthesis_rampDown_burst(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;
	// float tscale = atof(inTimeScale.Value().c_str());
    float burstPointCount  = (CH_SIGNAL_SIZE_DEFAULT * burstCount) /  (freq * tscale / 1000 * 10);
    float burstPerCount  = (CH_SIGNAL_SIZE_DEFAULT * period) /  (tscale * 1000 * 10);
    float point_time = (tscale * 10) / (float)CH_SIGNAL_SIZE_DEFAULT; // millisecons;
    if (burstPointCount >= CH_SIGNAL_SIZE_DEFAULT){
        float shift = 0;
        for(int i = 0; i < sigSize; i++) {
            float angle = M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase - shift;
            if(angle > M_PI) {
                angle -= M_PI;
                shift += M_PI;
            }
            (*signal)[sigSize - i - 1] = (float) (-1.0 * (acos(cos(angle)) / M_PI - 1)) * amp + off + showOff;
        }
    }
    else{
        int x = 0;
        int rep = 0;
        while(x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize){
            if (rep < reps){
                float shift = 0;
                for(int i = 0 ; i < burstPointCount && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    float angle = M_PI * (float) i / (float) sigSize * (freq * tscale/1000) * 10 + phase - shift;
                    if(angle > M_PI) {
                        angle -= M_PI;
                        shift += M_PI;
                    }
                    (*signal)[x] = (float) (-1.0 * (acos(cos(angle)) / M_PI - 1)) * amp + off + showOff;
                }
                for(int i = 0 ; i < (burstPerCount - burstPointCount) && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    (*signal)[x] = (float) (off + showOff);
                }
                rep++;
            }else{
                for(; x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; x++){
                    (*signal)[x] = (float) (off + showOff);
                }
            }
        }
    }
}

int synthesis_DC(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return RP_OK;
    for(int i = 0; i < signal->GetSize(); i++) {
        (*signal)[i] = 1.0*amp + off + showOff;
    }
    return RP_OK;
}

void synthesis_DC_burst(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;
	// float tscale = atof(inTimeScale.Value().c_str());
    float burstPointCount  = (CH_SIGNAL_SIZE_DEFAULT * burstCount) /  (freq * tscale / 1000 * 10);
    float burstPerCount  = (CH_SIGNAL_SIZE_DEFAULT * period) /  (tscale * 1000 * 10);
    float point_time = (tscale * 10) / (float)CH_SIGNAL_SIZE_DEFAULT; // millisecons;
    if (burstPointCount >= CH_SIGNAL_SIZE_DEFAULT){
        for(int i = 0; i < sigSize; i++) {
            (*signal)[i] = 1.0*amp + off + showOff;
        }
    }
    else{
        int x = 0;
        int rep = 0;
        while(x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize){
            if (rep < reps){
                for(int i = 0 ; i < burstPointCount && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    (*signal)[x] = 1.0*amp + off + showOff;
                }
                for(int i = 0 ; i < (burstPerCount - burstPointCount) && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    (*signal)[x] = (float) (off + showOff);
                }
                rep++;
            }else{
                for(; x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; x++){
                    (*signal)[x] = (float) (off + showOff);
                }
            }
        }
    }
}

int synthesis_DC_NEG(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return RP_OK;
    for(int i = 0; i < signal->GetSize(); i++) {
        (*signal)[i] = -1.0*amp + off + showOff;
    }
    return RP_OK;
}

void synthesis_DC_NEG_burst(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff,int burstCount, int period, int reps,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;
	// float tscale = atof(inTimeScale.Value().c_str());
    float burstPointCount  = (CH_SIGNAL_SIZE_DEFAULT * burstCount) /  (freq * tscale / 1000 * 10);
    float burstPerCount  = (CH_SIGNAL_SIZE_DEFAULT * period) /  (tscale * 1000 * 10);
    float point_time = (tscale * 10) / (float)CH_SIGNAL_SIZE_DEFAULT; // millisecons;
    if (burstPointCount >= CH_SIGNAL_SIZE_DEFAULT){
        for(int i = 0; i < sigSize; i++) {
            (*signal)[i] = -1.0*amp + off + showOff;
        }
    }
    else{
        int x = 0;
        int rep = 0;
        while(x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize){
            if (rep < reps){
                for(int i = 0 ; i < burstPointCount && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    (*signal)[x] = -1.0*amp + off + showOff;
                }
                for(int i = 0 ; i < (burstPerCount - burstPointCount) && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    (*signal)[x] = (float) (off + showOff);
                }
                rep++;
            }else{
                for(; x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; x++){
                    (*signal)[x] = (float) (off + showOff);
                }
            }
        }
    }
}

int synthesis_PWM(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff, float ratio /*duty cycle*/,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return RP_OK;
	// float tscale = atof(inTimeScale.Value().c_str());
    float period = (float) sigSize / (freq * tscale * 10.f / 1000.f);
    float duty = period * ratio;
    float fphase = period * phase / (2.f * M_PI);

    float shift = 0;
    for(int i = 0; i < sigSize; i++) {
        float value = (float)i + fphase - shift;
        if(value > period) {
            value -= period;
            shift += period;
        }
        (*signal)[i] = (value > duty) ? (-amp + off + showOff) : (amp + off + showOff);
    }
    return RP_OK;
}

void synthesis_PWM_burst(CFloatBase64Signal *signal, float freq, float phase, float amp, float off, float showOff, float ratio ,int burstCount, int burst_period, int reps,float tscale) {
    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return;
	// float tscale = atof(inTimeScale.Value().c_str());
    float burstPointCount  = (CH_SIGNAL_SIZE_DEFAULT * burstCount) /  (freq * tscale / 1000 * 10);
    float burstPerCount  = (CH_SIGNAL_SIZE_DEFAULT * burst_period) /  (tscale * 1000 * 10);
    float point_time = (tscale * 10) / (float)CH_SIGNAL_SIZE_DEFAULT; // millisecons;
    float period = (float) sigSize / (freq * tscale * 10.f / 1000.f);
    float duty = period * ratio;
    float fphase = period * phase / (2.f * M_PI);

    if (burstPointCount >= CH_SIGNAL_SIZE_DEFAULT){
         float shift = 0;
        for(int i = 0; i < sigSize; i++) {
            float value = (float)i + fphase - shift;
            if(value > period) {
                value -= period;
                shift += period;
            }
            (*signal)[i] = (value > duty) ? (-amp + off + showOff) : (amp + off + showOff);
        }
    }
    else{
        int x = 0;
        int rep = 0;
        while(x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize){
            if (rep < reps){
                float shift = 0;
                for(int i = 0 ; i < burstPointCount && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    float value = (float)i + fphase - shift;
                    if(value > period) {
                        value -= period;
                        shift += period;
                    }
                    (*signal)[x] = (value > duty) ? (-amp + off + showOff) : (amp + off + showOff);
                }
                for(int i = 0 ; i < (burstPerCount - burstPointCount) && x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; i++,x++){
                    (*signal)[x] = (float) (off + showOff);
                }
                rep++;
            }else{
                for(; x < CH_SIGNAL_SIZE_DEFAULT && x < sigSize; x++){
                    (*signal)[x] = (float) (off + showOff);
                }
            }
        }
    }
}

int synthesis_sweep(CFloatBase64Signal *signal,float frequency,float frequency_start,float frequency_end,rp_gen_sweep_mode_t mode,rp_gen_sweep_dir_t dir
                    ,float phase, float amp, float off, float showOff,float tscale) {

    auto sigSize = (*signal).GetSize();
    if (sigSize == 0) return RP_OK;
    bool inverDir = false;
    if (frequency_end < frequency_start){
        inverDir = true;
    }
    // double tscale = (double)atof(inTimeScale.Value().c_str());
    float period = (float) sigSize / (frequency * tscale * 10.f / 1000.f);
    for(auto i = 0; i < sigSize; i++) {
        float sign = 1;
        double x = ((double)i- ((float) sigSize/2.0)) / period;
        x = x - floor(x);
        if (dir == RP_GEN_SWEEP_DIR_UP_DOWN){
            x = x * 2;
            if (x > 1) {
                x = 2 - x;
                sign = -1;
            }
        }
        double freq = 0;
        if (mode == RP_GEN_SWEEP_MODE_LINEAR){
            freq = ((frequency_end - frequency_start)*x + frequency_start) * 2;
        }
        if (mode == RP_GEN_SWEEP_MODE_LOG){
            freq = frequency_start  * exp(x *log(frequency_end/frequency_start));
        }
        if (inverDir) x = 1 - x;
        (*signal)[i] = sin(freq * 2 * M_PI * (x)/frequency + phase) * sign * amp + off + showOff;
    }
    return RP_OK;
}

auto synthesis_noise(CFloatBase64Signal *signal, float amp, float off, float showOff) -> void{
    auto sigSize = (*signal).GetSize();
    for(int i = 0; i < signal->GetSize(); i++) {
        auto r = ((double) rand() / (RAND_MAX));
        (*signal)[i] = r * amp + off + showOff;
    }
}