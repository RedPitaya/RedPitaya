/**
 * $Id: $
 *
 * @brief Red Pitaya library common module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef COMMON_H_
#define COMMON_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvolatile"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <cstddef>
#include "rp.h"
#include "rp_hw_calib.h"

#define CHANNEL_ACTION(CHANNEL, CHANNEL_1_ACTION, CHANNEL_2_ACTION) \
    if ((CHANNEL) == RP_CH_1) {                                     \
        CHANNEL_1_ACTION;                                           \
    } else if ((CHANNEL) == RP_CH_2) {                              \
        CHANNEL_2_ACTION;                                           \
    } else {                                                        \
        return RP_EPN;                                              \
    }

#define CHANNEL_ACTION_4CH(CHANNEL, CHANNEL_1_ACTION, CHANNEL_2_ACTION, CHANNEL_3_ACTION, CHANNEL_4_ACTION) \
    if ((CHANNEL) == RP_CH_1) {                                                                             \
        CHANNEL_1_ACTION;                                                                                   \
    } else if ((CHANNEL) == RP_CH_2) {                                                                      \
        CHANNEL_2_ACTION;                                                                                   \
    } else if ((CHANNEL) == RP_CH_3) {                                                                      \
        CHANNEL_3_ACTION;                                                                                   \
    } else if ((CHANNEL) == RP_CH_4) {                                                                      \
        CHANNEL_4_ACTION;                                                                                   \
    } else {                                                                                                \
        return RP_EPN;                                                                                      \
    }

#define cmn_Debug(...)              \
    {                               \
        if (cmn_isEnableDebugReg()) \
            WARNING(__VA_ARGS__)    \
    }

#define printReg(X, Y, W, Z)                    \
    {                                           \
        auto tmp = Z;                           \
        printf(X, Y, 0x40000000 + W, tmp, tmp); \
    }
#define printRegBit(X, Y, Z)    \
    {                           \
        auto tmp = Z;           \
        printf(X, Y, tmp, tmp); \
    }

// unmasked IO read/write (p - pointer, v - value)
#define ioread32(p) (*(volatile uint32_t*)(p))
#define iowrite32(v, p) (*(volatile uint32_t*)(p) = (v))

#define SET_BITS(x, b) ((x) |= (b))
#define UNSET_BITS(x, b) ((x) &= ~(b))
#define SET_VALUE(x, b) ((x) = (b))
#define ARE_BITS_SET(x, b) (((x) & (b)) == (b))

#define VALIDATE_BITS(b, m)    \
    {                          \
        if (((b) & ~(m)) != 0) \
            return RP_EOOR;    \
    }

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define FLOAT_EPS 0.00001f

#define FULL_SCALE_NORM 20.0  // V

int cmn_Init();
int cmn_Release();

// void cmn_DebugReg(const char* msg, uint32_t value);
// void cmn_DebugRegCh(const char* msg, int ch, uint32_t value);
void cmn_enableDebugReg();
int cmn_isEnableDebugReg();

int cmn_Map(size_t size, size_t offset, void** mapped);
int cmn_Unmap(size_t size, void** mapped);

int cmn_InitMap(size_t size, size_t offset, void** mapped, int* fd);
int cmn_ReleaseClose(int fd, size_t size, void** mapped);

int cmn_SetBits(volatile uint32_t* field, uint32_t bits, uint32_t mask);
int cmn_UnsetBits(volatile uint32_t* field, uint32_t bits, uint32_t mask);
int cmn_SetValue(volatile uint32_t* field, uint32_t value, uint32_t mask, uint32_t* settedValue);
int cmn_SetShiftedValue(volatile uint32_t* field, uint32_t value, uint32_t mask, uint32_t bitsToSet, uint32_t* settedValue);
int cmn_GetValue(volatile uint32_t* field, uint32_t* value, uint32_t mask);
int cmn_GetShiftedValue(volatile uint32_t* field, uint32_t* value, uint32_t mask, uint32_t bitsToSetShift);
int cmn_AreBitsSet(volatile uint32_t field, uint32_t bits, uint32_t mask, bool* result);

int cmn_GetReservedMemory(uint32_t* _startAddress, uint32_t* _size);

// int intcmp(const void *a, const void *b);
// int int16cmp(const void *aa, const void *bb);
// int floatCmp(const void *a, const void *b);

rp_channel_calib_t convertCh(rp_channel_t ch);
rp_channel_t convertChFromIndex(uint8_t index);
rp_channel_calib_t convertPINCh(rp_apin_t pin);
rp_acq_ac_dc_mode_calib_t convertPower(rp_acq_ac_dc_mode_t ch);

#pragma GCC diagnostic pop

#endif /* COMMON_H_ */
