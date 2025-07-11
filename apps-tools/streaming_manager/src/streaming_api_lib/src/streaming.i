// clang-format off
%module(directors="1") streaming

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <std_string.i>
%include <std_vector.i>
%include <carrays.i>
%include <cpointer.i>

%{
#include "callbacks.h"
#include "adc_streaming.h"
#include "dac_streaming.h"
%}


%feature("director") ADCCallback;
%feature("director") DACCallback;


%template(StringVector) std::vector<std::string>;
%template(Int16Vector) std::vector<int16_t>;
%template(Int8Vector) std::vector<int8_t>;

%typemap(out) std::vector<int16_t>* %{
    $result = PyList_New($1->size()); // Create outer Python list of correct size
    for(size_t i = 0; i < $1->size(); ++i)
    {
        PyList_SET_ITEM($result,i,PyLong_FromLong((*$1)[i]));
    }
%}

/* Parse the header file to generate wrappers */
%include "adc_streaming.h"
%include "dac_streaming.h"
%include "callbacks.h"
