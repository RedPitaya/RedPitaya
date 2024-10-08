/**
 * $Id: $
 *
 * @brief Red Pitaya Applications library common module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

/* @brief ADC buffer size is 16 k samples. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include "rp.h"
#include "rp_hw_can.h"


#define ECHECK_APP(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        ERROR_LOG("%s returned \"%s\"", #x, rp_CanGetError(retval)); \
        return retval; \
    } \
}


#define ECHECK_APP_NO_RET(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        ERROR_LOG("%s returned \"%s\"", #x, rp_CanGetError(retval)); \
    } \
}

inline const char* getInterfaceName(rp_can_interface_t _interface){
    switch(_interface){
        case RP_CAN_0: return "can0";
        case RP_CAN_1: return "can1";
        default: ;
    }
    return "";
}

#endif /* _COMMON_H_ */
