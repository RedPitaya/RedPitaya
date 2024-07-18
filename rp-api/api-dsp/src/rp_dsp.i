%module rp_dsp

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>

%apply int { window_mode_t }
%apply int { mode_t }

%apply uint32_t *OUTPUT { uint32_t *value };

%inline %{
typedef double *double_ptr;
typedef float *float_ptr;
%}

%{
/* Includes the header in the wrapper code */
#include "rp_dsp.h"
#include "rp_math.h"
%}

%array_class(double, arrDouble);
%array_class(float, arrFloat);
%array_class(double_ptr, arrpDouble);
%array_class(float_ptr, arrpFloat);


/* Parse the header file to generate wrappers */
%include "rp_dsp.h"
%include "rp_math.h"