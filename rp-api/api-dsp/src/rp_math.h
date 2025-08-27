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

float log10f_neon(float x);
float sqrtf_neon(float x);

void memcpy_neon(volatile void* dst, volatile const void* src, size_t n);
void memcpy_stride_8bit_neon(volatile void* dst, volatile const void* src, size_t n);

#endif
