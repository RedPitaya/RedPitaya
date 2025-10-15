/**
 * $Id$
 *
 * @brief Red Pitaya Spectrum Analyzer DSC processing.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_MATH_H__
#define __RP_MATH_H__

#include <cstdint>
#include <cstring>

float log10f_neon(float x);
float sqrtf_neon(float x);

void memcpy_neon(volatile void* dst, volatile const void* src, size_t n);
void memcpy_stride_8bit_neon(volatile void* dst, volatile const void* src, size_t n);

void add_arrays_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n);
void subtract_arrays_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n);
void multiply_arrays_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n);
void divide_arrays_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n);
void divide_arrays_neon_Ex(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n, float limit);

void add_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n);
void subtract_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n);
void multiply_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n);
void divide_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n);

void add_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n);
void subtract_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n);
void multiply_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n);

void add_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n);
void subtract_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n);
void multiply_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n);

void add_scalar_to_array_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n);
void subtract_scalar_from_array_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n);
void multiply_array_by_scalar_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n);
void divide_array_by_scalar_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n);

void add_scalar_to_array_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n);
void subtract_scalar_from_array_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n);
void multiply_array_by_scalar_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n);
void divide_array_by_scalar_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n);

#endif
