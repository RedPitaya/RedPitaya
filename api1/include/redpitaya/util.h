#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

typedef struct {uint8_t s; uint8_t m; uint8_t f;} fixp_t;

// number of possible values
#define fixp_num(v)          (INT64_C(1)<<(v.s+v.m+v.f))
// fixed point max/min value
#define fixp_max(v)        (+(INT64_C(1)<<(    v.m+v.f))-1)
#define fixp_min(v) (v.s ? (-(INT64_C(1)<<(    v.m+v.f))  ) : 0)
// fixed point unit value
#define fixp_unit(v)         (INT64_C(1)<<(        v.f))
// number of bits
#define fixp_bits(v)                      (v.s+v.m+v.f)
// bit mask
#define fixp_mask(v)       ( (INT64_C(1)<<(v.s+v.m+v.f))-1)

char * rp_util_fixp_print (fixp_t value);

#endif

