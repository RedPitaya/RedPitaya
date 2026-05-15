#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"

#include "rp_math.h"
#include <math.h>
#include <cstdint>
#include <cstring>
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
    if (x <= 0.0f) {
        if (x < 0.0f) {
            return NAN;  // or handle as error
        }
        return 0.0f;  // sqrt(0) = 0
    }
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

void add_arrays_float_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n) {
#ifdef ARCH_ARM
    // Increased threshold for scalar path to reduce overhead for small arrays
    if (n < 32) {
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
            // Preload first iteration data into cache
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONAddPLD%=:\n"
            // Preload data for next iteration (256 bytes ahead of current position)
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 16 floats from src1 using 128-bit loads (faster than VLDM on many cores)
            "    VLD1.32 {q0-q1}, [%[src1]]!\n"  // Load 8 floats (q0-q1) from src1
            "    VLD1.32 {q2-q3}, [%[src1]]!\n"  // Load next 8 floats (q2-q3) from src1

            // Load 16 floats from src2 using 128-bit loads
            "    VLD1.32 {q4-q5}, [%[src2]]!\n"  // Load 8 floats (q4-q5) from src2
            "    VLD1.32 {q6-q7}, [%[src2]]!\n"  // Load next 8 floats (q6-q7) from src2

            // Add 16 floats (4 quad-word operations)
            "    VADD.F32 q0, q0, q4\n"  // Add first 4 floats
            "    VADD.F32 q1, q1, q5\n"  // Add next 4 floats
            "    VADD.F32 q2, q2, q6\n"  // Add next 4 floats
            "    VADD.F32 q3, q3, q7\n"  // Add last 4 floats

            // Store 16 results using 128-bit stores
            "    VST1.32 {q0-q1}, [%[dst]]!\n"  // Store 8 floats
            "    VST1.32 {q2-q3}, [%[dst]]!\n"  // Store next 8 floats

            "    SUBS %[n], %[n], #16\n"  // Decrement counter by 16 elements
            "    BGT NEONAddPLD%=\n"      // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    // Handle remaining elements (0-15 floats)
    if (remainder > 0) {
        size_t i = main_count;
        // Unroll remainder loop: process 4 elements at a time
        for (; i + 3 < n; i += 4) {
            dst[i] = src1[i] + src2[i];
            dst[i + 1] = src1[i + 1] + src2[i + 1];
            dst[i + 2] = src1[i + 2] + src2[i + 2];
            dst[i + 3] = src1[i + 3] + src2[i + 3];
        }
        // Finish any remaining elements (0-3 floats)
        for (; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] + src2[i];
    }
#endif
}

void subtract_arrays_float_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n) {
#ifdef ARCH_ARM
    // Increased threshold for scalar path to reduce overhead for small arrays
    if (n < 32) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
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
            // Preload first iteration data into cache
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONSubPLD%=:\n"
            // Preload data for next iteration (256 bytes ahead of current position)
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 16 floats from src1 using 128-bit loads (faster than VLDM on many cores)
            "    VLD1.32 {q0-q1}, [%[src1]]!\n"  // Load 8 floats (q0-q1) from src1
            "    VLD1.32 {q2-q3}, [%[src1]]!\n"  // Load next 8 floats (q2-q3) from src1

            // Load 16 floats from src2 using 128-bit loads
            "    VLD1.32 {q4-q5}, [%[src2]]!\n"  // Load 8 floats (q4-q5) from src2
            "    VLD1.32 {q6-q7}, [%[src2]]!\n"  // Load next 8 floats (q6-q7) from src2

            // Subtract 16 floats (4 quad-word operations)
            "    VSUB.F32 q0, q0, q4\n"  // Subtract first 4 floats
            "    VSUB.F32 q1, q1, q5\n"  // Subtract next 4 floats
            "    VSUB.F32 q2, q2, q6\n"  // Subtract next 4 floats
            "    VSUB.F32 q3, q3, q7\n"  // Subtract last 4 floats

            // Store 16 results using 128-bit stores
            "    VST1.32 {q0-q1}, [%[dst]]!\n"  // Store 8 floats
            "    VST1.32 {q2-q3}, [%[dst]]!\n"  // Store next 8 floats

            "    SUBS %[n], %[n], #16\n"  // Decrement counter by 16 elements
            "    BGT NEONSubPLD%=\n"      // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    // Handle remaining elements (0-15 floats)
    if (remainder > 0) {
        size_t i = main_count;
        // Unroll remainder loop: process 4 elements at a time
        for (; i + 3 < n; i += 4) {
            dst[i] = src1[i] - src2[i];
            dst[i + 1] = src1[i + 1] - src2[i + 1];
            dst[i + 2] = src1[i + 2] - src2[i + 2];
            dst[i + 3] = src1[i + 3] - src2[i + 3];
        }
        // Finish any remaining elements (0-3 floats)
        for (; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] - src2[i];
    }
#endif
}

void multiply_arrays_float_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n) {
#ifdef ARCH_ARM
    // Increased threshold for scalar path to reduce overhead for small arrays
    if (n < 32) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
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
            // Preload first iteration data into cache
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONMulPLD%=:\n"
            // Preload data for next iteration (256 bytes ahead of current position)
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 16 floats from src1 using 128-bit loads (faster than VLDM on many cores)
            "    VLD1.32 {q0-q1}, [%[src1]]!\n"  // Load 8 floats (q0-q1) from src1
            "    VLD1.32 {q2-q3}, [%[src1]]!\n"  // Load next 8 floats (q2-q3) from src1

            // Load 16 floats from src2 using 128-bit loads
            "    VLD1.32 {q4-q5}, [%[src2]]!\n"  // Load 8 floats (q4-q5) from src2
            "    VLD1.32 {q6-q7}, [%[src2]]!\n"  // Load next 8 floats (q6-q7) from src2

            // Multiply 16 floats (4 quad-word operations)
            "    VMUL.F32 q0, q0, q4\n"  // Multiply first 4 floats
            "    VMUL.F32 q1, q1, q5\n"  // Multiply next 4 floats
            "    VMUL.F32 q2, q2, q6\n"  // Multiply next 4 floats
            "    VMUL.F32 q3, q3, q7\n"  // Multiply last 4 floats

            // Store 16 results using 128-bit stores
            "    VST1.32 {q0-q1}, [%[dst]]!\n"  // Store 8 floats
            "    VST1.32 {q2-q3}, [%[dst]]!\n"  // Store next 8 floats

            "    SUBS %[n], %[n], #16\n"  // Decrement counter by 16 elements
            "    BGT NEONMulPLD%=\n"      // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    // Handle remaining elements (0-15 floats)
    if (remainder > 0) {
        size_t i = main_count;
        // Unroll remainder loop: process 4 elements at a time
        for (; i + 3 < n; i += 4) {
            dst[i] = src1[i] * src2[i];
            dst[i + 1] = src1[i + 1] * src2[i + 1];
            dst[i + 2] = src1[i + 2] * src2[i + 2];
            dst[i + 3] = src1[i + 3] * src2[i + 3];
        }
        // Finish any remaining elements (0-3 floats)
        for (; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] * src2[i];
    }
#endif
}

void divide_arrays_float_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n) {
#ifdef ARCH_ARM
    // Increased threshold for scalar path to reduce overhead for small arrays
    if (n < 32) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] / src2[i];
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
            // Preload first iteration data into cache
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONDivPLD%=:\n"
            // Preload data for next iteration (256 bytes ahead of current position)
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 16 floats from src1 using 128-bit loads
            "    VLD1.32 {q0-q1}, [%[src1]]!\n"  // Load 8 floats (q0-q1) from src1
            "    VLD1.32 {q2-q3}, [%[src1]]!\n"  // Load next 8 floats (q2-q3) from src1

            // Load 16 floats from src2 using 128-bit loads
            "    VLD1.32 {q4-q5}, [%[src2]]!\n"  // Load 8 floats (q4-q5) from src2
            "    VLD1.32 {q6-q7}, [%[src2]]!\n"  // Load next 8 floats (q6-q7) from src2

            // Compute reciprocal of src2 using Newton-Raphson iteration
            // For first 8 floats (q0-q1 / q4-q5)
            "    VRECPE.F32 q8, q4\n"       // Initial reciprocal approximation for q4
            "    VRECPE.F32 q9, q5\n"       // Initial reciprocal approximation for q5
            "    VRECPS.F32 q10, q8, q4\n"  // Newton-Raphson step 1 for q4
            "    VRECPS.F32 q11, q9, q5\n"  // Newton-Raphson step 1 for q5
            "    VMUL.F32 q8, q8, q10\n"    // Refine: q8 = q8 * q10
            "    VMUL.F32 q9, q9, q11\n"    // Refine: q9 = q9 * q11
            "    VRECPS.F32 q10, q8, q4\n"  // Newton-Raphson step 2 for q4
            "    VRECPS.F32 q11, q9, q5\n"  // Newton-Raphson step 2 for q5
            "    VMUL.F32 q8, q8, q10\n"    // Refine: q8 = q8 * q10
            "    VMUL.F32 q9, q9, q11\n"    // Refine: q9 = q9 * q11

            // For next 8 floats (q2-q3 / q6-q7)
            "    VRECPE.F32 q12, q6\n"       // Initial reciprocal approximation for q6
            "    VRECPE.F32 q13, q7\n"       // Initial reciprocal approximation for q7
            "    VRECPS.F32 q14, q12, q6\n"  // Newton-Raphson step 1 for q6
            "    VRECPS.F32 q15, q13, q7\n"  // Newton-Raphson step 1 for q7
            "    VMUL.F32 q12, q12, q14\n"   // Refine: q12 = q12 * q14
            "    VMUL.F32 q13, q13, q15\n"   // Refine: q13 = q13 * q15
            "    VRECPS.F32 q14, q12, q6\n"  // Newton-Raphson step 2 for q6
            "    VRECPS.F32 q15, q13, q7\n"  // Newton-Raphson step 2 for q7
            "    VMUL.F32 q12, q12, q14\n"   // Refine: q12 = q12 * q14
            "    VMUL.F32 q13, q13, q15\n"   // Refine: q13 = q13 * q15

            // Multiply src1 by refined reciprocal: result = src1 * (1/src2)
            "    VMUL.F32 q0, q0, q8\n"   // First 4 floats: q0 / q4
            "    VMUL.F32 q1, q1, q9\n"   // Next 4 floats: q1 / q5
            "    VMUL.F32 q2, q2, q12\n"  // Next 4 floats: q2 / q6
            "    VMUL.F32 q3, q3, q13\n"  // Last 4 floats: q3 / q7

            // Store 16 results using 128-bit stores
            "    VST1.32 {q0-q1}, [%[dst]]!\n"  // Store 8 floats
            "    VST1.32 {q2-q3}, [%[dst]]!\n"  // Store next 8 floats

            "    SUBS %[n], %[n], #16\n"  // Decrement counter by 16 elements
            "    BGT NEONDivPLD%=\n"      // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15", "cc", "memory");
    }

    // Handle remaining elements (0-15 floats)
    if (remainder > 0) {
        size_t i = main_count;
        // Unroll remainder loop: process 4 elements at a time
        for (; i + 3 < n; i += 4) {
            dst[i] = src1[i] / src2[i];
            dst[i + 1] = src1[i + 1] / src2[i + 1];
            dst[i + 2] = src1[i + 2] / src2[i + 2];
            dst[i + 3] = src1[i + 3] / src2[i + 3];
        }
        // Finish any remaining elements (0-3 floats)
        for (; i < n; ++i) {
            dst[i] = src1[i] / src2[i];
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] / src2[i];
    }
#endif
}

void divide_arrays_neon_Ex(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n, float limit) {
#ifdef ARCH_ARM
    if (n < 16) {
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

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

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
            // Duplicate constants to all lanes
            "    VDUP.32 q8, %[max_val]\n"    // q8 = limit (all lanes)
            "    VDUP.32 q9, %[min_val]\n"    // q9 = -limit (all lanes)
            "    VDUP.32 q10, %[zero_val]\n"  // q10 = 0.0f (all lanes)

            // Preload first iteration
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONDivPLD%=:\n"
            // Preload next iteration
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 16 floats from src1
            "    VLD1.32 {q0-q1}, [%[src1]]!\n"  // Load 8 floats (q0-q1)
            "    VLD1.32 {q2-q3}, [%[src1]]!\n"  // Load next 8 floats (q2-q3)

            // Load 16 floats from src2
            "    VLD1.32 {q4-q5}, [%[src2]]!\n"  // Load 8 floats (q4-q5)
            "    VLD1.32 {q6-q7}, [%[src2]]!\n"  // Load next 8 floats (q6-q7)

            // Check for division by zero in all 16 elements
            "    VCEQ.F32 q11, q4, q10\n"  // q11 = mask (q4 == 0)
            "    VCEQ.F32 q12, q5, q10\n"  // q12 = mask (q5 == 0)
            "    VCEQ.F32 q13, q6, q10\n"  // q13 = mask (q6 == 0)
            "    VCEQ.F32 q14, q7, q10\n"  // q14 = mask (q7 == 0)

            // Calculate reciprocal of src2 using Newton-Raphson iteration
            // For first 8 floats (q4-q5)
            "    VRECPE.F32 q15, q4\n"      // Initial reciprocal estimate for q4
            "    VRECPS.F32 q9, q15, q4\n"  // Newton-Raphson step 1 (using q9 as temp, restored later)
            "    VMUL.F32 q15, q15, q9\n"   // Refine reciprocal
            "    VRECPS.F32 q9, q15, q4\n"  // Newton-Raphson step 2
            "    VMUL.F32 q15, q15, q9\n"   // Final reciprocal for q4

            "    VRECPE.F32 q9, q5\n"      // Initial reciprocal estimate for q5
            "    VRECPS.F32 q4, q9, q5\n"  // Newton-Raphson step 1 (using q4 as temp)
            "    VMUL.F32 q9, q9, q4\n"    // Refine reciprocal
            "    VRECPS.F32 q4, q9, q5\n"  // Newton-Raphson step 2
            "    VMUL.F32 q9, q9, q4\n"    // Final reciprocal for q5

            // For next 8 floats (q6-q7)
            "    VRECPE.F32 q4, q6\n"      // Initial reciprocal estimate for q6
            "    VRECPS.F32 q5, q4, q6\n"  // Newton-Raphson step 1
            "    VMUL.F32 q4, q4, q5\n"    // Refine reciprocal
            "    VRECPS.F32 q5, q4, q6\n"  // Newton-Raphson step 2
            "    VMUL.F32 q4, q4, q5\n"    // Final reciprocal for q6

            "    VRECPE.F32 q5, q7\n"      // Initial reciprocal estimate for q7
            "    VRECPS.F32 q6, q5, q7\n"  // Newton-Raphson step 1
            "    VMUL.F32 q5, q5, q6\n"    // Refine reciprocal
            "    VRECPS.F32 q6, q5, q7\n"  // Newton-Raphson step 2
            "    VMUL.F32 q5, q5, q6\n"    // Final reciprocal for q7

            // Multiply src1 by reciprocal: result = src1 * (1/src2)
            "    VMUL.F32 q0, q0, q15\n"  // First 4: q0 / q4
            "    VMUL.F32 q1, q1, q9\n"   // Next 4: q1 / q5
            "    VMUL.F32 q2, q2, q4\n"   // Next 4: q2 / q6
            "    VMUL.F32 q3, q3, q5\n"   // Last 4: q3 / q7

            // Restore q9 from stack (we clobbered it for -limit)
            // Actually, let's reconstruct -limit instead
            "    VNEG.F32 q9, q8\n"  // q9 = -limit (negate q8)

            // Check sign of division result for zero-division cases
            "    VCGT.F32 q4, q0, q10\n"  // q4 = mask (q0 > 0)
            "    VCGT.F32 q5, q1, q10\n"  // q5 = mask (q1 > 0)
            "    VCGT.F32 q6, q2, q10\n"  // q6 = mask (q2 > 0)
            "    VCGT.F32 q7, q3, q10\n"  // q7 = mask (q3 > 0)

            // For zero division: select +limit if result > 0, else -limit
            "    VBSL q4, q8, q9\n"  // q4 = if (q0>0) then +limit else -limit
            "    VBSL q5, q8, q9\n"  // q5 = if (q1>0) then +limit else -limit
            "    VBSL q6, q8, q9\n"  // q6 = if (q2>0) then +limit else -limit
            "    VBSL q7, q8, q9\n"  // q7 = if (q3>0) then +limit else -limit

            // Select between division result and limit value based on zero mask
            "    VBSL q11, q4, q0\n"  // q0 = if zero_mask then limit_value else division_result
            "    VBSL q12, q5, q1\n"  // q1 = if zero_mask then limit_value else division_result
            "    VBSL q13, q6, q2\n"  // q2 = if zero_mask then limit_value else division_result
            "    VBSL q14, q7, q3\n"  // q3 = if zero_mask then limit_value else division_result

            // Move results back to q0-q3 for storage
            "    VMOV q0, q11\n"
            "    VMOV q1, q12\n"
            "    VMOV q2, q13\n"
            "    VMOV q3, q14\n"

            // Store 16 results
            "    VST1.32 {q0-q1}, [%[dst]]!\n"  // Store 8 floats
            "    VST1.32 {q2-q3}, [%[dst]]!\n"  // Store next 8 floats

            "    SUBS %[n], %[n], #16\n"  // Decrement counter by 16 elements
            "    BGT NEONDivPLD%=\n"      // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            : [max_val] "r"(max_val), [min_val] "r"(min_val), [zero_val] "r"(zero_val)
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15", "cc", "memory");
    }

    // Handle remaining elements (0-15 floats)
    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            for (size_t j = 0; j < 4 && (i + j) < n; ++j) {
                float _v1 = src1[i + j];
                float _v2 = src2[i + j];
                if (_v2 != 0)
                    dst[i + j] = _v1 / _v2;
                else
                    dst[i + j] = _v1 > 0 ? limit : -limit;
            }
        }
        for (; i < n; ++i) {
            float _v1 = src1[i];
            float _v2 = src2[i];
            if (_v2 != 0)
                dst[i] = _v1 / _v2;
            else
                dst[i] = _v1 > 0 ? limit : -limit;
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        float _v1 = src1[i];
        float _v2 = src2[i];
        if (_v2 != 0)
            dst[i] = _v1 / _v2;
        else
            dst[i] = _v1 > 0 ? limit : -limit;
    }
#endif
}

void add_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n) {
#ifdef ARCH_ARM
    // Increased threshold for scalar path to reduce overhead for small arrays
    if (n < 32) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    if (main_count > 0) {
        volatile int* dst_main = dst;
        volatile const int* src1_main = src1;
        volatile const int* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            // Preload first iteration data into cache
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONAddIntPLD%=:\n"
            // Preload data for next iteration (256 bytes ahead of current position)
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 16 integers from src1 using 128-bit loads
            "    VLD1.32 {q0-q1}, [%[src1]]!\n"  // Load 8 integers (q0-q1) from src1
            "    VLD1.32 {q2-q3}, [%[src1]]!\n"  // Load next 8 integers (q2-q3) from src1

            // Load 16 integers from src2 using 128-bit loads
            "    VLD1.32 {q4-q5}, [%[src2]]!\n"  // Load 8 integers (q4-q5) from src2
            "    VLD1.32 {q6-q7}, [%[src2]]!\n"  // Load next 8 integers (q6-q7) from src2

            // Add 16 integers (4 quad-word operations)
            "    VADD.I32 q0, q0, q4\n"  // Add first 4 integers
            "    VADD.I32 q1, q1, q5\n"  // Add next 4 integers
            "    VADD.I32 q2, q2, q6\n"  // Add next 4 integers
            "    VADD.I32 q3, q3, q7\n"  // Add last 4 integers

            // Store 16 results using 128-bit stores
            "    VST1.32 {q0-q1}, [%[dst]]!\n"  // Store 8 integers
            "    VST1.32 {q2-q3}, [%[dst]]!\n"  // Store next 8 integers

            "    SUBS %[n], %[n], #16\n"  // Decrement counter by 16 elements
            "    BGT NEONAddIntPLD%=\n"   // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    // Handle remaining elements (0-15 integers)
    if (remainder > 0) {
        size_t i = main_count;
        // Unroll remainder loop: process 4 elements at a time
        for (; i + 3 < n; i += 4) {
            dst[i] = src1[i] + src2[i];
            dst[i + 1] = src1[i + 1] + src2[i + 1];
            dst[i + 2] = src1[i + 2] + src2[i + 2];
            dst[i + 3] = src1[i + 3] + src2[i + 3];
        }
        // Finish any remaining elements (0-3 integers)
        for (; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] + src2[i];
    }
#endif
}

void subtract_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n) {
#ifdef ARCH_ARM
    // Increased threshold for scalar path to reduce overhead for small arrays
    if (n < 32) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    if (main_count > 0) {
        volatile int* dst_main = dst;
        volatile const int* src1_main = src1;
        volatile const int* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            // Preload first iteration data into cache
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONSubIntPLD%=:\n"
            // Preload data for next iteration (256 bytes ahead of current position)
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 16 integers from src1 using 128-bit loads
            "    VLD1.32 {q0-q1}, [%[src1]]!\n"  // Load 8 integers (q0-q1) from src1
            "    VLD1.32 {q2-q3}, [%[src1]]!\n"  // Load next 8 integers (q2-q3) from src1

            // Load 16 integers from src2 using 128-bit loads
            "    VLD1.32 {q4-q5}, [%[src2]]!\n"  // Load 8 integers (q4-q5) from src2
            "    VLD1.32 {q6-q7}, [%[src2]]!\n"  // Load next 8 integers (q6-q7) from src2

            // Subtract 16 integers (4 quad-word operations)
            "    VSUB.I32 q0, q0, q4\n"  // Subtract first 4 integers
            "    VSUB.I32 q1, q1, q5\n"  // Subtract next 4 integers
            "    VSUB.I32 q2, q2, q6\n"  // Subtract next 4 integers
            "    VSUB.I32 q3, q3, q7\n"  // Subtract last 4 integers

            // Store 16 results using 128-bit stores
            "    VST1.32 {q0-q1}, [%[dst]]!\n"  // Store 8 integers
            "    VST1.32 {q2-q3}, [%[dst]]!\n"  // Store next 8 integers

            "    SUBS %[n], %[n], #16\n"  // Decrement counter by 16 elements
            "    BGT NEONSubIntPLD%=\n"   // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    // Handle remaining elements (0-15 integers)
    if (remainder > 0) {
        size_t i = main_count;
        // Unroll remainder loop: process 4 elements at a time
        for (; i + 3 < n; i += 4) {
            dst[i] = src1[i] - src2[i];
            dst[i + 1] = src1[i + 1] - src2[i + 1];
            dst[i + 2] = src1[i + 2] - src2[i + 2];
            dst[i + 3] = src1[i + 3] - src2[i + 3];
        }
        // Finish any remaining elements (0-3 integers)
        for (; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] - src2[i];
    }
#endif
}
void multiply_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n) {
#ifdef ARCH_ARM
    // Increased threshold for scalar path to reduce overhead for small arrays
    if (n < 32) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    if (main_count > 0) {
        volatile int* dst_main = dst;
        volatile const int* src1_main = src1;
        volatile const int* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            // Preload first iteration data into cache
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONMulIntPLD%=:\n"
            // Preload data for next iteration (256 bytes ahead of current position)
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 16 integers from src1 using 128-bit loads
            "    VLD1.32 {q0-q1}, [%[src1]]!\n"  // Load 8 integers (q0-q1) from src1
            "    VLD1.32 {q2-q3}, [%[src1]]!\n"  // Load next 8 integers (q2-q3) from src1

            // Load 16 integers from src2 using 128-bit loads
            "    VLD1.32 {q4-q5}, [%[src2]]!\n"  // Load 8 integers (q4-q5) from src2
            "    VLD1.32 {q6-q7}, [%[src2]]!\n"  // Load next 8 integers (q6-q7) from src2

            // Multiply 16 integers (4 quad-word operations)
            "    VMUL.I32 q0, q0, q4\n"  // Multiply first 4 integers
            "    VMUL.I32 q1, q1, q5\n"  // Multiply next 4 integers
            "    VMUL.I32 q2, q2, q6\n"  // Multiply next 4 integers
            "    VMUL.I32 q3, q3, q7\n"  // Multiply last 4 integers

            // Store 16 results using 128-bit stores
            "    VST1.32 {q0-q1}, [%[dst]]!\n"  // Store 8 integers
            "    VST1.32 {q2-q3}, [%[dst]]!\n"  // Store next 8 integers

            "    SUBS %[n], %[n], #16\n"  // Decrement counter by 16 elements
            "    BGT NEONMulIntPLD%=\n"   // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    // Handle remaining elements (0-15 integers)
    if (remainder > 0) {
        size_t i = main_count;
        // Unroll remainder loop: process 4 elements at a time
        for (; i + 3 < n; i += 4) {
            dst[i] = src1[i] * src2[i];
            dst[i + 1] = src1[i + 1] * src2[i + 1];
            dst[i + 2] = src1[i + 2] * src2[i + 2];
            dst[i + 3] = src1[i + 3] * src2[i + 3];
        }
        // Finish any remaining elements (0-3 integers)
        for (; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] * src2[i];
    }
#endif
}

void add_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n) {
#ifdef ARCH_ARM
    // Increased threshold for scalar path to reduce overhead for small arrays
    if (n < 64) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
        return;
    }

    size_t main_count = n & ~0x1F;  // Round down to multiple of 32
    size_t remainder = n & 0x1F;    // Remainder (0-31 elements)

    if (main_count > 0) {
        volatile int16_t* dst_main = dst;
        volatile const int16_t* src1_main = src1;
        volatile const int16_t* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            // Preload first iteration data into cache
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONAddInt16PLD%=:\n"
            // Preload data for next iteration (256 bytes ahead of current position)
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 32 int16_t from src1 using 128-bit loads (each q-register holds 8 int16_t)
            "    VLD1.16 {q0-q1}, [%[src1]]!\n"  // Load 16 int16_t (q0-q1)
            "    VLD1.16 {q2-q3}, [%[src1]]!\n"  // Load next 16 int16_t (q2-q3)

            // Load 32 int16_t from src2 using 128-bit loads
            "    VLD1.16 {q4-q5}, [%[src2]]!\n"  // Load 16 int16_t (q4-q5)
            "    VLD1.16 {q6-q7}, [%[src2]]!\n"  // Load next 16 int16_t (q6-q7)

            // Add 32 int16_t (4 quad-word operations, each processing 8 int16_t)
            "    VADD.I16 q0, q0, q4\n"  // Add first 8 int16_t
            "    VADD.I16 q1, q1, q5\n"  // Add next 8 int16_t
            "    VADD.I16 q2, q2, q6\n"  // Add next 8 int16_t
            "    VADD.I16 q3, q3, q7\n"  // Add last 8 int16_t

            // Store 32 results using 128-bit stores
            "    VST1.16 {q0-q1}, [%[dst]]!\n"  // Store 16 int16_t
            "    VST1.16 {q2-q3}, [%[dst]]!\n"  // Store next 16 int16_t

            "    SUBS %[n], %[n], #32\n"   // Decrement counter by 32 elements
            "    BGT NEONAddInt16PLD%=\n"  // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    // Handle remaining elements (0-31 int16_t)
    if (remainder > 0) {
        size_t i = main_count;
        // Unroll remainder loop: process 8 elements at a time
        for (; i + 7 < n; i += 8) {
            dst[i] = src1[i] + src2[i];
            dst[i + 1] = src1[i + 1] + src2[i + 1];
            dst[i + 2] = src1[i + 2] + src2[i + 2];
            dst[i + 3] = src1[i + 3] + src2[i + 3];
            dst[i + 4] = src1[i + 4] + src2[i + 4];
            dst[i + 5] = src1[i + 5] + src2[i + 5];
            dst[i + 6] = src1[i + 6] + src2[i + 6];
            dst[i + 7] = src1[i + 7] + src2[i + 7];
        }
        // Finish any remaining elements (0-7 int16_t)
        for (; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] + src2[i];
    }
#endif
}

void subtract_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n) {
#ifdef ARCH_ARM
    // Increased threshold for scalar path to reduce overhead for small arrays
    if (n < 64) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
        return;
    }

    size_t main_count = n & ~0x1F;  // Round down to multiple of 32
    size_t remainder = n & 0x1F;    // Remainder (0-31 elements)

    if (main_count > 0) {
        volatile int16_t* dst_main = dst;
        volatile const int16_t* src1_main = src1;
        volatile const int16_t* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            // Preload first iteration data into cache
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONSubInt16PLD%=:\n"
            // Preload data for next iteration (256 bytes ahead of current position)
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 32 int16_t from src1 using 128-bit loads (each q-register holds 8 int16_t)
            "    VLD1.16 {q0-q1}, [%[src1]]!\n"  // Load 16 int16_t (q0-q1)
            "    VLD1.16 {q2-q3}, [%[src1]]!\n"  // Load next 16 int16_t (q2-q3)

            // Load 32 int16_t from src2 using 128-bit loads
            "    VLD1.16 {q4-q5}, [%[src2]]!\n"  // Load 16 int16_t (q4-q5)
            "    VLD1.16 {q6-q7}, [%[src2]]!\n"  // Load next 16 int16_t (q6-q7)

            // Subtract 32 int16_t (4 quad-word operations, each processing 8 int16_t)
            "    VSUB.I16 q0, q0, q4\n"  // Subtract first 8 int16_t
            "    VSUB.I16 q1, q1, q5\n"  // Subtract next 8 int16_t
            "    VSUB.I16 q2, q2, q6\n"  // Subtract next 8 int16_t
            "    VSUB.I16 q3, q3, q7\n"  // Subtract last 8 int16_t

            // Store 32 results using 128-bit stores
            "    VST1.16 {q0-q1}, [%[dst]]!\n"  // Store 16 int16_t
            "    VST1.16 {q2-q3}, [%[dst]]!\n"  // Store next 16 int16_t

            "    SUBS %[n], %[n], #32\n"   // Decrement counter by 32 elements
            "    BGT NEONSubInt16PLD%=\n"  // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    // Handle remaining elements (0-31 int16_t)
    if (remainder > 0) {
        size_t i = main_count;
        // Unroll remainder loop: process 8 elements at a time
        for (; i + 7 < n; i += 8) {
            dst[i] = src1[i] - src2[i];
            dst[i + 1] = src1[i + 1] - src2[i + 1];
            dst[i + 2] = src1[i + 2] - src2[i + 2];
            dst[i + 3] = src1[i + 3] - src2[i + 3];
            dst[i + 4] = src1[i + 4] - src2[i + 4];
            dst[i + 5] = src1[i + 5] - src2[i + 5];
            dst[i + 6] = src1[i + 6] - src2[i + 6];
            dst[i + 7] = src1[i + 7] - src2[i + 7];
        }
        // Finish any remaining elements (0-7 int16_t)
        for (; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] - src2[i];
    }
#endif
}
void multiply_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n) {
#ifdef ARCH_ARM
    // Increased threshold for scalar path to reduce overhead for small arrays
    if (n < 64) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
        return;
    }

    size_t main_count = n & ~0x1F;  // Round down to multiple of 32
    size_t remainder = n & 0x1F;    // Remainder (0-31 elements)

    if (main_count > 0) {
        volatile int16_t* dst_main = dst;
        volatile const int16_t* src1_main = src1;
        volatile const int16_t* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            // Preload first iteration data into cache
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONMulInt16PLD%=:\n"
            // Preload data for next iteration (256 bytes ahead of current position)
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 32 int16_t from src1 using 128-bit loads (each q-register holds 8 int16_t)
            "    VLD1.16 {q0-q1}, [%[src1]]!\n"  // Load 16 int16_t (q0-q1)
            "    VLD1.16 {q2-q3}, [%[src1]]!\n"  // Load next 16 int16_t (q2-q3)

            // Load 32 int16_t from src2 using 128-bit loads
            "    VLD1.16 {q4-q5}, [%[src2]]!\n"  // Load 16 int16_t (q4-q5)
            "    VLD1.16 {q6-q7}, [%[src2]]!\n"  // Load next 16 int16_t (q6-q7)

            // Multiply 32 int16_t (4 quad-word operations, each processing 8 int16_t)
            // Note: VMUL.I16 keeps lower 16 bits of 32-bit product (wraparound semantics)
            "    VMUL.I16 q0, q0, q4\n"  // Multiply first 8 int16_t
            "    VMUL.I16 q1, q1, q5\n"  // Multiply next 8 int16_t
            "    VMUL.I16 q2, q2, q6\n"  // Multiply next 8 int16_t
            "    VMUL.I16 q3, q3, q7\n"  // Multiply last 8 int16_t

            // Store 32 results using 128-bit stores
            "    VST1.16 {q0-q1}, [%[dst]]!\n"  // Store 16 int16_t
            "    VST1.16 {q2-q3}, [%[dst]]!\n"  // Store next 16 int16_t

            "    SUBS %[n], %[n], #32\n"   // Decrement counter by 32 elements
            "    BGT NEONMulInt16PLD%=\n"  // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    // Handle remaining elements (0-31 int16_t)
    if (remainder > 0) {
        size_t i = main_count;
        // Unroll remainder loop: process 8 elements at a time
        for (; i + 7 < n; i += 8) {
            dst[i] = src1[i] * src2[i];
            dst[i + 1] = src1[i + 1] * src2[i + 1];
            dst[i + 2] = src1[i + 2] * src2[i + 2];
            dst[i + 3] = src1[i + 3] * src2[i + 3];
            dst[i + 4] = src1[i + 4] * src2[i + 4];
            dst[i + 5] = src1[i + 5] * src2[i + 5];
            dst[i + 6] = src1[i + 6] * src2[i + 6];
            dst[i + 7] = src1[i + 7] * src2[i + 7];
        }
        // Finish any remaining elements (0-7 int16_t)
        for (; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] * src2[i];
    }
#endif
}
void add_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n) {
#ifdef ARCH_ARM
    // Increased threshold for scalar path to reduce overhead for small arrays
    if (n < 8) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src1_main = src1;
        volatile const double* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            // Preload first iteration data into cache
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONAddDoublePLD%=:\n"
            // Preload data for next iteration (256 bytes ahead of current position)
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 8 doubles from src1 using 128-bit loads (each q-register holds 2 doubles)
            "    VLD1.64 {q0-q1}, [%[src1]]!\n"  // Load 4 doubles (q0-q1)
            "    VLD1.64 {q2-q3}, [%[src1]]!\n"  // Load next 4 doubles (q2-q3)

            // Load 8 doubles from src2 using 128-bit loads
            "    VLD1.64 {q4-q5}, [%[src2]]!\n"  // Load 4 doubles (q4-q5)
            "    VLD1.64 {q6-q7}, [%[src2]]!\n"  // Load next 4 doubles (q6-q7)

            // Add 8 doubles (operations on double-word registers, NOT quad-word)
            "    VADD.F64 d0, d0, d8\n"   // Add first double
            "    VADD.F64 d1, d1, d9\n"   // Add second double
            "    VADD.F64 d2, d2, d10\n"  // Add third double
            "    VADD.F64 d3, d3, d11\n"  // Add fourth double
            "    VADD.F64 d4, d4, d12\n"  // Add fifth double
            "    VADD.F64 d5, d5, d13\n"  // Add sixth double
            "    VADD.F64 d6, d6, d14\n"  // Add seventh double
            "    VADD.F64 d7, d7, d15\n"  // Add eighth double

            // Store 8 results using 128-bit stores
            "    VST1.64 {q0-q1}, [%[dst]]!\n"  // Store 4 doubles
            "    VST1.64 {q2-q3}, [%[dst]]!\n"  // Store next 4 doubles

            "    SUBS %[n], %[n], #8\n"     // Decrement counter by 8 elements
            "    BGT NEONAddDoublePLD%=\n"  // Branch if more elements remain

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    // Handle remaining elements (0-7 doubles)
    if (remainder > 0) {
        size_t i = main_count;
        // Unroll remainder loop: process 4 elements at a time
        for (; i + 3 < n; i += 4) {
            dst[i] = src1[i] + src2[i];
            dst[i + 1] = src1[i + 1] + src2[i + 1];
            dst[i + 2] = src1[i + 2] + src2[i + 2];
            dst[i + 3] = src1[i + 3] + src2[i + 3];
        }
        // Finish any remaining elements (0-3 doubles)
        for (; i < n; ++i) {
            dst[i] = src1[i] + src2[i];
        }
    }
#else
    // Scalar fallback for non-ARM architectures
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] + src2[i];
    }
#endif
}

void subtract_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src1_main = src1;
        volatile const double* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONSubDoublePLD%=:\n"
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            "    VLD1.64 {q0-q1}, [%[src1]]!\n"
            "    VLD1.64 {q2-q3}, [%[src1]]!\n"
            "    VLD1.64 {q4-q5}, [%[src2]]!\n"
            "    VLD1.64 {q6-q7}, [%[src2]]!\n"

            "    VSUB.F64 d0, d0, d8\n"
            "    VSUB.F64 d1, d1, d9\n"
            "    VSUB.F64 d2, d2, d10\n"
            "    VSUB.F64 d3, d3, d11\n"
            "    VSUB.F64 d4, d4, d12\n"
            "    VSUB.F64 d5, d5, d13\n"
            "    VSUB.F64 d6, d6, d14\n"
            "    VSUB.F64 d7, d7, d15\n"

            "    VST1.64 {q0-q1}, [%[dst]]!\n"
            "    VST1.64 {q2-q3}, [%[dst]]!\n"

            "    SUBS %[n], %[n], #8\n"
            "    BGT NEONSubDoublePLD%=\n"

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            dst[i] = src1[i] - src2[i];
            dst[i + 1] = src1[i + 1] - src2[i + 1];
            dst[i + 2] = src1[i + 2] - src2[i + 2];
            dst[i + 3] = src1[i + 3] - src2[i + 3];
        }
        for (; i < n; ++i) {
            dst[i] = src1[i] - src2[i];
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] - src2[i];
    }
#endif
}

void multiply_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src1_main = src1;
        volatile const double* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONMulDoublePLD%=:\n"
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            "    VLD1.64 {q0-q1}, [%[src1]]!\n"
            "    VLD1.64 {q2-q3}, [%[src1]]!\n"
            "    VLD1.64 {q4-q5}, [%[src2]]!\n"
            "    VLD1.64 {q6-q7}, [%[src2]]!\n"

            "    VMUL.F64 d0, d0, d8\n"
            "    VMUL.F64 d1, d1, d9\n"
            "    VMUL.F64 d2, d2, d10\n"
            "    VMUL.F64 d3, d3, d11\n"
            "    VMUL.F64 d4, d4, d12\n"
            "    VMUL.F64 d5, d5, d13\n"
            "    VMUL.F64 d6, d6, d14\n"
            "    VMUL.F64 d7, d7, d15\n"

            "    VST1.64 {q0-q1}, [%[dst]]!\n"
            "    VST1.64 {q2-q3}, [%[dst]]!\n"

            "    SUBS %[n], %[n], #8\n"
            "    BGT NEONMulDoublePLD%=\n"

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            dst[i] = src1[i] * src2[i];
            dst[i + 1] = src1[i + 1] * src2[i + 1];
            dst[i + 2] = src1[i + 2] * src2[i + 2];
            dst[i + 3] = src1[i + 3] * src2[i + 3];
        }
        for (; i < n; ++i) {
            dst[i] = src1[i] * src2[i];
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] * src2[i];
    }
#endif
}

void divide_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src1[i] / src2[i];
        }
        return;
    }

    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src1_main = src1;
        volatile const double* src2_main = src2;
        size_t n_main = main_count;

        asm volatile(
            "    PLD [%[src1], #0x100]\n"
            "    PLD [%[src2], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONDivDoublePLD%=:\n"
            "    PLD [%[src1], #0x180]\n"
            "    PLD [%[src2], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            "    VLD1.64 {q0-q1}, [%[src1]]!\n"
            "    VLD1.64 {q2-q3}, [%[src1]]!\n"
            "    VLD1.64 {q4-q5}, [%[src2]]!\n"
            "    VLD1.64 {q6-q7}, [%[src2]]!\n"

            // VDIV.F64 is a VFP instruction, not NEON - serialized and slow
            "    VDIV.F64 d0, d0, d8\n"
            "    VDIV.F64 d1, d1, d9\n"
            "    VDIV.F64 d2, d2, d10\n"
            "    VDIV.F64 d3, d3, d11\n"
            "    VDIV.F64 d4, d4, d12\n"
            "    VDIV.F64 d5, d5, d13\n"
            "    VDIV.F64 d6, d6, d14\n"
            "    VDIV.F64 d7, d7, d15\n"

            "    VST1.64 {q0-q1}, [%[dst]]!\n"
            "    VST1.64 {q2-q3}, [%[dst]]!\n"

            "    SUBS %[n], %[n], #8\n"
            "    BGT NEONDivDoublePLD%=\n"

            : [dst] "+r"(dst_main), [src1] "+r"(src1_main), [src2] "+r"(src2_main), [n] "+r"(n_main)
            :
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc", "memory");
    }

    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            dst[i] = src1[i] / src2[i];
            dst[i + 1] = src1[i + 1] / src2[i + 1];
            dst[i + 2] = src1[i + 2] / src2[i + 2];
            dst[i + 3] = src1[i + 3] / src2[i + 3];
        }
        for (; i < n; ++i) {
            dst[i] = src1[i] / src2[i];
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src1[i] / src2[i];
    }
#endif
}

void add_scalar_to_array_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 32) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] + scalar;
        }
        return;
    }

    size_t main_count = n & ~0xF;  // Round down to multiple of 16
    size_t remainder = n & 0xF;    // Remainder (0-15 elements)

    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            // Duplicate scalar to all 4 lanes of q4 (only need one q-register for scalar)
            "    VDUP.32 q4, %[scalar]\n"

            // Preload first iteration data into cache
            "    PLD [%[src], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONAddScalarFloatPLD%=:\n"
            // Preload data for next iteration
            "    PLD [%[src], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 16 floats from src using modern 128-bit loads
            "    VLD1.32 {q0-q1}, [%[src]]!\n"  // Load 8 floats
            "    VLD1.32 {q2-q3}, [%[src]]!\n"  // Load next 8 floats

            // Add scalar to all 16 floats
            "    VADD.F32 q0, q0, q4\n"  // Add scalar to first 4 floats
            "    VADD.F32 q1, q1, q4\n"  // Add scalar to next 4 floats
            "    VADD.F32 q2, q2, q4\n"  // Add scalar to next 4 floats
            "    VADD.F32 q3, q3, q4\n"  // Add scalar to last 4 floats

            // Store 16 results using modern 128-bit stores
            "    VST1.32 {q0-q1}, [%[dst]]!\n"  // Store 8 floats
            "    VST1.32 {q2-q3}, [%[dst]]!\n"  // Store next 8 floats

            "    SUBS %[n], %[n], #16\n"         // Decrement counter by 16 elements
            "    BGT NEONAddScalarFloatPLD%=\n"  // Branch if more elements

            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "r"(scalar)
            : "q0", "q1", "q2", "q3", "q4", "cc", "memory");
    }

    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            dst[i] = src[i] + scalar;
            dst[i + 1] = src[i + 1] + scalar;
            dst[i + 2] = src[i + 2] + scalar;
            dst[i + 3] = src[i + 3] + scalar;
        }
        for (; i < n; ++i) {
            dst[i] = src[i] + scalar;
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] + scalar;
    }
#endif
}

void subtract_scalar_from_array_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 32) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] - scalar;
        }
        return;
    }

    size_t main_count = n & ~0xF;
    size_t remainder = n & 0xF;

    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VDUP.32 q4, %[scalar]\n"
            "    PLD [%[src], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONSubScalarFloatPLD%=:\n"
            "    PLD [%[src], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"
            "    VLD1.32 {q0-q1}, [%[src]]!\n"
            "    VLD1.32 {q2-q3}, [%[src]]!\n"
            "    VSUB.F32 q0, q0, q4\n"
            "    VSUB.F32 q1, q1, q4\n"
            "    VSUB.F32 q2, q2, q4\n"
            "    VSUB.F32 q3, q3, q4\n"
            "    VST1.32 {q0-q1}, [%[dst]]!\n"
            "    VST1.32 {q2-q3}, [%[dst]]!\n"
            "    SUBS %[n], %[n], #16\n"
            "    BGT NEONSubScalarFloatPLD%=\n"

            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "r"(scalar)
            : "q0", "q1", "q2", "q3", "q4", "cc", "memory");
    }

    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            dst[i] = src[i] - scalar;
            dst[i + 1] = src[i + 1] - scalar;
            dst[i + 2] = src[i + 2] - scalar;
            dst[i + 3] = src[i + 3] - scalar;
        }
        for (; i < n; ++i) {
            dst[i] = src[i] - scalar;
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] - scalar;
    }
#endif
}

void multiply_array_by_scalar_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 32) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] * scalar;
        }
        return;
    }

    size_t main_count = n & ~0xF;
    size_t remainder = n & 0xF;

    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VDUP.32 q4, %[scalar]\n"
            "    PLD [%[src], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONMulScalarFloatPLD%=:\n"
            "    PLD [%[src], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"
            "    VLD1.32 {q0-q1}, [%[src]]!\n"
            "    VLD1.32 {q2-q3}, [%[src]]!\n"
            "    VMUL.F32 q0, q0, q4\n"
            "    VMUL.F32 q1, q1, q4\n"
            "    VMUL.F32 q2, q2, q4\n"
            "    VMUL.F32 q3, q3, q4\n"
            "    VST1.32 {q0-q1}, [%[dst]]!\n"
            "    VST1.32 {q2-q3}, [%[dst]]!\n"
            "    SUBS %[n], %[n], #16\n"
            "    BGT NEONMulScalarFloatPLD%=\n"

            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "r"(scalar)
            : "q0", "q1", "q2", "q3", "q4", "cc", "memory");
    }

    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            dst[i] = src[i] * scalar;
            dst[i + 1] = src[i + 1] * scalar;
            dst[i + 2] = src[i + 2] * scalar;
            dst[i + 3] = src[i + 3] * scalar;
        }
        for (; i < n; ++i) {
            dst[i] = src[i] * scalar;
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] * scalar;
    }
#endif
}

void divide_array_by_scalar_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 32) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] / scalar;
        }
        return;
    }

    size_t main_count = n & ~0xF;
    size_t remainder = n & 0xF;

    if (main_count > 0) {
        volatile float* dst_main = dst;
        volatile const float* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            // Compute reciprocal of scalar once outside the loop
            "    VDUP.32 q4, %[scalar]\n"  // Duplicate scalar
            "    VRECPE.F32 q5, q4\n"      // Initial reciprocal estimate
            "    VRECPS.F32 q6, q5, q4\n"  // Newton-Raphson step 1
            "    VMUL.F32 q5, q5, q6\n"    // Refine reciprocal
            "    VRECPS.F32 q6, q5, q4\n"  // Newton-Raphson step 2
            "    VMUL.F32 q5, q5, q6\n"    // Final refined reciprocal

            "    PLD [%[src], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONDivScalarFloatPLD%=:\n"
            "    PLD [%[src], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"
            "    VLD1.32 {q0-q1}, [%[src]]!\n"
            "    VLD1.32 {q2-q3}, [%[src]]!\n"
            "    VMUL.F32 q0, q0, q5\n"  // Multiply by reciprocal
            "    VMUL.F32 q1, q1, q5\n"
            "    VMUL.F32 q2, q2, q5\n"
            "    VMUL.F32 q3, q3, q5\n"
            "    VST1.32 {q0-q1}, [%[dst]]!\n"
            "    VST1.32 {q2-q3}, [%[dst]]!\n"
            "    SUBS %[n], %[n], #16\n"
            "    BGT NEONDivScalarFloatPLD%=\n"

            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "r"(scalar)
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "cc", "memory");
    }

    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            dst[i] = src[i] / scalar;
            dst[i + 1] = src[i + 1] / scalar;
            dst[i + 2] = src[i + 2] / scalar;
            dst[i + 3] = src[i + 3] / scalar;
        }
        for (; i < n; ++i) {
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
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] + scalar;
        }
        return;
    }

    size_t main_count = n & ~0x7;
    size_t remainder = n & 0x7;

    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            // Load scalar into a d-register (not q-register for double)
            "    VMOV.F64 d16, %[scalar]\n"

            "    PLD [%[src], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONAddScalarDoublePLD%=:\n"
            "    PLD [%[src], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            "    VLD1.64 {q0-q1}, [%[src]]!\n"
            "    VLD1.64 {q2-q3}, [%[src]]!\n"

            // Use d-register operations with scalar in d16
            "    VADD.F64 d0, d0, d16\n"
            "    VADD.F64 d1, d1, d16\n"
            "    VADD.F64 d2, d2, d16\n"
            "    VADD.F64 d3, d3, d16\n"
            "    VADD.F64 d4, d4, d16\n"
            "    VADD.F64 d5, d5, d16\n"
            "    VADD.F64 d6, d6, d16\n"
            "    VADD.F64 d7, d7, d16\n"

            "    VST1.64 {q0-q1}, [%[dst]]!\n"
            "    VST1.64 {q2-q3}, [%[dst]]!\n"

            "    SUBS %[n], %[n], #8\n"
            "    BGT NEONAddScalarDoublePLD%=\n"

            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "w"(scalar)
            : "q0", "q1", "q2", "q3", "d16", "cc", "memory");
    }

    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            dst[i] = src[i] + scalar;
            dst[i + 1] = src[i + 1] + scalar;
            dst[i + 2] = src[i + 2] + scalar;
            dst[i + 3] = src[i + 3] + scalar;
        }
        for (; i < n; ++i) {
            dst[i] = src[i] + scalar;
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] + scalar;
    }
#endif
}

void subtract_scalar_from_array_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] - scalar;
        }
        return;
    }

    size_t main_count = n & ~0x7;
    size_t remainder = n & 0x7;

    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VMOV.F64 d16, %[scalar]\n"
            "    PLD [%[src], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONSubScalarDoublePLD%=:\n"
            "    PLD [%[src], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            "    VLD1.64 {q0-q1}, [%[src]]!\n"
            "    VLD1.64 {q2-q3}, [%[src]]!\n"

            "    VSUB.F64 d0, d0, d16\n"
            "    VSUB.F64 d1, d1, d16\n"
            "    VSUB.F64 d2, d2, d16\n"
            "    VSUB.F64 d3, d3, d16\n"
            "    VSUB.F64 d4, d4, d16\n"
            "    VSUB.F64 d5, d5, d16\n"
            "    VSUB.F64 d6, d6, d16\n"
            "    VSUB.F64 d7, d7, d16\n"

            "    VST1.64 {q0-q1}, [%[dst]]!\n"
            "    VST1.64 {q2-q3}, [%[dst]]!\n"

            "    SUBS %[n], %[n], #8\n"
            "    BGT NEONSubScalarDoublePLD%=\n"

            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "w"(scalar)
            : "q0", "q1", "q2", "q3", "d16", "cc", "memory");
    }

    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            dst[i] = src[i] - scalar;
            dst[i + 1] = src[i + 1] - scalar;
            dst[i + 2] = src[i + 2] - scalar;
            dst[i + 3] = src[i + 3] - scalar;
        }
        for (; i < n; ++i) {
            dst[i] = src[i] - scalar;
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] - scalar;
    }
#endif
}

void multiply_array_by_scalar_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n) {
#ifdef ARCH_ARM
    if (n < 8) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] * scalar;
        }
        return;
    }

    size_t main_count = n & ~0x7;
    size_t remainder = n & 0x7;

    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            "    VMOV.F64 d16, %[scalar]\n"
            "    PLD [%[src], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONMulScalarDoublePLD%=:\n"
            "    PLD [%[src], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            "    VLD1.64 {q0-q1}, [%[src]]!\n"
            "    VLD1.64 {q2-q3}, [%[src]]!\n"

            "    VMUL.F64 d0, d0, d16\n"
            "    VMUL.F64 d1, d1, d16\n"
            "    VMUL.F64 d2, d2, d16\n"
            "    VMUL.F64 d3, d3, d16\n"
            "    VMUL.F64 d4, d4, d16\n"
            "    VMUL.F64 d5, d5, d16\n"
            "    VMUL.F64 d6, d6, d16\n"
            "    VMUL.F64 d7, d7, d16\n"

            "    VST1.64 {q0-q1}, [%[dst]]!\n"
            "    VST1.64 {q2-q3}, [%[dst]]!\n"

            "    SUBS %[n], %[n], #8\n"
            "    BGT NEONMulScalarDoublePLD%=\n"

            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "w"(scalar)
            : "q0", "q1", "q2", "q3", "d16", "cc", "memory");
    }

    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            dst[i] = src[i] * scalar;
            dst[i + 1] = src[i + 1] * scalar;
            dst[i + 2] = src[i + 2] * scalar;
            dst[i + 3] = src[i + 3] * scalar;
        }
        for (; i < n; ++i) {
            dst[i] = src[i] * scalar;
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] * scalar;
    }
#endif
}

void divide_array_by_scalar_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n) {
#ifdef ARCH_ARM
    // VDIV.F64 is very slow, use higher threshold
    if (n < 32) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i] / scalar;
        }
        return;
    }

    // Process fewer doubles per iteration due to VDIV being serialized
    size_t main_count = n & ~0x7;  // Round down to multiple of 8
    size_t remainder = n & 0x7;    // Remainder (0-7 elements)

    if (main_count > 0) {
        volatile double* dst_main = dst;
        volatile const double* src_main = src;
        size_t n_main = main_count;

        asm volatile(
            // Move scalar to d-register (only need one double lane)
            "    VMOV.F64 d8, %[scalar]\n"

            "    PLD [%[src], #0x100]\n"
            "    PLD [%[dst], #0x100]\n"

            "NEONDivScalarDoublePLD%=:\n"
            "    PLD [%[src], #0x180]\n"
            "    PLD [%[dst], #0x180]\n"

            // Load 8 doubles
            "    VLD1.64 {q0-q1}, [%[src]]!\n"
            "    VLD1.64 {q2-q3}, [%[src]]!\n"

            // Divide each double by scalar (VDIV.F64 is VFP, serialized and slow)
            "    VDIV.F64 d0, d0, d8\n"
            "    VDIV.F64 d1, d1, d8\n"
            "    VDIV.F64 d2, d2, d8\n"
            "    VDIV.F64 d3, d3, d8\n"
            "    VDIV.F64 d4, d4, d8\n"
            "    VDIV.F64 d5, d5, d8\n"
            "    VDIV.F64 d6, d6, d8\n"
            "    VDIV.F64 d7, d7, d8\n"

            // Store 8 results
            "    VST1.64 {q0-q1}, [%[dst]]!\n"
            "    VST1.64 {q2-q3}, [%[dst]]!\n"

            "    SUBS %[n], %[n], #8\n"
            "    BGT NEONDivScalarDoublePLD%=\n"

            : [dst] "+r"(dst_main), [src] "+r"(src_main), [n] "+r"(n_main)
            : [scalar] "w"(scalar)
            : "q0", "q1", "q2", "q3", "d8", "cc", "memory");
    }

    if (remainder > 0) {
        size_t i = main_count;
        for (; i + 3 < n; i += 4) {
            dst[i] = src[i] / scalar;
            dst[i + 1] = src[i + 1] / scalar;
            dst[i + 2] = src[i + 2] / scalar;
            dst[i + 3] = src[i + 3] / scalar;
        }
        for (; i < n; ++i) {
            dst[i] = src[i] / scalar;
        }
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] / scalar;
    }
#endif
}

#pragma GCC diagnostic pop