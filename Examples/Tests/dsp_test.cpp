/* Red Pitaya C API example Acquiring a signal from a buffer
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "rp.h"
#include "math/rp_algorithms.h"

#define DATA_SIZE 20
#define OFFSET 10

std::vector<float> fillBuffer(uint32_t size, float amp,float phase, float periods, float noise, float offset){
    std::vector<float> f;
    for(uint32_t i = 0; i < size; i++){
        float z = amp * sin((float)i / ((float)size) * periods * 2 * M_PI + (phase / 180.0f * M_PI)) + offset;
        if (noise > 0) {
           z += 2 * noise * ((double)(std::rand() % 1000) / 1000.0 - 0.5);
        }
        f.push_back(z);
    }
    return f;
}

float calcFreq(double dec, double rate, double size, double periods) {
    return 1.0 / ((dec / rate) * (size / periods));
}

typedef struct {
    int size = 8;
    float chA[2];
    float chP[2];
    float chANoise[2] = {0,0};
    float offset[2] = {0,0};
    int periods = 2;
    float errorPhase = 0;
    float errorGain = 0;
    int decimate = 1;
    int maxRate = 125e6;
} settings_t;

typedef struct {
    float noise;
    float offset;
    uint64_t tcount;
    uint64_t terror;
} result_t;

int test(settings_t &s){
    bool ret = 0;
    auto b1 = fillBuffer(s.size,s.chA[0],s.chP[0],s.periods,s.chANoise[0],s.offset[0]);
    auto b2 = fillBuffer(s.size,s.chA[1],s.chP[1],s.periods,s.chANoise[1],s.offset[1]);
    auto freq = calcFreq(s.decimate,s.maxRate,s.size,s.periods);
    std::vector<float> b1f;
    std::vector<float> b2f;
    b1f.resize(s.size * 100);
    b2f.resize(s.size * 100);

    // fir(b1,FIR_11);
    // fir(b2,FIR_11);

    oversampling(b1.data(),b1.size(),b1f.data(),b1f.size(),OM_LANCZOS);
    oversampling(b2.data(),b2.size(),b2f.data(),b2f.size(),OM_LANCZOS);
    // firF(b1,&b1f);
    // firF(b2,&b2f);
    float p[2];
    float gain;
    float phaseDiff;

    analysisTrap(b1f,b2f,freq / 100 ,s.decimate,s.maxRate,0.001,&p[0],&p[1], &gain, &phaseDiff);
    printf("Freq %f Gen A[%f,%f] P[%f,%f] P[%f,%f] Gain %lf Phas Diff %lf",freq ,s.chA[0],s.chA[1],s.chP[0],s.chP[1],p[0],p[1],gain,phaseDiff);
    auto pdOr = s.chP[1] - s.chP[0];
    auto ampOr = s.chA[0] /s.chA[1];

    if ((pdOr + s.errorPhase >= phaseDiff) && (pdOr - s.errorPhase <= phaseDiff)){
        printf(" Phase [OK]");
    }else{
        printf(" Phase [ERROR]");
        ret = 1;
    }
    if ((ampOr * (1  + s.errorGain) >= gain) && (ampOr * (1  - s.errorGain) <= gain)){
        printf(" Gain [OK]\n");
    }else{
        printf(" Gain [ERROR]\n");
        ret = 1;
    }
    return ret;
}

int main(int argc, char **argv){
    std::srand(std::time(nullptr));

    std::vector<result_t> result;
    settings_t s;
    float offset_max = 0;
    for(float offset = -offset_max; offset <= offset_max; offset += 0.05){
        for(float noise = 0.0; noise <= 0.3; noise += 0.02){
            uint64_t error_count = 0;
            uint64_t test_count = 0;

            for(float a1 = 1; a1 <= 1; a1 *= 2){
                for(float a2 = 0.005; a2 <= 1.5; a2 *= 2){
                    for(float p2 = -90; p2 <= 90; p2 += 10){

                        s.chA[0] = a1;
                        s.chA[1] = a2;
                        s.chP[0] = 0;
                        s.chP[1] = p2;
                        s.offset[0] = offset;
                        s.offset[1] = offset;
                        s.errorPhase = 5; // 1 deg error
                        s.errorGain = 0.5;
                        s.chANoise[0] = noise * a1;
                        s.chANoise[1] = noise * a2;

                        if (test(s)) {
                            error_count++;
                        }
                        test_count++;
            // return 0;

                    }
                }
            }
            result_t r;
            r.noise = noise;
            r.tcount = test_count;
            r.terror = error_count;
            r.offset = offset;
            result.push_back(r);
        }
    }

    for(size_t i = 0 ; i < result.size(); i++){
        printf("Offset %f Noise %f Test count %lld Error count %lld - %d%%\n",result[i].offset, result[i].noise, result[i].tcount,result[i].terror , (int)(100 * (double)result[i].terror / (double)result[i].tcount));
    }
    return 0;
}
