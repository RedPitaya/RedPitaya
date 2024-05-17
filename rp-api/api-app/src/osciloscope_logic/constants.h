#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#include <stdint.h>


#define MAX_VIEW_CHANNELS (RPAPP_OSC_SOUR_MATH + 1)

#define VIEW_SIZE_DEFAULT             1024
#define VIEW_SIZE_MAX                 1024 * 8

#define DIVISIONS_COUNT_X             10
#define DIVISIONS_COUNT_Y             10

typedef uint16_t vsize_t;

#define PERIOD_EXISTS_MIN_THRESHOLD       0.75  // ratio
#define PERIOD_EXISTS_MAX_THRESHOLD       0.92  // ratio
#define PERIOD_EXISTS_PEAK_THRESHOLD      0.99  // ratio


#define CONTIOUS_MODE_SCALE_THRESHOLD 50 // ms
#define WAIT_TO_FILL_BUF_TIMEOUT      500.f //(2*CLOCKS_PER_SEC)

#endif // __CONSTANTS_H