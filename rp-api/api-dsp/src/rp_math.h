/**
 * @brief Red Pitaya NEON-optimized math functions.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_MATH_H__
#define __RP_MATH_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Basic Math Functions
// ============================================================================

/**
 * @brief NEON-optimized log10 for float.
 * @param val Input value
 * @return log10(val)
 */
float log10f_neon(float val);

/**
 * @brief NEON-optimized sqrt for float.
 * @param val Input value
 * @return sqrt(val)
 */
float sqrtf_neon(float val);

// ============================================================================
// Memory Operations
// ============================================================================

/**
 * @brief NEON-optimized memory copy with volatile support.
 * @param dst Pointer to destination buffer (volatile)
 * @param src Pointer to source buffer (volatile)
 * @param n Number of bytes to copy
 */
void memcpy_neon(volatile void* dst, volatile const void* src, size_t n);

/**
 * @brief NEON-optimized strided memory copy for 8-bit data.
 * @param dst Pointer to destination buffer (volatile)
 * @param src Pointer to source buffer (volatile)
 * @param n Number of bytes to copy
 */
void memcpy_stride_8bit_neon(volatile void* dst, volatile const void* src, size_t n);

// ============================================================================
// Array Arithmetic Functions - Float
// ============================================================================

/**
 * @brief Add two float arrays: dst[i] = src1[i] + src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 */
void add_arrays_float_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n);

/**
 * @brief Subtract two float arrays: dst[i] = src1[i] - src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 */
void subtract_arrays_float_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n);

/**
 * @brief Multiply two float arrays: dst[i] = src1[i] * src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 */
void multiply_arrays_float_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n);

/**
 * @brief Divide two float arrays: dst[i] = src1[i] / src2[i]
 * @param dst Destination array
 * @param src1 Numerator array
 * @param src2 Denominator array
 * @param n Number of elements
 * @note Division by zero produces infinity
 */
void divide_arrays_float_neon(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n);

/**
 * @brief Divide two float arrays with zero handling: dst[i] = src1[i] / src2[i]
 * @param dst Destination array
 * @param src1 Numerator array
 * @param src2 Denominator array
 * @param n Number of elements
 * @param limit Value to use when denominator is zero (sign based on numerator)
 */
void divide_arrays_neon_Ex(volatile float* dst, volatile const float* src1, volatile const float* src2, size_t n, float limit);

// ============================================================================
// Array Arithmetic Functions - Double
// ============================================================================

/**
 * @brief Add two double arrays: dst[i] = src1[i] + src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 */
void add_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n);

/**
 * @brief Subtract two double arrays: dst[i] = src1[i] - src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 */
void subtract_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n);

/**
 * @brief Multiply two double arrays: dst[i] = src1[i] * src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 */
void multiply_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n);

/**
 * @brief Divide two double arrays: dst[i] = src1[i] / src2[i]
 * @param dst Destination array
 * @param src1 Numerator array
 * @param src2 Denominator array
 * @param n Number of elements
 * @note Uses VDIV.F64 (VFP instruction, serialized and slow)
 */
void divide_arrays_double_neon(volatile double* dst, volatile const double* src1, volatile const double* src2, size_t n);

// ============================================================================
// Array Arithmetic Functions - Integer (32-bit)
// ============================================================================

/**
 * @brief Add two int arrays: dst[i] = src1[i] + src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 */
void add_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n);

/**
 * @brief Subtract two int arrays: dst[i] = src1[i] - src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 */
void subtract_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n);

/**
 * @brief Multiply two int arrays: dst[i] = src1[i] * src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 * @note Result is truncated to 32 bits (wraparound semantics)
 */
void multiply_arrays_int_neon(volatile int* dst, volatile const int* src1, volatile const int* src2, size_t n);

// ============================================================================
// Array Arithmetic Functions - Int16
// ============================================================================

/**
 * @brief Add two int16 arrays: dst[i] = src1[i] + src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 */
void add_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n);

/**
 * @brief Subtract two int16 arrays: dst[i] = src1[i] - src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 */
void subtract_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n);

/**
 * @brief Multiply two int16 arrays: dst[i] = src1[i] * src2[i]
 * @param dst Destination array
 * @param src1 First source array
 * @param src2 Second source array
 * @param n Number of elements
 * @note Result is truncated to 16 bits (wraparound semantics)
 */
void multiply_arrays_int16_neon(volatile int16_t* dst, volatile const int16_t* src1, volatile const int16_t* src2, size_t n);

// ============================================================================
// Scalar-Array Operations - Float
// ============================================================================

/**
 * @brief Add scalar to float array: dst[i] = src[i] + scalar
 * @param dst Destination array
 * @param src Source array
 * @param scalar Scalar value to add
 * @param n Number of elements
 */
void add_scalar_to_array_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n);

/**
 * @brief Subtract scalar from float array: dst[i] = src[i] - scalar
 * @param dst Destination array
 * @param src Source array
 * @param scalar Scalar value to subtract
 * @param n Number of elements
 */
void subtract_scalar_from_array_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n);

/**
 * @brief Multiply float array by scalar: dst[i] = src[i] * scalar
 * @param dst Destination array
 * @param src Source array
 * @param scalar Scalar value to multiply
 * @param n Number of elements
 */
void multiply_array_by_scalar_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n);

/**
 * @brief Divide float array by scalar: dst[i] = src[i] / scalar
 * @param dst Destination array
 * @param src Source array
 * @param scalar Scalar value to divide by
 * @param n Number of elements
 * @note Uses reciprocal approximation for speed
 */
void divide_array_by_scalar_float_neon(volatile float* dst, volatile const float* src, const float scalar, size_t n);

// ============================================================================
// Scalar-Array Operations - Double
// ============================================================================

/**
 * @brief Add scalar to double array: dst[i] = src[i] + scalar
 * @param dst Destination array
 * @param src Source array
 * @param scalar Scalar value to add
 * @param n Number of elements
 */
void add_scalar_to_array_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n);

/**
 * @brief Subtract scalar from double array: dst[i] = src[i] - scalar
 * @param dst Destination array
 * @param src Source array
 * @param scalar Scalar value to subtract
 * @param n Number of elements
 */
void subtract_scalar_from_array_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n);

/**
 * @brief Multiply double array by scalar: dst[i] = src[i] * scalar
 * @param dst Destination array
 * @param src Source array
 * @param scalar Scalar value to multiply
 * @param n Number of elements
 */
void multiply_array_by_scalar_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n);

/**
 * @brief Divide double array by scalar: dst[i] = src[i] / scalar
 * @param dst Destination array
 * @param src Source array
 * @param scalar Scalar value to divide by
 * @param n Number of elements
 * @note Uses VDIV.F64 (VFP instruction, serialized and slow)
 */
void divide_array_by_scalar_double_neon(volatile double* dst, volatile const double* src, const double scalar, size_t n);

#ifdef __cplusplus
}
#endif

#endif  // __RP_MATH_H__