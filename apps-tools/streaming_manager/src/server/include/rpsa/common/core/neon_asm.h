//
// Created by user on 03.04.19.
//
#pragma once
#pragma GCC diagnostic ignored "-Wunused-function"
#include <iostream>

namespace {
    static void memcpy_neon(volatile void *dst, volatile const void *src, size_t n) noexcept
    {

#ifdef ARCH_ARM
        if (n & 63) {
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

    static void memcpy_stride_8bit_neon(volatile void *dst, volatile const void *src, size_t n) noexcept
    {
#ifdef ARCH_ARM
        if (n & 63) {
            memcpy((void*)dst,(void*)src,n);
            //std::cout << "Warning: Non-optimal neon copy\n";
        return;
    }

    asm volatile (
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
        : [dst]"+r"(dst), [src]"+r"(src), [n]"+r"(n) : : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
#else
        memcpy((void*)dst,(void*)src,n);
#endif
    }
}
