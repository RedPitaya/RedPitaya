%module rp_sweep

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>

%apply int { rp_gen_sweep_mode_t }
%apply int { rp_gen_sweep_dir_t }

%apply bool *OUTPUT { bool *state };
%apply float *OUTPUT { float *_freq };
%apply int *OUTPUT { int *_us };
%apply float *OUTPUT { float *freq };
%apply int *OUTPUT { int *us };
%apply int *OUTPUT { rp_gen_sweep_mode_t *_mode };
%apply int *OUTPUT { rp_gen_sweep_dir_t *_dir };
%apply int *OUTPUT { rp_gen_sweep_mode_t *mode };
%apply int *OUTPUT { rp_gen_sweep_dir_t *dir };

%inline %{
typedef double *double_ptr;
typedef float *float_ptr;
%}

%{
/* Includes the header in the wrapper code */
#include "rp_enums.h"
#include "rp_sweep.h"
%}

%array_class(double, arrDouble);
%array_class(float, arrFloat);
%array_class(double_ptr, arrpDouble);
%array_class(float_ptr, arrpFloat);


/* Parse the header file to generate wrappers */
%include "rp_enums.h"
%include "rp_sweep.h"
