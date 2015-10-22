/**
 * $Id: $
 *
 * @brief Red Pitaya library housekeeping module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#include <stdio.h>

#include "common.h"
#include "housekeeping.h"

static volatile housekeeping_control_t *hk = NULL;

/**
 * general
 */


int hk_Init()
{
//    ECHECK(cmn_Init());
    ECHECK(cmn_Map(HOUSEKEEPING_BASE_SIZE, HOUSEKEEPING_BASE_ADDR, (void**)&hk));
    return RP_OK;
}

int hk_Release()
{
    ECHECK(cmn_Unmap(HOUSEKEEPING_BASE_SIZE, (void**)&hk));
//    ECHECK(cmn_Release());
    return RP_OK;
}


/**
 * ID and DNA
 */

int hk_GetID(uint32_t *id)
{
    return cmn_GetValue(&hk->id, id, 0xffffffff);
}

int hk_GetDNA(uint64_t *dna)
{
    int msg;
    uint32_t dna_lo;
    uint32_t dna_hi;
    msg  = cmn_GetValue(&hk->dna_lo, &dna_lo, 0xffffffff);
    msg |= cmn_GetValue(&hk->dna_hi, &dna_hi, 0xffffffff);
    *dna = ((uint64_t) dna_hi << 32) | ((uint64_t) dna_lo);
    // TODO check if return message is constructed correctly
    return msg;
}


/**
 * led_control
 */

int hk_SetLedBits(uint32_t bits)
{
    return cmn_SetBits(&hk->led_control, bits, LED_CONTROL_MASK);
}

int hk_UnsetLedBits(uint32_t bits)
{
    return cmn_UnsetBits(&hk->led_control, bits, LED_CONTROL_MASK);
}

int hk_AreLedBitsSet(uint32_t bits, bool* result)
{
    return cmn_AreBitsSet(hk->led_control, bits, LED_CONTROL_MASK, result);
}


/**
 * ex_cd_p
 */

int hk_SetExCdPBits(uint32_t bits)
{
    return cmn_SetBits(&hk->ex_cd_p, bits, EX_CD_P_MASK);
}

int hk_UnsetExCdPBits(uint32_t bits)
{
    return cmn_UnsetBits(&hk->ex_cd_p, bits, EX_CD_P_MASK);
}

int hk_AreExCdPBitsSet(uint32_t bits, bool* result)
{
    return cmn_AreBitsSet(hk->ex_cd_p, bits, EX_CD_P_MASK, result);
}


/**
 * ex_cd_n
 */

int hk_SetExCdNBits(uint32_t bits)
{
    return cmn_SetBits(&hk->ex_cd_n, bits, EX_CD_N_MASK);
}

int hk_UnsetExCdNBits(uint32_t bits)
{
    return cmn_UnsetBits(&hk->ex_cd_n, bits, EX_CD_N_MASK);
}

int hk_AreExCdNBitsSet(uint32_t bits, bool* result)
{
    return cmn_AreBitsSet(hk->ex_cd_n, bits, EX_CD_N_MASK, result);
}


/**
 * ex_co_p
 */

int hk_SetExCoPBits(uint32_t bits)
{
    return cmn_SetBits(&hk->ex_co_p, bits, EX_CO_P_MASK);
}

int hk_UnsetExCoPBits(uint32_t bits)
{
    return cmn_UnsetBits(&hk->ex_co_p, bits, EX_CO_P_MASK);
}

int hk_AreExCoPBitsSet(uint32_t bits, bool* result)
{
    return cmn_AreBitsSet(hk->ex_co_p, bits, EX_CO_P_MASK, result);
}



/**
 * ex_co_n
 */

int hk_SetExCoNBits(uint32_t bits)
{
    return cmn_SetBits(&hk->ex_co_n, bits, EX_CO_N_MASK);
}

int hk_UnsetExCoNBits(uint32_t bits)
{
    return cmn_UnsetBits(&hk->ex_co_n, bits, EX_CO_N_MASK);
}

int hk_AreExCoNBitsSet(uint32_t bits, bool* result)
{
    return cmn_AreBitsSet(hk->ex_co_n, bits, EX_CO_N_MASK, result);
}



/**
 * ex_ci_p
 */

int hk_AreExCiPBitsSet(uint32_t bits, bool* result)
{
    return cmn_AreBitsSet(hk->ex_ci_p, bits, EX_CI_P_MASK, result);
}



/**
 * ex_ci_n
 */

int hk_AreExCiNBitsSet(uint32_t bits, bool* result)
{
    return cmn_AreBitsSet(hk->ex_ci_n, bits, EX_CI_N_MASK, result);
}

/**
 * Digital loop
 */

int hk_EnableDigitalLoop(bool enable) {
    if (enable) {
        return cmn_SetBits(&hk->digital_loop, 1, DIGITAL_LOOP_MASK);
    } else {
        return cmn_UnsetBits(&hk->digital_loop, 1, DIGITAL_LOOP_MASK);
    }
}
