%module rp_hw_calib

%include <typemaps.i>
%include <cstring.i>

%apply unsigned int { rp_HPeModels_t }

%apply float *OUTPUT { float *_out_value };
%apply unsigned int *OUTPUT { uint32_t *_out_value };
%apply unsigned char *OUTPUT { uint8_t *_out_value };
%apply bool *OUTPUT { bool *_out_value };
%apply unsigned char { uint8_t }
%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }

#define __attribute__(x) 

%{
/* Includes the header in the wrapper code */
#include "rp_hw-calib.h"

%}

/* Parse the header file to generate wrappers */
%include "rp_hw-calib.h"