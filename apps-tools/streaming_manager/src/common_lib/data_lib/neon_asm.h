#ifndef DATA_LIB_NEON_H
#define DATA_LIB_NEON_H

#include <cstring>

void memcpy_neon(volatile void* dst, volatile const void* src, size_t n) noexcept;
void memcpy_stride_8bit_neon(volatile void* dst, volatile const void* src, size_t n) noexcept;

#endif
