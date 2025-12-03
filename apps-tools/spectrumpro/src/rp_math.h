/**
 * $Id$
 *
 * @brief Red Pitaya Spectrum Analyzer DSC processing.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_MATH_H__
#define __RP_MATH_H__

#include <CustomParameters.h>
#include <math.h>
#include <vector>

float __attribute__((optimize("O0"))) log10f_neon(float x);
float __attribute__((optimize("O0"))) sinf_neon(float x);
float __attribute__((optimize("O0"))) cosf_neon(float x);
float __attribute__((optimize("O0"))) sqrtf_neon(float x);

// use in waterfall
void decimateDataByMax(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size);
void decimateData(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size, int log_mode, int* indexArray);
void decimateDataMax(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size, int log_mode, int* indexArray);
void decimateDataAvg(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size, int log_mode, int* indexArray);
void decimateDataFirstN(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size, int log_mode, int* indexArray);
void decimateDataMinMax(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size, int log_mode, int* indexArray);
void prepareIndexArray(std::vector<int>* data, int start, int stop, int view_size, int log_mode);
float koffInLogSpace(float start, float stop, float value);
float indexInLogSpace(int start, int stop, int value);

#endif
