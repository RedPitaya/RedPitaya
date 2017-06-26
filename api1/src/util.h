#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

typedef struct {uint8_t s; uint8_t m; uint8_t f;} fixp_t;

// number of possible values
#define fixp_num(v)          (1<<(v.s+v.m+v.f))
// fixed point max/min value  (signed : unsigned)
#define fixp_max(v)        (+(1<<(    v.m+v.f))-1)
#define fixp_min(v) (v.s ? (-(1<<(    v.m+v.f))  ) : 0)
// fixed point unit value
#define fixp_unit(v)         (1<<(        v.f))
#define fixp_bits(v)             (v.s+v.m+v.f)

#endif

