#pragma GCC diagnostic ignored "-Wunused-function"

#include <string.h>
#include "neon_asm.h"

void memcpy_neon(volatile void *dst, volatile const void *src, size_t n)
{

#ifdef ARCH_ARM
    if ((n % 64) || n < 64){
        memcpy((void*)dst,(void*)src,n);
        //std::cout << "Warning: Non-optimal neon copy\n";
        return;
    }
asm volatile (
    "NEONCopyPLD%=:\n"
    "    PLD [%[src], #0xC0]\n"
    "    VLDM %[src]!,{d0-d7}\n"
    "    VSTM %[dst]!,{d0-d7}\n"
    "    SUBS %[n],%[n],#0x40\n"
    "    BGT NEONCopyPLD%=\n"
    : [dst]"+r"(dst), [src]"+r"(src), [n]"+r"(n) : : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
#else
    memcpy((void*)dst,(void*)src,n);
#endif // ARCH_ARM
}
