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

#ifdef __cplusplus
extern "C" {
#endif

float __attribute__((optimize("O0"))) log10f_neon(float x);
float __attribute__((optimize("O0"))) sinf_neon(float x);
float __attribute__((optimize("O0"))) cosf_neon(float x);
float __attribute__((optimize("O0"))) sqrtf_neon(float x);

#ifdef __cplusplus
}
#endif

#endif
