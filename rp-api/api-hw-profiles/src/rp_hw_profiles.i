%module rp_hw_profiles

%include <typemaps.i>
%include <cstring.i>

%apply int *OUTPUT { rp_HPeModels_t *_out_value};
%apply int *OUTPUT { rp_HPeZynqModels_t *_out_value};
%apply float *OUTPUT { float *_out_value };
%apply unsigned int *OUTPUT { uint32_t *_out_value };
%apply unsigned char *OUTPUT { uint8_t *_out_value };
%apply bool *OUTPUT { bool *_out_value };
%apply unsigned char { uint8_t }
%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }

// This input typemap declares that char** requires no input parameter.
// Instead, the address of a local char* is used to call the function.
%typemap(in,numinputs=0) char** (char* tmp) %{
    $1 = &tmp;
%}

// After the function is called, the char** parameter contains a malloc'ed char* pointer.
// Construct a Python Unicode object (I'm using Python 3) and append it to
// any existing return value for the wrapper.
%typemap(argout) char** (PyObject* obj) %{
    obj = PyUnicode_FromString(*$1);
    $result = SWIG_Python_AppendOutput($result,obj);
%}


%{
/* Includes the header in the wrapper code */
#include "rp_hw-profiles.h"

%}

/* Parse the header file to generate wrappers */
%include "rp_hw-profiles.h"