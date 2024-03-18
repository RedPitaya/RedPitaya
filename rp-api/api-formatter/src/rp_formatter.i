%module rp_formatter

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>
%include <std_string.i>

%apply int { rp_mode_t }
%apply int { rp_endianness_t }
%apply int { rp_bits_t }
%apply int { rp_channel_t }

%inline %{
    typedef uint8_t *u8_ptr;
    typedef uint16_t *u16_ptr;
    typedef float *float_ptr;
    typedef double *double_ptr;
%}


%{
#define SWIG_FILE_WITH_INIT
/* Includes the header in the wrapper code */
#include "rp_formatter.h"
%}

%include "numpy.i"

%init %{
import_array();
%}


%array_class(uint8_t, arrUInt8);
%array_class(uint16_t, arrUInt16);
%array_class(float, arrFloat);
%array_class(double, arrDouble);

%array_class(u8_ptr, arrpUInt8);
%array_class(u16_ptr, arrpUInt16);
%array_class(float_ptr, arrpFloat);
%array_class(double_ptr, arrpDouble);

%numpy_typemaps(uint8_t,    NPY_UINT16   ,unsigned short)
%numpy_typemaps(uint16_t,    NPY_UINT32   , unsigned int)

%apply (uint8_t* IN_ARRAY1, int DIM1) {(uint8_t* _np_buffer, int _samplesCount)};
%apply (uint16_t* IN_ARRAY1, int DIM1) {(uint16_t* _np_buffer, int _samplesCount)};
%apply (float* IN_ARRAY1, int DIM1) {(float* _np_buffer, int _samplesCount)};
%apply (double* IN_ARRAY1, int DIM1) {(double* _np_buffer, int _samplesCount)};

/* Parse the header file to generate wrappers */
%include "rp_formatter.h"
