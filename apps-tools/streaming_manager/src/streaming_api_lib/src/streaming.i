// clang-format off
%module(directors="1") streaming

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <std_string.i>
%include <std_array.i>
%include <std_vector.i>
%include <std_list.i>
%include <carrays.i>
%include <cpointer.i>
%include <std_shared_ptr.i>

%{
#include "callbacks.h"
#include "adc_streaming.h"
#include "dac_streaming.h"
#include "config_streaming.h"
%}


%feature("director") ADCCallback;
%feature("director") DACCallback;
%feature("director") ConfigCallback;

%shared_ptr(ConfigCallback)
%shared_ptr(DACCallback)
%shared_ptr(ADCCallback)
%shared_ptr(ConfigStreamClient)

%template(StringVector) std::vector<std::string>;
%template(Int16Vector) std::vector<int16_t>;
%template(Int8Vector) std::vector<int8_t>;
%template(StringList) std::list<std::string>;
%template(BoolArray4) std::array<bool, 4>;

%typemap(out) std::vector<int16_t>* %{
    if (!$1) {
        Py_RETURN_NONE;
    }
    $result = PyList_New($1->size());
    if (!$result) {
        return NULL;
    }
    for(size_t i = 0; i < $1->size(); ++i)
    {
        PyObject* item = PyLong_FromLongLong((*$1)[i]);
        if (!item) {
            Py_DECREF($result);
            return NULL;
        }
        PyList_SET_ITEM($result, i, item);
    }
%}

// Handle ch1: Use the 'size' argument for the length
%typemap(directorin) int8_t* ch1_8Bit {
    if ($1 != NULL) {
        $input = PyMemoryView_FromMemory((char*)$1, size, PyBUF_WRITE);
    } else {
        $input = Py_None;
        Py_INCREF(Py_None);
    }
}

%typemap(directorin) int8_t* ch2_8Bit {
    if ($1 != NULL) {
        $input = PyMemoryView_FromMemory((char*)$1, size, PyBUF_WRITE);
    } else {
        $input = Py_None;
        Py_INCREF(Py_None);
    }
}

%typemap(directorin) int16_t* ch1_16Bit {
    if ($1 != NULL) {
        $input = PyMemoryView_FromMemory((char*)$1, size * sizeof(int16_t), PyBUF_WRITE);
    } else {
        $input = Py_None;
        Py_INCREF(Py_None);
    }
}

%typemap(directorin) int16_t* ch2_16Bit {
    if ($1 != NULL) {
        $input = PyMemoryView_FromMemory((char*)$1, size * sizeof(int16_t), PyBUF_WRITE);
    } else {
        $input = Py_None;
        Py_INCREF(Py_None);
    }
}


/* Parse the header file to generate wrappers */
%include "adc_streaming.h"
%include "dac_streaming.h"
%include "config_streaming.h"
%include "callbacks.h"
