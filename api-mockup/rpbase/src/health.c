/**
 * $Id: $
 *
 * @brief Red Pitaya library Health module interface
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
#include "health.h"

// Base Health address
static const int HEALTH_BASE_ADDR = 0x40400000;
static const int HEALTH_BASE_SIZE = 0x4C;

typedef struct health_control_s {
    uint32_t unused[12];
    uint32_t temperature;
    uint32_t vccPint;
    uint32_t vccPaux;
    uint32_t vccBram;
    uint32_t vccInt;
    uint32_t vccAux;
    uint32_t vccDddr;
} health_control_t;

static const int HEALTH_MASK = 0xFFF;

static volatile health_control_t *health = NULL;


int health_Init()
{
    ECHECK(cmn_Init());
    ECHECK(cmn_Map(HEALTH_BASE_SIZE, HEALTH_BASE_ADDR, (void**)&health));
    return RP_OK;
}

int health_Release()
{
    ECHECK(cmn_Unmap(HEALTH_BASE_SIZE, (void**)&health));
    ECHECK(cmn_Release());
    return RP_OK;
}

int health_GetTemperature(float* value)
{
    uint32_t valueTmp;
    int result = cmn_GetValue(&health->temperature, &valueTmp, HEALTH_MASK);
    // Form documentation
    *value = (float) (((float)valueTmp  * 503.975 / 4095.0) - 273.15);
    return result;
}

int health_GetVccPint(float* value)
{
    uint32_t valueTmp;
    int result = cmn_GetValue(&health->vccPint, &valueTmp, HEALTH_MASK);
    *value = (float) ((float)valueTmp  / 0xFFF * 3.0);
    return result;
}

int health_GetVccPaux(float* value)
{
    uint32_t valueTmp;
    int result = cmn_GetValue(&health->vccPaux, &valueTmp, HEALTH_MASK);
    *value = (float) ((float)valueTmp  / 0xFFF * 3.0);
    return result;
}

int health_GetVccBram(float* value)
{
    uint32_t valueTmp;
    int result = cmn_GetValue(&health->vccBram, &valueTmp, HEALTH_MASK);
    *value = (float) ((float)valueTmp  / 0xFFF * 3.0);
    return result;
}

int health_GetVccInt(float* value)
{
    uint32_t valueTmp;
    int result = cmn_GetValue(&health->vccInt, &valueTmp, HEALTH_MASK);
    *value = (float) ((float)valueTmp  / 0xFFF * 3.0);
    return result;
}

int health_GetVccAux(float* value)
{
    uint32_t valueTmp;
    int result = cmn_GetValue(&health->vccAux, &valueTmp, HEALTH_MASK);
    *value = (float) ((float)valueTmp  / 0xFFF * 3.0);
    return result;
}

int health_GetVccDdr(float* value)
{
    uint32_t valueTmp;
    int result = cmn_GetValue(&health->vccDddr, &valueTmp, HEALTH_MASK);
    *value = (float) ((float)valueTmp  / 0xFFF * 3.0);
    return result;
}
