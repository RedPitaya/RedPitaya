/**
 * $Id$
 *
 * @brief Red Pitaya Spectrum Analyzer DSP processing.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#ifndef __RP_MATH_H__
#define __RP_MATH_H__

#include <stdint.h>
#include <vector>

#define RP_A_OK                0
#define RP_A_SMALL_SIGNAL      1
#define RP_A_ERROR             2
#define RP_A_DATA_SIZE_ZERO    3

typedef enum{
    OM_DISABLED    = 0,
    OM_BSPLINE     = 1,
    OM_CATMULLROM  = 2,
    OM_LANCZOS     = 3
} oversampling_mode_t;

typedef enum{
    FIR_3 = 3,
    FIR_5 = 5,
    FIR_7 = 7,
    FIR_9 = 9,
    FIR_11= 11
} fir_core_t;

int mean(const std::vector<float> &data, float *_value);
int mean(const std::vector<double> &data, float *_value);
int rms(const std::vector<float> &data, float *_value);
int rms(const std::vector<double> &data, float *_value);
int p2p(const std::vector<float> &data, float *_value);
int p2p(const std::vector<double> &data, float *_value);

int analysisTrap(const std::vector<float> &ch1, const std::vector<float> &ch2,
                uint32_t signalFreq, uint32_t decimation, uint32_t adcRate, float input_threshold,
                float *phaseCh1, float *phaseCh2, float *gain, float *phaseDiff);

int analysisTrap(const std::vector<double> &ch1, const std::vector<double> &ch2,
                uint32_t signalFreq, uint32_t decimation, uint32_t adcRate, float input_threshold,
                double *phaseCh1, double *phaseCh2, double *gain, double *phaseDiff);

int analysisTrap(const float *ch1, const float *ch2,
                size_t ch1_size, size_t ch2_size,
                uint32_t signalFreq, uint32_t decimation, uint32_t adcRate, float input_threshold,
                double *phaseCh1, double *phaseCh2, double *gain, double *phaseDiff);

int analysisTrap(const double *ch1, const double *ch2,
                size_t ch1_size, size_t ch2_size,
                uint32_t signalFreq, uint32_t decimation, uint32_t adcRate, float input_threshold,
                double *phaseCh1, double *phaseCh2, double *gain, double *phaseDiff);

int fir(float *data,size_t size,fir_core_t mode);
int fir(double *data,size_t size,fir_core_t mode);
int fir(std::vector<float> &data,fir_core_t mode);
int fir(std::vector<double> &data,fir_core_t mode);

bool oversampling(const float *_inData,size_t _inSize,float *_outData,size_t _outSize, oversampling_mode_t _mode);


#endif