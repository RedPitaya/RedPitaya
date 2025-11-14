#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"

#include "rp_math.h"
#include <math.h>
#include <cstdint>
#include <iostream>

const float __log10f_rng = 0.3010299957f;

const float __log10f_lut[8] = {
    -0.99697286229624,  //p0
    -1.07301643912502,  //p4
    -2.46980061535534,  //p2
    -0.07176870463131,  //p6
    2.247870219989470,  //p1
    0.366547581117400,  //p5
    1.991005185100089,  //p3
    0.006135635201050,  //p7
};

float log10f_neon(float x) {
#ifdef ARCH_ARM
    float dest;
    asm volatile(

        "vdup.f32		d0, d0[0]				\n\t"  //d0 = {x,x};

        //extract exponent
        "vmov.i32		d2, #127				\n\t"  //d2 = 127;
        "vshr.u32		d6, d0, #23				\n\t"  //d6 = d0 >> 23;
        "vsub.i32		d6, d6, d2				\n\t"  //d6 = d6 - d2;
        "vshl.u32		d1, d6, #23				\n\t"  //d1 = d6 << 23;
        "vsub.i32		d0, d0, d1				\n\t"  //d0 = d0 + d1;

        //polynomial:
        "vmul.f32 		d1, d0, d0				\n\t"  //d1 = d0*d0 = {x^2, x^2}
        "vld1.32 		{d2, d3, d4, d5}, [%2]	\n\t"  //q1 = {p0, p4, p2, p6}, q2 = {p1, p5, p3, p7} ;
        "vmla.f32 		q1, q2, d0[0]			\n\t"  //q1 = q1 + q2 * d0[0]
        "vmla.f32 		d2, d3, d1[0]			\n\t"  //d2 = d2 + d3 * d1[0]
        "vmul.f32 		d1, d1, d1				\n\t"  //d1 = d1 * d1 = {x^4, x^4}
        "vmla.f32 		d2, d1, d2[1]			\n\t"  //d2 = d2 + d1 * d2[1]

        //add exponent
        "vdup.32 		d7, %1					\n\t"  //d7 = {rng, rng}
        "vcvt.f32.s32 	d6, d6					\n\t"  //d6 = (float) d6
        "vmla.f32 		d2, d6, d7				\n\t"  //d2 = d2 + d6 * d7

        "vmov.f32 		%0, s4	        		\n\t"  //s0 = s4

        : "=r"(dest)
        : "r"(__log10f_rng), "r"(__log10f_lut)
        : "d0", "d1", "q1", "q2", "d6", "d7");
    return dest;
#else
    return log10f(x);
#endif
}

float sqrtf_neon(float x) {
#ifdef ARCH_ARM
    float dest;
    asm volatile(

        //fast invsqrt approx
        "vmov.f32 		d1, d0					\n\t"  //d1 = d0
        "vrsqrte.f32 	d0, d0					\n\t"  //d0 = ~ 1.0 / sqrt(d0)
        "vmul.f32 		d2, d0, d1				\n\t"  //d2 = d0 * d1
        "vrsqrts.f32 	d3, d2, d0				\n\t"  //d3 = (3 - d0 * d2) / 2
        "vmul.f32 		d0, d0, d3				\n\t"  //d0 = d0 * d3
        "vmul.f32 		d2, d0, d1				\n\t"  //d2 = d0 * d1
        "vrsqrts.f32 	d3, d2, d0				\n\t"  //d4 = (3 - d0 * d3) / 2
        "vmul.f32 		d0, d0, d3				\n\t"  //d0 = d0 * d3

        //fast reciporical approximation
        "vrecpe.f32		d1, d0					\n\t"          //d1 = ~ 1 / d0;
        "vrecps.f32		d2, d1, d0				\n\t"          //d2 = 2.0 - d1 * d0;
        "vmul.f32		d1, d1, d2				\n\t"          //d1 = d1 * d2;
        "vrecps.f32		d2, d1, d0				\n\t"          //d2 = 2.0 - d1 * d0;
        "vmul.f32		d0, d1, d2				\n\t"          //d0 = d1 * d2;
        "vmov.f32 		%0, s0	        		        \n\t"  //s0 = s4
        : "=r"(dest)::"d0", "d1", "d2", "d3");
    return dest;
#else
    return sqrtf(x);
#endif
}

void memcpy_neon(volatile void* dst, volatile const void* src, size_t n) {
#ifdef ARCH_ARM
    if (n < 32) {
        memcpy((void*)dst, (void*)src, n);
        return;
    }

    uintptr_t dst_align = (uintptr_t)dst & 0x7;
    uintptr_t src_align = (uintptr_t)src & 0x7;

    if (dst_align != src_align) {
        memcpy((void*)dst, (void*)src, n);
        return;
    }

    size_t align_bytes = (8 - src_align) & 0x7;
    if (align_bytes > 0 && n >= align_bytes) {
        memcpy((void*)dst, (void*)src, align_bytes);
        dst = (volatile void*)((uint8_t*)dst + align_bytes);
        src = (volatile const void*)((uint8_t*)src + align_bytes);
        n -= align_bytes;
    }

    size_t main_count = n & ~0x3F;  // Round down to multiple of 64
    size_t remainder = n & 0x3F;    // Remainder (0-63 bytes)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile void* dst_main = dst;
        volatile const void* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "NEONCopyPLD%=:\n"
            "    PLD [%[src], #128]\n"
            "    VLDM %[src]!, {d0-d7}\n"  // Load 64 bytes
            "    VSTM %[dst]!, {d0-d7}\n"  // Store 64 bytes
            "    SUBS %[n], %[n], #64\n"   // Decrement counter
            "    BGT NEONCopyPLD%=\n"      // Branch if more data
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder
    if (remainder > 0) {
        memcpy((void*)((uint8_t*)dst + main_count), (void*)((uint8_t*)src + main_count), remainder);
    }
#else
    memcpy((void*)dst, (void*)src, n);
#endif
}

void memcpy_stride_8bit_neon(volatile void* dst, volatile const void* src, size_t n) {
#ifdef ARCH_ARM
    if (n % 64) {
        memcpy((void*)dst, (void*)src, n);
        //std::cout << "Warning: Non-optimal neon copy\n";
        return;
    }

    asm volatile(
        "NEONCopyPLD_8bit%=:\n"
        "    PLD [%[src], #0xC0]\n"
        "    VLD2.8 {d0,d1},[%[src]]!\n"
        "    VLD2.8 {d2,d3},[%[src]]!\n"
        "    VLD2.8 {d4,d5},[%[src]]!\n"
        "    VLD2.8 {d6,d7},[%[src]]!\n"

        "    VMOV d2,d3              \n"
        "    VMOV d3,d5              \n"
        "    VMOV d4,d7              \n"
        "    VSTM %[dst]!,{d1-d4}\n"
        "    SUBS %[n],%[n],#0x20\n"
        "    BGT NEONCopyPLD_8bit%=\n"
        : [dst] "+r"(dst), [src] "+r"(src), [n] "+r"(n)
        :
        : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
#else
    memcpy((void*)dst, (void*)src, n);
#endif
}

void add_arrays_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src1_main = src1;
        volatile const float* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONAddPLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"  // Preload 192 bytes (48 floats)
            "    PLD [%[src2], #0xC0]\n"  // Preload 192 bytes (48 floats)

            // Load 16 floats from src1 (q0-q3 = d0-d7)
            "VLDM %[src1]!, {d0-d7}\n"  // 16 floats from src1 (q0,q1,q2,q3)

            // Load 16 floats from src2 (q4-q7 = d8-d15)
            "VLDM %[src2]!, {d8-d15}\n"  // 16 floats from src2 (q4,q5,q6,q7)

            // Add 16 floats
            "VADD.F32 q0, q0, q4\n"  // q0 = q0 + q4 (4 floats)
            "VADD.F32 q1, q1, q5\n"  // q1 = q1 + q5 (4 floats)
            "VADD.F32 q2, q2, q6\n"  // q2 = q2 + q6 (4 floats)
            "VADD.F32 q3, q3, q7\n"  // q3 = q3 + q7 (4 floats)

            // Store 16 results
            "VSTM %[dst]!, {d0-d7}\n"  // Store 16 floats (q0-q3)

            "SUBS %[n], %[n], #16\n"  // Decrement counter by 16 elements
            "BGT NEONAddPLD%=\n"      // Branch if more data

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] + src2[i];
    }
#endif  // ARCH_ARM
}

void subtract_arrays_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src1_main = src1;
        volatile const float* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONSubPLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 8 floats from src1 (d0-d3 = q0,q1)
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 8 floats from src2 (d4-d7 = q2,q3)
            "    VSUB.F32 q0, q0, q2\n"     // Subtract first 4 floats (q0 - q2)
            "    VSUB.F32 q1, q1, q3\n"     // Subtract next 4 floats (q1 - q3)
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (8 floats)
            "    SUBS %[n], %[n], #16\n"    // Decrement counter by 16 elements
            "    BGT NEONSubPLD%=\n"        // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] - src2[i];
    }
#endif  // ARCH_ARM
}

void multiply_arrays_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src1_main = src1;
        volatile const float* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONMulPLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 8 floats from src1 (d0-d3 = q0,q1)
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 8 floats from src2 (d4-d7 = q2,q3)
            "    VMUL.F32 q0, q0, q2\n"     // Multiply first 4 floats (q0 * q2)
            "    VMUL.F32 q1, q1, q3\n"     // Multiply next 4 floats (q1 * q3)
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (8 floats)
            "    SUBS %[n], %[n], #8\n"     // Decrement counter by 16 elements
            "    BGT NEONMulPLD%=\n"        // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] * src2[i];
    }
#endif  // ARCH_ARM
}

void divide_arrays_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] / src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src1_main = src1;
        volatile const float* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONDivPLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 8 floats from src1 (d0-d3 = q0,q1)
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 8 floats from src2 (d4-d7 = q2,q3)
            "    VRECPE.F32 q4, q2\n"       // Reciprocal approximation for first 4 floats
            "    VRECPE.F32 q5, q3\n"       // Reciprocal approximation for next 4 floats
            "    VRECPS.F32 q6, q4, q2\n"   // Newton-Raphson step 1 for first 4
            "    VRECPS.F32 q7, q5, q3\n"   // Newton-Raphson step 1 for next 4
            "    VMUL.F32 q4, q4, q6\n"     // Newton-Raphson step 2 for first 4
            "    VMUL.F32 q5, q5, q7\n"     // Newton-Raphson step 2 for next 4
            "    VRECPS.F32 q6, q4, q2\n"   // Newton-Raphson step 3 for first 4
            "    VRECPS.F32 q7, q5, q3\n"   // Newton-Raphson step 3 for next 4
            "    VMUL.F32 q4, q4, q6\n"     // Newton-Raphson step 4 for first 4
            "    VMUL.F32 q5, q5, q7\n"     // Newton-Raphson step 4 for next 4
            "    VMUL.F32 q0, q0, q4\n"     // Multiply src1 by reciprocal (q0 * 1/q2)
            "    VMUL.F32 q1, q1, q5\n"     // Multiply src1 by reciprocal (q1 * 1/q3)
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (8 floats)
            "    SUBS %[n], %[n], #16\n"    // Decrement counter by 16 elements
            "    BGT NEONDivPLD%=\n"        // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] / src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] / src2[i];
    }
#endif  // ARCH_ARM
}

void divide_arrays_neon_Ex(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n, float limit) {
#ifdef ARCH_ARM
    if (n < 8) {
        for (size_t i = 0; i < n; ++i) {
            float _v1 = src1[i];
            float _v2 = src2[i];
            float ret;
            if (_v2 != 0)
                ret = _v1 / _v2;
            else
                ret = _v1 > 0 ? limit : -limit;
            dst[i] = ret;
        }
        return;
    }

    size_t main_count = n & ~0x7;
    size_t remainder = n & 0x7;

    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src1_main = src1;
        volatile const float* src2_main = src2;
        size_t n_main = main_count;

        // Prepare constants for NEON
        float max_val = limit;
        float min_val = -limit;
        float zero_val = 0.0f;

        asm volatile(
            "    VDUP.32 q8, %[max_val]\n"    // q8 = max_val (replicated)
            "    VDUP.32 q9, %[min_val]\n"    // q9 = min_val (replicated)
            "    VDUP.32 q10, %[zero_val]\n"  // q10 = 0.0f (replicated)

            "NEONDivPLD%=:\n"
            "    PLD [%[src1], #0x80]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0x80]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 8 floats from src1 (q0,q1)
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 8 floats from src2 (q2,q3)

            // Check for division by zero
            "    VCEQ.F32 q11, q2, q10\n"  // q11 = mask (q2 == 0)
            "    VCEQ.F32 q12, q3, q10\n"  // q12 = mask (q3 == 0)

            // Calculate reciprocal with Newton-Raphson
            "    VRECPE.F32 q4, q2\n"      // Reciprocal approximation for first 4 floats
            "    VRECPE.F32 q5, q3\n"      // Reciprocal approximation for next 4 floats
            "    VRECPS.F32 q6, q4, q2\n"  // Newton-Raphson step 1 for first 4
            "    VRECPS.F32 q7, q5, q3\n"  // Newton-Raphson step 1 for next 4
            "    VMUL.F32 q4, q4, q6\n"    // Newton-Raphson step 2 for first 4
            "    VMUL.F32 q5, q5, q7\n"    // Newton-Raphson step 2 for next 4

            // Multiply src1 by reciprocal (division)
            "    VMUL.F32 q0, q0, q4\n"  // q0 = q0 * 1/q2
            "    VMUL.F32 q1, q1, q5\n"  // q1 = q1 * 1/q3

            // Handle division by zero cases
            "    VCGT.F32 q6, q0, q10\n"  // q6 = mask (q0 > 0)
            "    VCGT.F32 q7, q1, q10\n"  // q7 = mask (q1 > 0)
            "    VBSL q11, q8, q9\n"      // q11 = if zero_mask then max_val else min_val
            "    VBSL q12, q8, q9\n"      // q12 = if zero_mask then max_val else min_val
            "    VBSL q11, q11, q0\n"     // q0 = if zero_mask then max/min_val else division result
            "    VBSL q12, q12, q1\n"     // q1 = if zero_mask then max/min_val else division result

            // Store result
            "    VSTM %[dst]!, {d0-d3}\n"  // Store 8 floats (q0,q1)
            "    SUBS %[n], %[n], #8\n"    // Decrement counter by 8 elements
            "    BGT NEONDivPLD%=\n"       // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            : [max_val] "r"(max_val), [min_val] "r"(min_val), [zero_val] "r"(zero_val)
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d25", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7",
              "q8", "q9", "q10", "q11", "q12", "cc", "memory");
    }

    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            float _v1 = src1[i];
            float _v2 = src2[i];
            float ret;
            if (_v2 != 0)
                ret = _v1 / _v2;
            else
                ret = _v1 > 0 ? limit : -limit;
            dst[i] = ret;
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        float _v1 = src1[i];
        float _v2 = src2[i];
        float ret;
        if (_v2 != 0)
            ret = _v1 / _v2;
        else
            ret = _v1 > 0 ? limit : -limit;
        dst[i] = ret;
    }
#endif
}

void add_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile int* dst_main = dst;
        volatile const int* src1_main = src1;
        volatile const int* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONAddIntPLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 8 integers from src1 (d0-d3 = q0,q1)
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 8 integers from src2 (d4-d7 = q2,q3)
            "    VADD.I32 q0, q0, q2\n"     // Add first 4 integers (q0 + q2)
            "    VADD.I32 q1, q1, q3\n"     // Add next 4 integers (q1 + q3)
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (8 integers)
            "    SUBS %[n], %[n], #16\n"    // Decrement counter by 16 elements
            "    BGT NEONAddIntPLD%=\n"     // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] + src2[i];
    }
#endif  // ARCH_ARM
}

void subtract_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile int* dst_main = dst;
        volatile const int* src1_main = src1;
        volatile const int* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONSubIntPLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 8 integers from src1 (d0-d3 = q0,q1)
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 8 integers from src2 (d4-d7 = q2,q3)
            "    VSUB.I32 q0, q0, q2\n"     // Subtract first 4 integers (q0 - q2)
            "    VSUB.I32 q1, q1, q3\n"     // Subtract next 4 integers (q1 - q3)
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (8 integers)
            "    SUBS %[n], %[n], #16\n"    // Decrement counter by 16 elements
            "    BGT NEONSubIntPLD%=\n"     // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] - src2[i];
    }
#endif  // ARCH_ARM
}

void multiply_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile int* dst_main = dst;
        volatile const int* src1_main = src1;
        volatile const int* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONMulIntPLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 8 integers from src1 (d0-d3 = q0,q1)
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 8 integers from src2 (d4-d7 = q2,q3)
            "    VMUL.I32 q0, q0, q2\n"     // Multiply first 4 integers (q0 * q2)
            "    VMUL.I32 q1, q1, q3\n"     // Multiply next 4 integers (q1 * q3)
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (8 integers)
            "    SUBS %[n], %[n], #16\n"    // Decrement counter by 16 elements
            "    BGT NEONMulIntPLD%=\n"     // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] * src2[i];
    }
#endif  // ARCH_ARM
}

void add_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile int16_t* dst_main = dst;
        volatile const int16_t* src1_main = src1;
        volatile const int16_t* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONAddInt16PLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 16 int16_t from src1
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 16 int16_t from src2
            "    VADD.I16 q0, q0, q2\n"     // Add first 8 int16_t (q0 + q2)
            "    VADD.I16 q1, q1, q3\n"     // Add next 8 int16_t (q1 + q3)
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (16 int16_t)
            "    SUBS %[n], %[n], #16\n"    // Decrement counter by 16 elements
            "    BGT NEONAddInt16PLD%=\n"   // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] + src2[i];
    }
#endif  // ARCH_ARM
}

void subtract_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile int16_t* dst_main = dst;
        volatile const int16_t* src1_main = src1;
        volatile const int16_t* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONSubInt16PLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 16 int16_t from src1
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 16 int16_t from src2
            "    VSUB.I16 q0, q0, q2\n"     // Subtract first 8 int16_t (q0 - q2)
            "    VSUB.I16 q1, q1, q3\n"     // Subtract next 8 int16_t (q1 - q3)
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (16 int16_t)
            "    SUBS %[n], %[n], #16\n"    // Decrement counter by 16 elements
            "    BGT NEONSubInt16PLD%=\n"   // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] - src2[i];
    }
#endif  // ARCH_ARM
}

void multiply_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile int16_t* dst_main = dst;
        volatile const int16_t* src1_main = src1;
        volatile const int16_t* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONMulInt16PLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 16 int16_t from src1
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 16 int16_t from src2
            "    VMUL.I16 q0, q0, q2\n"     // Multiply first 8 int16_t (q0 * q2)
            "    VMUL.I16 q1, q1, q3\n"     // Multiply next 8 int16_t (q1 * q3)
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (16 int16_t)
            "    SUBS %[n], %[n], #16\n"    // Decrement counter by 16 elements
            "    BGT NEONMulInt16PLD%=\n"   // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] * src2[i];
    }
#endif  // ARCH_ARM
}

void add_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src1_main = src1;
        volatile const double* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONAddDoublePLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 4 doubles from src1
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 4 doubles from src2
            "    VADD.F64 d0, d0, d4\n"     // Add first 2 doubles
            "    VADD.F64 d1, d1, d5\n"     // Add next 2 doubles
            "    VADD.F64 d2, d2, d6\n"     // Add next 2 doubles
            "    VADD.F64 d3, d3, d7\n"     // Add last 2 doubles
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (4 doubles)
            "    SUBS %[n], %[n], #8\n"     // Decrement counter by 8 elements
            "    BGT NEONAddDoublePLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] + src2[i];
    }
#endif  // ARCH_ARM
}

void subtract_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src1_main = src1;
        volatile const double* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONSubDoublePLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 4 doubles from src1
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 4 doubles from src2
            "    VSUB.F64 d0, d0, d4\n"     // Subtract first 2 doubles
            "    VSUB.F64 d1, d1, d5\n"     // Subtract next 2 doubles
            "    VSUB.F64 d2, d2, d6\n"     // Subtract next 2 doubles
            "    VSUB.F64 d3, d3, d7\n"     // Subtract last 2 doubles
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (4 doubles)
            "    SUBS %[n], %[n], #8\n"     // Decrement counter by 8 elements
            "    BGT NEONSubDoublePLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] - src2[i];
    }
#endif  // ARCH_ARM
}

void multiply_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src1_main = src1;
        volatile const double* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONMulDoublePLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 4 doubles from src1
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 4 doubles from src2
            "    VMUL.F64 d0, d0, d4\n"     // Multiply first 2 doubles
            "    VMUL.F64 d1, d1, d5\n"     // Multiply next 2 doubles
            "    VMUL.F64 d2, d2, d6\n"     // Multiply next 2 doubles
            "    VMUL.F64 d3, d3, d7\n"     // Multiply last 2 doubles
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (4 doubles)
            "    SUBS %[n], %[n], #8\n"     // Decrement counter by 8 elements
            "    BGT NEONMulDoublePLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] * src2[i];
    }
#endif  // ARCH_ARM
}

void divide_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] / src2[i];
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src1_main = src1;
        volatile const double* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "NEONDivDoublePLD%=:\n"
            "    PLD [%[src1], #0xC0]\n"    // Preload data cache for src1
            "    PLD [%[src2], #0xC0]\n"    // Preload data cache for src2
            "    VLDM %[src1]!, {d0-d3}\n"  // Load 4 doubles from src1
            "    VLDM %[src2]!, {d4-d7}\n"  // Load 4 doubles from src2
            "    VDIV.F64 d0, d0, d4\n"     // Divide first 2 doubles
            "    VDIV.F64 d1, d1, d5\n"     // Divide next 2 doubles
            "    VDIV.F64 d2, d2, d6\n"     // Divide next 2 doubles
            "    VDIV.F64 d3, d3, d7\n"     // Divide last 2 doubles
            "    VSTM %[dst]!, {d0-d3}\n"   // Store result (4 doubles)
            "    SUBS %[n], %[n], #8\n"     // Decrement counter by 8 elements
            "    BGT NEONDivDoublePLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src1[i] / src2[i];
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] / src2[i];
    }
#endif  // ARCH_ARM
}

void add_scalar_to_array_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] + scalar;
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VDUP.32 q4, %[scalar]\n"  // Duplicate scalar to all lanes of q4 (d8,d9)
            "    VDUP.32 q5, %[scalar]\n"  // Duplicate scalar to all lanes of q5 (d10,d11)
            "NEONAddScalarFloatPLD%=:\n"
            "    PLD [%[src], #0x100]\n"  // Preload 256 bytes (64 floats)

            // Load 16 floats from src (q0-q3 = d0-d7)
            "    VLDM %[src]!, {d0-d7}\n"  // Load 16 floats from src (q0,q1,q2,q3)

            // Add scalar to all 16 floats
            "    VADD.F32 q0, q0, q4\n"  // Add scalar to first 4 floats
            "    VADD.F32 q1, q1, q4\n"  // Add scalar to next 4 floats
            "    VADD.F32 q2, q2, q5\n"  // Add scalar to next 4 floats
            "    VADD.F32 q3, q3, q5\n"  // Add scalar to last 4 floats

            // Store 16 results
            "    VSTM %[dst]!, {d0-d7}\n"  // Store result (16 floats)

            "    SUBS %[n], %[n], #16\n"         // Decrement counter by 16 elements
            "    BGT NEONAddScalarFloatPLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "r"(scalar)
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10", "d11", "q0", "q1", "q2", "q3", "q4", "q5", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src[i] + scalar;
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] + scalar;
    }
#endif  // ARCH_ARM
}

void subtract_scalar_from_array_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] - scalar;
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VDUP.32 q4, %[scalar]\n"  // Duplicate scalar to all lanes of q4 (d8,d9)
            "    VDUP.32 q5, %[scalar]\n"  // Duplicate scalar to all lanes of q5 (d10,d11)
            "NEONSubScalarFloatPLD%=:\n"
            "    PLD [%[src], #0x100]\n"  // Preload 256 bytes (64 floats)

            // Load 16 floats from src (q0-q3 = d0-d7)
            "    VLDM %[src]!, {d0-d7}\n"  // Load 16 floats from src (q0,q1,q2,q3)

            // Subtract scalar from all 16 floats
            "    VSUB.F32 q0, q0, q4\n"  // Subtract scalar from first 4 floats
            "    VSUB.F32 q1, q1, q4\n"  // Subtract scalar from next 4 floats
            "    VSUB.F32 q2, q2, q5\n"  // Subtract scalar from next 4 floats
            "    VSUB.F32 q3, q3, q5\n"  // Subtract scalar from last 4 floats

            // Store 16 results
            "    VSTM %[dst]!, {d0-d7}\n"  // Store result (16 floats)

            "    SUBS %[n], %[n], #16\n"         // Decrement counter by 16 elements
            "    BGT NEONSubScalarFloatPLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "r"(scalar)
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10", "d11", "q0", "q1", "q2", "q3", "q4", "q5", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src[i] - scalar;
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] - scalar;
    }
#endif  // ARCH_ARM
}

void multiply_array_by_scalar_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 16) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] * scalar;
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VDUP.32 q4, %[scalar]\n"  // Duplicate scalar to all lanes of q4 (d8,d9)
            "    VDUP.32 q5, %[scalar]\n"  // Duplicate scalar to all lanes of q5 (d10,d11)
            "NEONMulScalarFloatPLD%=:\n"
            "    PLD [%[src], #0x100]\n"  // Preload 256 bytes (64 floats)

            // Load 16 floats from src (q0-q3 = d0-d7)
            "    VLDM %[src]!, {d0-d7}\n"  // Load 16 floats from src (q0,q1,q2,q3)

            // Multiply all 16 floats by scalar
            "    VMUL.F32 q0, q0, q4\n"  // Multiply first 4 floats by scalar
            "    VMUL.F32 q1, q1, q4\n"  // Multiply next 4 floats by scalar
            "    VMUL.F32 q2, q2, q5\n"  // Multiply next 4 floats by scalar
            "    VMUL.F32 q3, q3, q5\n"  // Multiply last 4 floats by scalar

            // Store 16 results
            "    VSTM %[dst]!, {d0-d7}\n"  // Store result (16 floats)

            "    SUBS %[n], %[n], #16\n"         // Decrement counter by 16 elements
            "    BGT NEONMulScalarFloatPLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "r"(scalar)
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10", "d11", "q0", "q1", "q2", "q3", "q4", "q5", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src[i] * scalar;
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] * scalar;
    }
#endif  // ARCH_ARM
}
void divide_array_by_scalar_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] / scalar;
        }
        return;
    }

    size_t main_count = n & ~0x7;
    size_t remainder = n & 0x7;

    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VDUP.32 q4, %[scalar]\n"
            "    VRECPE.F32 q5, q4\n"
            "    VRECPS.F32 q6, q5, q4\n"
            "    VMUL.F32 q5, q5, q6\n"
            "NEONDivScalarFloatPLD%=:\n"
            "    PLD [%[src], #0x80]\n"
            "    VLDM %[src]!, {d0-d3}\n"
            "    VMUL.F32 q0, q0, q5\n"
            "    VMUL.F32 q1, q1, q5\n"
            "    VSTM %[dst]!, {d0-d3}\n"
            "    SUBS %[n], %[n], #8\n"
            "    BGT NEONDivScalarFloatPLD%=\n"
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "r"(scalar)
            : "d0", "d1", "d2", "d3", "d8", "d9", "d10", "d11", "d12", "d13", "q0", "q1", "q4", "q5", "q6", "cc", "memory");
    }

    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src[i] / scalar;
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] / scalar;
    }
#endif
}

void add_scalar_to_array_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] + scalar;
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VMOV d8, %[scalar]\n"  // Move scalar to NEON register
            "NEONAddScalarDoublePLD%=:\n"
            "    PLD [%[src], #0xC0]\n"           // Preload data cache for src
            "    VLDM %[src]!, {d0-d3}\n"         // Load 4 doubles from src
            "    VADD.F64 d0, d0, d8\n"           // Add scalar to first 2 doubles
            "    VADD.F64 d1, d1, d8\n"           // Add scalar to next 2 doubles
            "    VADD.F64 d2, d2, d8\n"           // Add scalar to next 2 doubles
            "    VADD.F64 d3, d3, d8\n"           // Add scalar to last 2 doubles
            "    VSTM %[dst]!, {d0-d3}\n"         // Store result (4 doubles)
            "    SUBS %[n], %[n], #8\n"           // Decrement counter by 8 elements
            "    BGT NEONAddScalarDoublePLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "w"(scalar)
            : "d0", "d1", "d2", "d3", "d8", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src[i] + scalar;
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] + scalar;
    }
#endif  // ARCH_ARM
}

void subtract_scalar_from_array_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] - scalar;
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VMOV d8, %[scalar]\n"  // Move scalar to NEON register
            "NEONSubScalarDoublePLD%=:\n"
            "    PLD [%[src], #0xC0]\n"           // Preload data cache for src
            "    VLDM %[src]!, {d0-d3}\n"         // Load 4 doubles from src
            "    VSUB.F64 d0, d0, d8\n"           // Subtract scalar from first 2 doubles
            "    VSUB.F64 d1, d1, d8\n"           // Subtract scalar from next 2 doubles
            "    VSUB.F64 d2, d2, d8\n"           // Subtract scalar from next 2 doubles
            "    VSUB.F64 d3, d3, d8\n"           // Subtract scalar from last 2 doubles
            "    VSTM %[dst]!, {d0-d3}\n"         // Store result (4 doubles)
            "    SUBS %[n], %[n], #8\n"           // Decrement counter by 8 elements
            "    BGT NEONSubScalarDoublePLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "w"(scalar)
            : "d0", "d1", "d2", "d3", "d8", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src[i] - scalar;
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] - scalar;
    }
#endif  // ARCH_ARM
}

void multiply_array_by_scalar_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] * scalar;
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VMOV d8, %[scalar]\n"  // Move scalar to NEON register
            "NEONMulScalarDoublePLD%=:\n"
            "    PLD [%[src], #0xC0]\n"           // Preload data cache for src
            "    VLDM %[src]!, {d0-d3}\n"         // Load 4 doubles from src
            "    VMUL.F64 d0, d0, d8\n"           // Multiply first 2 doubles by scalar
            "    VMUL.F64 d1, d1, d8\n"           // Multiply next 2 doubles by scalar
            "    VMUL.F64 d2, d2, d8\n"           // Multiply next 2 doubles by scalar
            "    VMUL.F64 d3, d3, d8\n"           // Multiply last 2 doubles by scalar
            "    VSTM %[dst]!, {d0-d3}\n"         // Store result (4 doubles)
            "    SUBS %[n], %[n], #8\n"           // Decrement counter by 8 elements
            "    BGT NEONMulScalarDoublePLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "w"(scalar)
            : "d0", "d1", "d2", "d3", "d8", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src[i] * scalar;
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] * scalar;
    }
#endif  // ARCH_ARM
}

void divide_array_by_scalar_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        // Process small arrays with scalar operations
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] / scalar;
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    // Process main portion using NEON
    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VMOV d8, %[scalar]\n"  // Move scalar to NEON register
            "NEONDivScalarDoublePLD%=:\n"
            "    PLD [%[src], #0xC0]\n"           // Preload data cache for src
            "    VLDM %[src]!, {d0-d3}\n"         // Load 4 doubles from src
            "    VDIV.F64 d0, d0, d8\n"           // Divide first 2 doubles by scalar
            "    VDIV.F64 d1, d1, d8\n"           // Divide next 2 doubles by scalar
            "    VDIV.F64 d2, d2, d8\n"           // Divide next 2 doubles by scalar
            "    VDIV.F64 d3, d3, d8\n"           // Divide last 2 doubles by scalar
            "    VSTM %[dst]!, {d0-d3}\n"         // Store result (4 doubles)
            "    SUBS %[n], %[n], #8\n"           // Decrement counter by 8 elements
            "    BGT NEONDivScalarDoublePLD%=\n"  // Branch if more elements to process
            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "w"(scalar)
            : "d0", "d1", "d2", "d3", "d8", "cc", "memory");
    }

    // Process remainder using scalar operations
    if (remainder > 0) {
        for (size_t i = main_count; i < n; ++i) {
            dst[i] = src[i] / scalar;
        }
    }
#else
    // Scalar version for other architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] / scalar;
    }
#endif  // ARCH_ARM
}

#pragma GCC diagnostic pop