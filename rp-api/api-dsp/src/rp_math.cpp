#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"

#include <math.h>
#include "rp_math.h"

const float __log10f_rng =  0.3010299957f;

const float __log10f_lut[8] = {
	-0.99697286229624, 		//p0
	-1.07301643912502, 		//p4
	-2.46980061535534, 		//p2
	-0.07176870463131, 		//p6
	2.247870219989470, 		//p1
	0.366547581117400, 		//p5
	1.991005185100089, 		//p3
	0.006135635201050,		//p7
};


const float __coshf_rng[2] = {
        1.442695041f,
        0.693147180f
};

const float __coshf_lut[16] = {
        0.00019578093328483123, //p7
        0.00019578093328483123, //p7
        0.0014122663401803872,  //p6
        0.0014122663401803872,  //p6
        0.008336936973260111,   //p5
        0.008336936973260111,   //p5
        0.04165989275009526,    //p4
        0.04165989275009526,    //p4
        0.16666570253074878,    //p3
        0.16666570253074878,    //p3
        0.5000006143673624,     //p2
        0.5000006143673624,     //p2
        1.000000059694879,              //p1
        1.000000059694879,              //p1
        0.9999999916728642,             //p0
        0.9999999916728642              //p0
};

const float __sinhf_rng[2] = {
        1.442695041f,
        0.693147180f
};

const float __sinhf_lut[16] = {
        0.00019578093328483123, //p7
        0.00019578093328483123, //p7
        0.0014122663401803872,  //p6
        0.0014122663401803872,  //p6
        0.008336936973260111,   //p5
        0.008336936973260111,   //p5
        0.04165989275009526,    //p4
        0.04165989275009526,    //p4
        0.16666570253074878,    //p3
        0.16666570253074878,    //p3
        0.5000006143673624,     //p2
        0.5000006143673624,     //p2
        1.000000059694879,              //p1
        1.000000059694879,              //p1
        0.9999999916728642,             //p0
        0.9999999916728642              //p0
};


float log10f_neon(float x)
{
#ifdef ARCH_ARM	
	asm volatile (
	
	"vdup.f32		d0, d0[0]				\n\t"	//d0 = {x,x};
	
	//extract exponent
	"vmov.i32		d2, #127				\n\t"	//d2 = 127;
	"vshr.u32		d6, d0, #23				\n\t"	//d6 = d0 >> 23;
	"vsub.i32		d6, d6, d2				\n\t"	//d6 = d6 - d2;
	"vshl.u32		d1, d6, #23				\n\t"	//d1 = d6 << 23;
	"vsub.i32		d0, d0, d1				\n\t"	//d0 = d0 + d1;

	//polynomial:
	"vmul.f32 		d1, d0, d0				\n\t"	//d1 = d0*d0 = {x^2, x^2}	
	"vld1.32 		{d2, d3, d4, d5}, [%1]	\n\t"	//q1 = {p0, p4, p2, p6}, q2 = {p1, p5, p3, p7} ;
	"vmla.f32 		q1, q2, d0[0]			\n\t"	//q1 = q1 + q2 * d0[0]		
	"vmla.f32 		d2, d3, d1[0]			\n\t"	//d2 = d2 + d3 * d1[0]		
	"vmul.f32 		d1, d1, d1				\n\t"	//d1 = d1 * d1 = {x^4, x^4}	
	"vmla.f32 		d2, d1, d2[1]			\n\t"	//d2 = d2 + d1 * d2[1]		

	//add exponent 	
	"vdup.32 		d7, %0					\n\t"	//d7 = {rng, rng}
	"vcvt.f32.s32 	d6, d6					\n\t"	//d6 = (float) d6
	"vmla.f32 		d2, d6, d7				\n\t"	//d2 = d2 + d6 * d7		

	"vmov.f32 		s0, s4					\n\t"	//s0 = s4

	:: "r"(__log10f_rng), "r"(__log10f_lut) 
    : "d0", "d1", "q1", "q2", "d6", "d7"
	);
#else
	return log10f(x);
#endif
}

float cosf_neon(float x)
{
#ifdef ARCH_ARM
        asm volatile (
        "vdup.f32               d0, d0[0]                               \n\t"   //d0 = {x, x}   
        "fnegs                  s1, s1                                  \n\t"   //s1 = -s1

        //Range Reduction:
        "vld1.32                d2, [%0]                                \n\t"   //d2 = {invrange, range}
        "vld1.32                {d16, d17}, [%1]!               \n\t"   
        "vmul.f32               d6, d0, d2[0]                   \n\t"   //d6 = d0 * d2[0] 
        "vcvt.s32.f32   d6, d6                                  \n\t"   //d6 = (int) d6
        "vld1.32                {d18}, [%1]!                    \n\t"   
        "vcvt.f32.s32   d1, d6                                  \n\t"   //d1 = (float) d6
        "vld1.32                {d19}, [%1]!                    \n\t"   
        "vmls.f32               d0, d1, d2[1]                   \n\t"   //d0 = d0 - d1 * d2[1]
        "vld1.32                {d20}, [%1]!                    \n\t"   

        //polynomial:
        "vmla.f32               d17, d16, d0                    \n\t"   //d17 = d17 + d16 * d0; 
        "vld1.32                {d21}, [%1]!                    \n\t"   
        "vmla.f32               d18, d17, d0                    \n\t"   //d18 = d18 + d17 * d0; 
        "vld1.32                {d22}, [%1]!                    \n\t"   
        "vmla.f32               d19, d18, d0                    \n\t"   //d19 = d19 + d18 * d0; 
        "vld1.32                {d23}, [%1]!                    \n\t"   
        "vmla.f32               d20, d19, d0                    \n\t"   //d20 = d20 + d19 * d0; 
        "vmla.f32               d21, d20, d0                    \n\t"   //d21 = d21 + d20 * d0; 
        "vmla.f32               d22, d21, d0                    \n\t"   //d22 = d22 + d21 * d0; 
        "vmla.f32               d23, d22, d0                    \n\t"   //d23 = d23 + d22 * d0; 

        //multiply by 2 ^ m     
        "vshl.i32               d6, d6, #23                             \n\t"   //d6 = d6 << 23         
        "vadd.i32               d0, d23, d6                             \n\t"   //d0 = d22 + d6         

        "vdup.f32               d2, d0[1]                               \n\t"   //d2 = s1
        "vmov.f32               d1, #0.5                                \n\t"   //d1 = 0.5
        "vadd.f32               d0, d0, d2                              \n\t"   //d0 = d0 + d2          
        "vmul.f32               d0, d1                                  \n\t"   //d0 = d0 * d1          

        :: "r"(__coshf_rng), "r"(__coshf_lut) 
    : "d0", "d1", "q1", "q2", "d6"
        );

#else
	return cosf(x);
#endif
}


float sinf_neon(float x)
{
#ifdef ARCH_ARM
        asm volatile (
        "vdup.f32               d0, d0[0]                               \n\t"   //d0 = {x, x}   
        "fnegs                  s1, s1                                  \n\t"   //s1 = -s1

        //Range Reduction:
        "vld1.32                d2, [%0]                                \n\t"   //d2 = {invrange, range}
        "vld1.32                {d16, d17}, [%1]!               \n\t"   
        "vmul.f32               d6, d0, d2[0]                   \n\t"   //d6 = d0 * d2[0] 
        "vcvt.s32.f32   d6, d6                                  \n\t"   //d6 = (int) d6
        "vld1.32                {d18}, [%1]!                    \n\t"   
        "vcvt.f32.s32   d1, d6                                  \n\t"   //d1 = (float) d6
        "vld1.32                {d19}, [%1]!                    \n\t"   
        "vmls.f32               d0, d1, d2[1]                   \n\t"   //d0 = d0 - d1 * d2[1]
        "vld1.32                {d20}, [%1]!                    \n\t"   

        //polynomial:
        "vmla.f32               d17, d16, d0                    \n\t"   //d17 = d17 + d16 * d0; 
        "vld1.32                {d21}, [%1]!                    \n\t"   
        "vmla.f32               d18, d17, d0                    \n\t"   //d18 = d18 + d17 * d0; 
        "vld1.32                {d22}, [%1]!                    \n\t"   
        "vmla.f32               d19, d18, d0                    \n\t"   //d19 = d19 + d18 * d0; 
        "vld1.32                {d23}, [%1]!                    \n\t"   
        "vmla.f32               d20, d19, d0                    \n\t"   //d20 = d20 + d19 * d0; 
        "vmla.f32               d21, d20, d0                    \n\t"   //d21 = d21 + d20 * d0; 
        "vmla.f32               d22, d21, d0                    \n\t"   //d22 = d22 + d21 * d0; 
        "vmla.f32               d23, d22, d0                    \n\t"   //d23 = d23 + d22 * d0; 

        //multiply by 2 ^ m     
        "vshl.i32               d6, d6, #23                             \n\t"   //d6 = d6 << 23         
        "vadd.i32               d0, d23, d6                             \n\t"   //d0 = d22 + d6         

        "vdup.f32               d2, d0[1]                               \n\t"   //d2 = s1
        "vmov.f32               d1, #0.5                                \n\t"   //d1 = 0.5
        "vsub.f32               d0, d0, d2                              \n\t"   //d0 = d0 - d2          
        "vmul.f32               d0, d1                                  \n\t"   //d0 = d0 * d1          

        :: "r"(__sinhf_rng), "r"(__sinhf_lut) 
    : "d0", "d1", "q1", "q2", "d6"
        );

#else
	return sinf(x);
#endif
}

float sqrtf_neon(float x)
{
#ifdef ARCH_ARM
	asm volatile (
		
	//fast invsqrt approx
	"vmov.f32 		d1, d0					\n\t"	//d1 = d0
	"vrsqrte.f32 	d0, d0					\n\t"	//d0 = ~ 1.0 / sqrt(d0)
	"vmul.f32 		d2, d0, d1				\n\t"	//d2 = d0 * d1
	"vrsqrts.f32 	d3, d2, d0				\n\t"	//d3 = (3 - d0 * d2) / 2 	
	"vmul.f32 		d0, d0, d3				\n\t"	//d0 = d0 * d3
	"vmul.f32 		d2, d0, d1				\n\t"	//d2 = d0 * d1	
	"vrsqrts.f32 	d3, d2, d0				\n\t"	//d4 = (3 - d0 * d3) / 2	
	"vmul.f32 		d0, d0, d3				\n\t"	//d0 = d0 * d3	
		
	//fast reciporical approximation
	"vrecpe.f32		d1, d0					\n\t"	//d1 = ~ 1 / d0; 
	"vrecps.f32		d2, d1, d0				\n\t"	//d2 = 2.0 - d1 * d0; 
	"vmul.f32		d1, d1, d2				\n\t"	//d1 = d1 * d2; 
	"vrecps.f32		d2, d1, d0				\n\t"	//d2 = 2.0 - d1 * d0; 
	"vmul.f32		d0, d1, d2				\n\t"	//d0 = d1 * d2; 

	::: "d0", "d1", "d2", "d3"
	);
#else
	return sqrtf(x);        
#endif
}

#pragma GCC diagnostic pop