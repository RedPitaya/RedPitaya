/**
 * $Id: $
 *
 * @brief Red Pitaya Applications library common module interface
 *
 * @Author Luka Golinar, RedPitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

 #ifndef UTILS_H_
 #define UTILS_H_


#include <sys/syslog.h>
#include "rp.h"
#include "lcrApp.h"

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

#define ECHECK_LCR_APP(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        WARNING("Runtime error: %s returned \"%s\"", #x, lcrApp_LcrGetError(static_cast<lcr_error_t>(retval))); \
        return retval; \
    } \
}


#define ECHECK_LCR_APP_NO_RET(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        WARNING("Runtime error: %s returned \"%s\"", #x, lcrApp_LcrGetError(static_cast<lcr_error_t>(retval))); \
    } \
}

#define ECHECK_APP(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        WARNING("Runtime error: %s returned \"%s\"", #x, rp_GetError(retval)); \
        return retval; \
    } \
}


void lcr_getDecimationValue(float frequency,int *dec_val,uint32_t adc_rate);

#endif //UTILS_H_
