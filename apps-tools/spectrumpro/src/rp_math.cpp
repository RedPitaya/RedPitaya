#include "rp_math.h"
#include <math.h>


const float __log10f_rng = 0.3010299957f;

const float __log10f_lut[8] = {
    -0.99697286229624,  //p0
    -1.07301643912502,  //p4
    -2.46980061535534,  //p2
    -0.07176870463131,  //p6
    2.247870219989470,  //p1
    0.366547581117400,  //p5
    1.991005185100089,  //p3
    0.006135635201050,  //p7
};

float log10f_neon(float x) {
#ifdef ARCH_ARM
    float dest = 0;
    asm volatile(

        "vdup.f32		d0, d0[0]				\n\t"  //d0 = {x,x};

        //extract exponent
        "vmov.i32		d2, #127				\n\t"  //d2 = 127;
        "vshr.u32		d6, d0, #23				\n\t"  //d6 = d0 >> 23;
        "vsub.i32		d6, d6, d2				\n\t"  //d6 = d6 - d2;
        "vshl.u32		d1, d6, #23				\n\t"  //d1 = d6 << 23;
        "vsub.i32		d0, d0, d1				\n\t"  //d0 = d0 + d1;

        //polynomial:
        "vmul.f32 		d1, d0, d0				\n\t"  //d1 = d0*d0 = {x^2, x^2}
        "vld1.32 		{d2, d3, d4, d5}, [%2]	\n\t"  //q1 = {p0, p4, p2, p6}, q2 = {p1, p5, p3, p7} ;
        "vmla.f32 		q1, q2, d0[0]			\n\t"  //q1 = q1 + q2 * d0[0]
        "vmla.f32 		d2, d3, d1[0]			\n\t"  //d2 = d2 + d3 * d1[0]
        "vmul.f32 		d1, d1, d1				\n\t"  //d1 = d1 * d1 = {x^4, x^4}
        "vmla.f32 		d2, d1, d2[1]			\n\t"  //d2 = d2 + d1 * d2[1]

        //add exponent
        "vdup.32 		d7, %1					\n\t"  //d7 = {rng, rng}
        "vcvt.f32.s32 	d6, d6					\n\t"  //d6 = (float) d6
        "vmla.f32 		d2, d6, d7				\n\t"  //d2 = d2 + d6 * d7

        "vmov.f32 		%0, s4	        		\n\t"  //s0 = s4

        : "r="(dest)
        : "r"(__log10f_rng), "r"(__log10f_lut)
        : "d0", "d1", "q1", "q2", "d6", "d7");
    return dest;
#else
    return log10f(x);
#endif
}

float sqrtf_neon(float x) {
#ifdef ARCH_ARM
    float dest = 0;
    asm volatile(

        //fast invsqrt approx
        "vmov.f32 		d1, d0					\n\t"  //d1 = d0
        "vrsqrte.f32 	d0, d0					\n\t"  //d0 = ~ 1.0 / sqrt(d0)
        "vmul.f32 		d2, d0, d1				\n\t"  //d2 = d0 * d1
        "vrsqrts.f32 	d3, d2, d0				\n\t"  //d3 = (3 - d0 * d2) / 2
        "vmul.f32 		d0, d0, d3				\n\t"  //d0 = d0 * d3
        "vmul.f32 		d2, d0, d1				\n\t"  //d2 = d0 * d1
        "vrsqrts.f32 	d3, d2, d0				\n\t"  //d4 = (3 - d0 * d3) / 2
        "vmul.f32 		d0, d0, d3				\n\t"  //d0 = d0 * d3

        //fast reciporical approximation
        "vrecpe.f32		d1, d0					\n\t"          //d1 = ~ 1 / d0;
        "vrecps.f32		d2, d1, d0				\n\t"          //d2 = 2.0 - d1 * d0;
        "vmul.f32		d1, d1, d2				\n\t"          //d1 = d1 * d2;
        "vrecps.f32		d2, d1, d0				\n\t"          //d2 = 2.0 - d1 * d0;
        "vmul.f32		d0, d1, d2				\n\t"          //d0 = d1 * d2;
        "vmov.f32 		%0, s0	        		        \n\t"  //s0 = s4
        : "r="(dest)::"d0", "d1", "d2", "d3");
    return dest;
#else
    return sqrtf(x);
#endif
}
