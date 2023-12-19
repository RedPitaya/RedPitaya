%module rp_arb

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>
%include <std_string.i>

%apply int { rp_channel_t }


%apply float *INOUT { float *_data };
%apply unsigned int *OUTPUT { uint32_t *_size };
%apply unsigned int *OUTPUT { uint32_t *_count };
%apply bool *OUTPUT { bool *_valid };
%apply std::string *OUTPUT { std::string *_name };
%apply std::string *OUTPUT { std::string *_fileName };

%inline %{
    typedef uint8_t *u8_ptr;
    typedef uint16_t *u16_ptr;
    typedef float *float_ptr;
    typedef double *double_ptr;
%}


%{
/* Includes the header in the wrapper code */
#include "rp_arb.h"
%}

%array_class(uint8_t, arrUInt8);
%array_class(uint16_t, arrUInt16);
%array_class(float, arrFloat);
%array_class(double, arrDouble);

%array_class(u8_ptr, arrpUInt8);
%array_class(u16_ptr, arrpUInt16);
%array_class(float_ptr, arrpFloat);
%array_class(double_ptr, arrpDouble);

/* Parse the header file to generate wrappers */
%include "rp_arb.h"
