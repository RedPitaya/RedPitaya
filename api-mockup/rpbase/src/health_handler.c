/**
 * $Id: $
 *
 * @brief Red Pitaya library Health handler interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#include "common.h"
#include "health.h"
#include "health_handler.h"

int health_GetValue(rp_health_t sensor, float *value)
{
    switch (sensor)
    {
    case RP_TEMP_FPGA:
        return health_GetTemperature(value);
    case RP_VCC_AUX:
        return health_GetVccAux(value);
    case RP_VCC_BRAM:
        return health_GetVccBram(value);
    case RP_VCC_DDR:
        return health_GetVccDdr(value);
    case RP_VCC_INT:
        return health_GetVccInt(value);
    case RP_VCC_PAUX:
        return health_GetVccPaux(value);
    case RP_VCC_PINT:
        return health_GetVccPint(value);
    default:
        return RP_EPN;
    }
}
