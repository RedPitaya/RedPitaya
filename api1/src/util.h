#ifndef UTIL_H
#define UTIL_H

// number of possible values
#define fixp_num(bits)  ( (1<<(bits  ))  )
// signed fixed point
#define fixp_smax(bits)  ( (1<<(bits-1))-1)
#define fixp_smin(bits)  (-(1<<(bits-1))  )
// unsigned fixed point
#define fixp_umax(bits)  ( (1<<(bits  ))-1)
#define fixp_umin(bits)  (0)

#endif

