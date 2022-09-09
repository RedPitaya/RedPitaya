#ifndef NEON_ASM_H_
#define NEON_ASM_H_

#ifdef __cplusplus
extern "C" {
#endif

void memcpy_neon(volatile void *dst, volatile const void *src, size_t n);

#ifdef __cplusplus
}
#endif

#endif