/**
 * $Id: $
 *
 * @brief Red Pitaya library
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef CONVERT_NEON_HPP_
#define CONVERT_NEON_HPP_

#include <math.h>
#include <stdint.h>
#include <cstdlib>
#include "common/rp_log.h"
void cmn_convertToCnt_neon(volatile uint32_t* dst, volatile const float* src, size_t n, uint8_t bits, float fullScale, bool is_signed, double gain, int32_t offset) {
#ifdef ARCH_ARM
    if (n < 4) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            uint32_t mask = ((uint64_t)1 << bits) - 1;
            float voltage = src[i];

            if (gain == 0 || fullScale == 0) {
                dst[i] = 0;
                continue;
            }

            voltage /= gain;

            if (voltage > fullScale)
                voltage = fullScale;
            else if (voltage < -fullScale)
                voltage = -fullScale;

            if (!is_signed && voltage < 0) {
                voltage = 0;
            }

            int32_t cnts = (int)round(voltage * (float)(1 << (bits - (is_signed ? 1 : 0))) / fullScale);
            cnts += offset;

            int32_t max_cnt = (1 << (bits - (is_signed ? 1 : 0))) - 1;
            int32_t min_cnt = is_signed ? -(1 << (bits - (is_signed ? 1 : 0))) : 0;

            if (cnts > max_cnt)
                cnts = max_cnt;
            else if (cnts < min_cnt)
                cnts = min_cnt;

            if (cnts < 0)
                cnts = cnts & mask;

            dst[i] = (uint32_t)cnts;
        }
        return;
    }

    size_t main_count = n & ~0x3;
    size_t remainder = n & 0x3;

    // Precompute constants
    uint32_t mask = ((uint64_t)1 << bits) - 1;
    float scale_factor = (float)(1 << (bits - (is_signed ? 1 : 0))) / fullScale;
    int32_t max_cnt = (1 << (bits - (is_signed ? 1 : 0))) - 1;
    int32_t min_cnt = is_signed ? -(1 << (bits - (is_signed ? 1 : 0))) : 0;
    float gain_reciprocal = 1.0f / (float)gain;
    float neg_fullScale = -fullScale;

    // Process main portion using NEON
    if (main_count > 0) {
        volatile uint32_t* dst_main = dst;
        volatile const float* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VDUP.32 q4, %[gain_reciprocal]\n"  // Duplicate gain_reciprocal
            "    VDUP.32 q5, %[scale_factor]\n"     // Duplicate scale_factor
            "    VDUP.32 q6, %[offset]\n"           // Duplicate offset
            "    VDUP.32 q7, %[max_cnt]\n"          // Duplicate max_cnt
            "    VDUP.32 q8, %[min_cnt]\n"          // Duplicate min_cnt
            "    VDUP.32 q9, %[mask]\n"             // Duplicate mask
            "    VDUP.32 q10, %[fullScale]\n"       // Duplicate fullScale
            "    VDUP.32 q11, %[neg_fullScale]\n"   // Duplicate neg_fullScale
            "    VMOV.F32 q12, #0.0\n"              // Load constant 0.0
            "NEONConvertToCntPLD%=:\n"
            "    PLD [%[src], #0xC0]\n"         // Preload data
            "    VLD1.32 {d0-d1}, [%[src]]!\n"  // Load 4 floats

            // Apply gain: voltage /= gain
            "    VMUL.F32 q0, q0, q4\n"  // voltage * (1/gain)

            // Limit voltage to [-fullScale, fullScale] range
            "    VMIN.F32 q0, q0, q10\n"  // min(voltage, fullScale)
            "    VMAX.F32 q0, q0, q11\n"  // max(voltage, -fullScale)

            // For unsigned mode: set negative voltages to 0
            "    VCGT.F32 q1, q0, q12\n"  // q1 = (voltage > 0) ? -1 : 0
            "    VBSL.F32 q1, q0, q12\n"  // Select voltage if > 0, else 0

            // Apply scale factor and convert to int
            "    VMUL.F32 q0, q1, q5\n"  // voltage * scale_factor
            "    VCVT.S32.F32 q1, q0\n"  // Convert to int32 with rounding

            // Apply offset
            "    VADD.S32 q1, q1, q6\n"  // cnts += offset

            // Limit cnts range
            "    VMIN.S32 q1, q1, q7\n"  // min(cnts, max_cnt)
            "    VMAX.S32 q1, q1, q8\n"  // max(cnts, min_cnt)

            // Apply mask for negative numbers
            "    VCGE.S32 q2, q1, q12\n"  // q2 = (cnts >= 0) ? -1 : 0
            "    VAND.S32 q3, q1, q9\n"   // q3 = cnts & mask
            "    VBSL.S32 q2, q1, q3\n"   // Select cnts if >=0, else (cnts & mask)

            // Store result
            "    VST1.32 {d4-d5}, [%[dst]]!\n"  // Store 4 uint32_t
            "    SUBS %[n], %[n], #4\n"         // Decrement counter
            "    BGT NEONConvertToCntPLD%=\n"   // Branch if more
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [gain_reciprocal] "r"(gain_reciprocal), [scale_factor] "r"(scale_factor), [offset] "r"(offset), [max_cnt] "r"(max_cnt), [min_cnt] "r"(min_cnt), [mask] "r"(mask),
              [fullScale] "r"(fullScale), [neg_fullScale] "r"(neg_fullScale)
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            uint32_t mask = ((uint64_t)1 << bits) - 1;
            float voltage = src[i];

            if (gain == 0 || fullScale == 0) {
                dst[i] = 0;
                continue;
            }

            voltage /= gain;

            if (voltage > fullScale)
                voltage = fullScale;
            else if (voltage < -fullScale)
                voltage = -fullScale;

            if (!is_signed && voltage < 0) {
                voltage = 0;
            }

            int32_t cnts = (int)round(voltage * (float)(1 << (bits - (is_signed ? 1 : 0))) / fullScale);
            cnts += offset;

            int32_t max_cnt = (1 << (bits - (is_signed ? 1 : 0))) - 1;
            int32_t min_cnt = is_signed ? -(1 << (bits - (is_signed ? 1 : 0))) : 0;

            if (cnts > max_cnt)
                cnts = max_cnt;
            else if (cnts < min_cnt)
                cnts = min_cnt;

            if (cnts < 0)
                cnts = cnts & mask;

            dst[i] = (uint32_t)cnts;
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        uint32_t mask = ((uint64_t)1 << bits) - 1;
        float voltage = src[i];

        if (gain == 0 || fullScale == 0) {
            dst[i] = 0;
            continue;
        }

        voltage /= gain;

        if (voltage > fullScale)
            voltage = fullScale;
        else if (voltage < -fullScale)
            voltage = -fullScale;

        if (!is_signed && voltage < 0) {
            voltage = 0;
        }

        int32_t cnts = (int)round(voltage * (float)(1 << (bits - (is_signed ? 1 : 0))) / fullScale);
        cnts += offset;

        int32_t max_cnt = (1 << (bits - (is_signed ? 1 : 0))) - 1;
        int32_t min_cnt = is_signed ? -(1 << (bits - (is_signed ? 1 : 0))) : 0;

        if (cnts > max_cnt)
            cnts = max_cnt;
        else if (cnts < min_cnt)
            cnts = min_cnt;

        if (cnts < 0)
            cnts = cnts & mask;

        dst[i] = (uint32_t)cnts;
    }
#endif  // ARCH_ARM
}
void cmn_CalibCntsSigned_neon(volatile int32_t* dst, volatile const uint32_t* src, size_t n, uint8_t bits, uint32_t gain, uint32_t base, int32_t offset) {
#ifdef ARCH_ARM
    if (n < 4) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            int32_t m;
            uint32_t cnts = src[i];

            /* check sign */
            if (cnts & (1 << (bits - 1))) {
                /* negative number */
                m = -1 * ((cnts ^ ((1 << bits) - 1)) + 1);
            } else {
                /* positive number */
                m = cnts;
            }

            /* adopt ADC count with calibrated DC offset */
            m -= offset;

            m = ((int32_t)gain * m) / (int32_t)base;

            dst[i] = m;
        }
        return;
    }

    size_t main_count = n & ~0x3;  // Round down to multiple of 4
    size_t remainder = n & 0x3;    // Remainder (0-3 elements)

    // Precompute constants
    uint32_t sign_mask = (1 << (bits - 1));
    uint32_t bit_mask = (1 << bits) - 1;

    // Process main portion using NEON
    if (main_count > 0) {
        volatile int32_t* dst_main = dst;
        volatile const uint32_t* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VDUP.32 q4, %[offset]\n"     // Load offset to all lanes
            "    VDUP.32 q5, %[gain]\n"       // Load gain to all lanes
            "    VDUP.32 q6, %[base]\n"       // Load base to all lanes
            "    VDUP.32 q7, %[sign_mask]\n"  // Load sign_mask to all lanes
            "    VDUP.32 q8, %[bit_mask]\n"   // Load bit_mask to all lanes
            "    VMOV.S32 q9, #1\n"           // Load constant 1 to all lanes
            "    VMOV.S32 q10, #-1\n"         // Load constant -1 to all lanes
            "NEONCalibPLD%=:\n"
            "    PLD [%[src], #0xC0]\n"         // Preload data cache for src
            "    VLD1.32 {d0-d1}, [%[src]]!\n"  // Load 4 uint32_t from src
            "    VMOV.S32 q1, q0\n"             // Copy to q1 for sign check

            // Check sign and convert to signed (two's complement)
            "    VAND.S32 q2, q1, q7\n"   // q2 = cnts & sign_mask
            "    VCGT.S32 q3, q2, #0\n"   // q3 = (cnts & sign_mask) > 0 ? -1 : 0
            "    VBIF.S32 q0, q10, q3\n"  // If negative, set q0 to -1, else keep original

            // For negative numbers: m = -1 * ((cnts ^ bit_mask) + 1)
            "    VEOR.S32 q2, q1, q8\n"   // q2 = cnts ^ bit_mask
            "    VADD.S32 q2, q2, q9\n"   // q2 = (cnts ^ bit_mask) + 1
            "    VMUL.S32 q2, q2, q10\n"  // q2 = -1 * ((cnts ^ bit_mask) + 1)

            // Combine positive and negative results
            "    VBSL.S32 q3, q2, q1\n"  // Select q2 if negative, q1 if positive

            // Apply offset: m -= offset
            "    VSUB.S32 q0, q3, q4\n"  // q0 = m - offset

            // Apply gain: m = (gain * m) / base using reciprocal multiplication
            "    VMUL.S32 q0, q0, q5\n"     // q0 = gain * m
            "    VCVT.F32.S32 q1, q0\n"     // Convert to float for division
            "    VCVT.F32.S32 q2, q6\n"     // Convert base to float
            "    VRECPE.F32 q3, q2\n"       // Reciprocal approximation
            "    VRECPS.F32 q11, q3, q2\n"  // Newton-Raphson refinement step
            "    VMUL.F32 q3, q3, q11\n"    // Improved reciprocal
            "    VMUL.F32 q1, q1, q3\n"     // Multiply by reciprocal (division)
            "    VCVT.S32.F32 q0, q1\n"     // Convert back to int32

            "    VST1.32 {d0-d1}, [%[dst]]!\n"  // Store result (4 int32_t)
            "    SUBS %[n], %[n], #4\n"         // Decrement counter by 4 elements
            "    BGT NEONCalibPLD%=\n"          // Branch if more elements to process
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [offset] "r"(offset), [gain] "r"(gain), [base] "r"(base), [sign_mask] "r"(sign_mask), [bit_mask] "r"(bit_mask)
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            int32_t m;
            uint32_t cnts = src[i];

            /* check sign */
            if (cnts & (1 << (bits - 1))) {
                /* negative number */
                m = -1 * ((cnts ^ ((1 << bits) - 1)) + 1);
            } else {
                /* positive number */
                m = cnts;
            }

            /* adopt ADC count with calibrated DC offset */
            m -= offset;

            m = ((int32_t)gain * m) / (int32_t)base;

            dst[i] = m;
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        int32_t m;
        uint32_t cnts = src[i];

        /* check sign */
        if (cnts & (1 << (bits - 1))) {
            /* negative number */
            m = -1 * ((cnts ^ ((1 << bits) - 1)) + 1);
        } else {
            /* positive number */
            m = cnts;
        }

        /* adopt ADC count with calibrated DC offset */
        m -= offset;

        m = ((int32_t)gain * m) / (int32_t)base;

        dst[i] = m;
    }
#endif  // ARCH_ARM
}
void cmn_CalibCntsUnsigned_neon(volatile uint32_t* dst, volatile const uint32_t* src, size_t n, uint8_t bits, uint32_t gain, uint32_t base, int32_t offset) {
#ifdef ARCH_ARM
    if (n < 4) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            int32_t m = src[i];

            /* adopt ADC count with calibrated DC offset */
            m -= offset;

            m = (gain * m) / base;

            dst[i] = m;
        }
        return;
    }

    size_t main_count = n & ~0x3;  // Round down to multiple of 4
    size_t remainder = n & 0x3;    // Remainder (0-3 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile uint32_t* dst_main = dst;
        volatile const uint32_t* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VDUP.32 q4, %[offset]\n"  // Load offset to all lanes
            "    VDUP.32 q5, %[gain]\n"    // Load gain to all lanes
            "    VDUP.32 q6, %[base]\n"    // Load base to all lanes
            "NEONCalibUnsignedPLD%=:\n"
            "    PLD [%[src], #0xC0]\n"         // Preload data cache for src
            "    VLD1.32 {d0-d1}, [%[src]]!\n"  // Load 4 uint32_t from src

            // Convert uint32 to int32 (no direct conversion, use bitwise copy)
            "    VSUB.S32 q0, q0, q4\n"  // m -= offset (works with uint32 as signed)

            // Apply gain: m = (gain * m) / base using reciprocal multiplication
            "    VMUL.S32 q0, q0, q5\n"    // q0 = gain * m
            "    VCVT.F32.S32 q1, q0\n"    // Convert to float for division
            "    VCVT.F32.S32 q2, q6\n"    // Convert base to float
            "    VRECPE.F32 q3, q2\n"      // Reciprocal approximation
            "    VRECPS.F32 q7, q3, q2\n"  // Newton-Raphson refinement step
            "    VMUL.F32 q3, q3, q7\n"    // Improved reciprocal
            "    VMUL.F32 q1, q1, q3\n"    // Multiply by reciprocal (division)
            "    VCVT.U32.F32 q0, q1\n"    // Convert back to uint32

            "    VST1.32 {d0-d1}, [%[dst]]!\n"  // Store result (4 uint32_t)
            "    SUBS %[n], %[n], #4\n"         // Decrement counter by 4 elements
            "    BGT NEONCalibUnsignedPLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [offset] "r"(offset), [gain] "r"(gain), [base] "r"(base)
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            int32_t m = src[i];

            /* adopt ADC count with calibrated DC offset */
            m -= offset;

            m = (gain * m) / base;

            dst[i] = m;
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        int32_t m = src[i];

        /* adopt ADC count with calibrated DC offset */
        m -= offset;

        m = (gain * m) / base;

        dst[i] = m;
    }
#endif  // ARCH_ARM
}
void cmn_convertToVoltSigned_neon(volatile float* dst, volatile const uint32_t* src, size_t n, uint8_t bits, float fullScale, uint32_t gain, uint32_t base, int32_t offset) {
#ifdef ARCH_ARM
    if (n < 4) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            int32_t calib_cnts;
            uint32_t cnts = src[i];

            /* check sign */
            if (cnts & (1 << (bits - 1))) {
                /* negative number */
                calib_cnts = -1 * ((cnts ^ ((1 << bits) - 1)) + 1);
            } else {
                /* positive number */
                calib_cnts = cnts;
            }

            /* adopt ADC count with calibrated DC offset */
            calib_cnts -= offset;

            calib_cnts = ((int32_t)gain * calib_cnts) / (int32_t)base;

            /* convert to voltage */
            dst[i] = ((float)calib_cnts * fullScale / (float)(1 << (bits - 1)));
        }
        return;
    }

    size_t main_count = n & ~0x3;  // Round down to multiple of 4
    size_t remainder = n & 0x3;    // Remainder (0-3 elements)

    // Precompute constants
    uint32_t sign_mask = (1 << (bits - 1));
    uint32_t bit_mask = (1 << bits) - 1;
    float scale_factor = fullScale / (float)(1 << (bits - 1));

    // Process main portion using NEON
    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const uint32_t* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VDUP.32 q4, %[offset]\n"         // Load offset to all lanes
            "    VDUP.32 q5, %[gain]\n"           // Load gain to all lanes
            "    VDUP.32 q6, %[base]\n"           // Load base to all lanes
            "    VDUP.32 q7, %[sign_mask]\n"      // Load sign_mask to all lanes
            "    VDUP.32 q8, %[bit_mask]\n"       // Load bit_mask to all lanes
            "    VMOV.S32 q9, #1\n"               // Load constant 1 to all lanes
            "    VMOV.S32 q10, #-1\n"             // Load constant -1 to all lanes
            "    VDUP.32 q11, %[scale_factor]\n"  // Duplicate scale_factor to all lanes
            "NEONConvertVoltPLD%=:\n"
            "    PLD [%[src], #0xC0]\n"         // Preload data cache for src
            "    VLD1.32 {d0-d1}, [%[src]]!\n"  // Load 4 uint32_t from src
            "    VMOV.S32 q1, q0\n"             // Copy to q1 for sign check

            // Check sign and convert to signed (two's complement)
            "    VAND.S32 q2, q1, q7\n"   // q2 = cnts & sign_mask
            "    VCGT.S32 q3, q2, #0\n"   // q3 = (cnts & sign_mask) > 0 ? -1 : 0
            "    VBIF.S32 q0, q10, q3\n"  // If negative, set q0 to -1, else keep original

            // For negative numbers: m = -1 * ((cnts ^ bit_mask) + 1)
            "    VEOR.S32 q2, q1, q8\n"   // q2 = cnts ^ bit_mask
            "    VADD.S32 q2, q2, q9\n"   // q2 = (cnts ^ bit_mask) + 1
            "    VMUL.S32 q2, q2, q10\n"  // q2 = -1 * ((cnts ^ bit_mask) + 1)

            // Combine positive and negative results
            "    VBSL.S32 q3, q2, q1\n"  // Select q2 if negative, q1 if positive

            // Apply offset: m -= offset
            "    VSUB.S32 q0, q3, q4\n"  // q0 = m - offset

            // Apply gain: m = (gain * m) / base using reciprocal multiplication
            "    VMUL.S32 q0, q0, q5\n"     // q0 = gain * m
            "    VCVT.F32.S32 q1, q0\n"     // Convert to float for division
            "    VCVT.F32.S32 q2, q6\n"     // Convert base to float
            "    VRECPE.F32 q3, q2\n"       // Reciprocal approximation
            "    VRECPS.F32 q12, q3, q2\n"  // Newton-Raphson refinement step
            "    VMUL.F32 q3, q3, q12\n"    // Improved reciprocal
            "    VMUL.F32 q1, q1, q3\n"     // Multiply by reciprocal (division)

            // Convert to voltage: ret_val = calib_cnts * scale_factor
            "    VMUL.F32 q0, q1, q11\n"  // q0 = calib_cnts * scale_factor

            "    VST1.32 {d0-d1}, [%[dst]]!\n"  // Store result (4 float)
            "    SUBS %[n], %[n], #4\n"         // Decrement counter by 4 elements
            "    BGT NEONConvertVoltPLD%=\n"    // Branch if more elements to process
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [offset] "r"(offset), [gain] "r"(gain), [base] "r"(base), [sign_mask] "r"(sign_mask), [bit_mask] "r"(bit_mask), [scale_factor] "r"(scale_factor)
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            int32_t calib_cnts;
            uint32_t cnts = src[i];

            /* check sign */
            if (cnts & (1 << (bits - 1))) {
                /* negative number */
                calib_cnts = -1 * ((cnts ^ ((1 << bits) - 1)) + 1);
            } else {
                /* positive number */
                calib_cnts = cnts;
            }

            /* adopt ADC count with calibrated DC offset */
            calib_cnts -= offset;

            calib_cnts = ((int32_t)gain * calib_cnts) / (int32_t)base;

            /* convert to voltage */
            dst[i] = ((float)calib_cnts * fullScale / (float)(1 << (bits - 1)));
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        int32_t calib_cnts;
        uint32_t cnts = src[i];

        /* check sign */
        if (cnts & (1 << (bits - 1))) {
            /* negative number */
            calib_cnts = -1 * ((cnts ^ ((1 << bits) - 1)) + 1);
        } else {
            /* positive number */
            calib_cnts = cnts;
        }

        /* adopt ADC count with calibrated DC offset */
        calib_cnts -= offset;

        calib_cnts = ((int32_t)gain * calib_cnts) / (int32_t)base;

        /* convert to voltage */
        dst[i] = ((float)calib_cnts * fullScale / (float)(1 << (bits - 1)));
    }
#endif  // ARCH_ARM
}

void cmn_convertToVoltUnsigned_neon(volatile float* dst, volatile const uint32_t* src, size_t n, uint8_t bits, float fullScale, uint32_t gain, uint32_t base, int32_t offset) {
#ifdef ARCH_ARM
    if (n < 4) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            uint32_t calib_cnts;
            uint32_t cnts = src[i];

            /* calibrate ADC counts */
            int32_t m = cnts;
            m -= offset;
            m = (gain * m) / base;
            calib_cnts = m;

            /* convert to voltage */
            dst[i] = ((float)calib_cnts * fullScale / (float)(1 << bits));
        }
        return;
    }

    size_t main_count = n & ~0x3;  // Round down to multiple of 4
    size_t remainder = n & 0x3;    // Remainder (0-3 elements)

    // Precompute constants
    float scale_factor = fullScale / (float)(1 << bits);

    // Process main portion using NEON
    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const uint32_t* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VDUP.32 q4, %[offset]\n"        // Load offset to all lanes
            "    VDUP.32 q5, %[gain]\n"          // Load gain to all lanes
            "    VDUP.32 q6, %[base]\n"          // Load base to all lanes
            "    VDUP.32 q7, %[scale_factor]\n"  // Duplicate scale_factor to all lanes
            "NEONConvertVoltUnsignedPLD%=:\n"
            "    PLD [%[src], #0xC0]\n"         // Preload data cache for src
            "    VLD1.32 {d0-d1}, [%[src]]!\n"  // Load 4 uint32_t from src

            // No conversion needed - work with uint32 directly for unsigned values
            "    VSUB.S32 q0, q0, q4\n"  // m -= offset (works with uint32)

            // Apply gain: m = (gain * m) / base using reciprocal multiplication
            "    VMUL.S32 q0, q0, q5\n"    // q0 = gain * m
            "    VCVT.F32.U32 q1, q0\n"    // Convert to float for division
            "    VCVT.F32.U32 q2, q6\n"    // Convert base to float
            "    VRECPE.F32 q3, q2\n"      // Reciprocal approximation
            "    VRECPS.F32 q8, q3, q2\n"  // Newton-Raphson refinement step
            "    VMUL.F32 q3, q3, q8\n"    // Improved reciprocal
            "    VMUL.F32 q1, q1, q3\n"    // Multiply by reciprocal (division)

            // Convert to voltage: ret_val = calib_cnts * scale_factor
            "    VMUL.F32 q0, q1, q7\n"  // q0 = calib_cnts * scale_factor

            "    VST1.32 {d0-d1}, [%[dst]]!\n"        // Store result (4 float)
            "    SUBS %[n], %[n], #4\n"               // Decrement counter by 4 elements
            "    BGT NEONConvertVoltUnsignedPLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [offset] "r"(offset), [gain] "r"(gain), [base] "r"(base), [scale_factor] "r"(scale_factor)
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            uint32_t calib_cnts;
            uint32_t cnts = src[i];

            /* calibrate ADC counts */
            int32_t m = cnts;
            m -= offset;
            m = (gain * m) / base;
            calib_cnts = m;

            /* convert to voltage */
            dst[i] = ((float)calib_cnts * fullScale / (float)(1 << bits));
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        uint32_t calib_cnts;
        uint32_t cnts = src[i];

        /* calibrate ADC counts */
        int32_t m = cnts;
        m -= offset;
        m = (gain * m) / base;
        calib_cnts = m;

        /* convert to voltage */
        dst[i] = ((float)calib_cnts * fullScale / (float)(1 << bits));
    }
#endif  // ARCH_ARM
}

#endif