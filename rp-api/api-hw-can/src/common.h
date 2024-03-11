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

#define __SHORT_FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define FATAL(X)  {fprintf(stderr, "Error at line %d, file %s errno %d [%s] %s\n", __LINE__, __SHORT_FILENAME__, errno, strerror(errno),X); exit(1);}
#define WARNING(...) { char error_msg[512]; snprintf(error_msg,512,__VA_ARGS__);fprintf(stderr,"[W] %s:%d %s\n",__SHORT_FILENAME__,__LINE__,error_msg);}

#ifdef TRACE_ENABLE
#define TRACE(...) { char error_msg[512]; snprintf(error_msg,512,__VA_ARGS__);fprintf(stderr,"[T] %s:%d %s\n",__SHORT_FILENAME__,__LINE__,error_msg);}
#define TRACE_SHORT(...) { char error_msg[512]; snprintf(error_msg,512,__VA_ARGS__);fprintf(stderr,"[T] %s\n",error_msg);}
#else
#define TRACE(...)
#define TRACE_SHORT(...)
#endif

#define ECHECK_APP_RP(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        WARNING("Runtime error: %s returned \"%s\"", #x, rp_GetError(retval)); \
        return retval; \
    } \
}


#define ECHECK_APP_NO_RET_RP(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        WARNING("Runtime error: %s returned \"%s\"", #x, rp_GetError(retval)); \
    } \
}

#define ECHECK_APP(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        WARNING("Runtime error: %s returned \"%s\"", #x, rp_CanGetError(retval)); \
        return retval; \
    } \
}


#define ECHECK_APP_NO_RET(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        WARNING("Runtime error: %s returned \"%s\"", #x, rp_CanGetError(retval)); \
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
