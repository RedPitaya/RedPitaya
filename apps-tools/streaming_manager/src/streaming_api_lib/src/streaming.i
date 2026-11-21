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
#include "gpio_streaming.h"
#include "config_streaming.h"
%}


%feature("director") ADCCallback;
%feature("director") DACCallback;
%feature("director") GPIOCallback;
%feature("director") ConfigCallback;

%shared_ptr(ConfigCallback)
%shared_ptr(DACCallback)
%shared_ptr(GPIOCallback)
%shared_ptr(ADCCallback)
%shared_ptr(ConfigStreamClient)

%template(StringVector) std::vector<std::string>;
%template(Int16Vector) std::vector<int16_t>;
%template(Int8Vector) std::vector<int8_t>;
%template(UInt32Vector) std::vector<uint32_t>;
%template(StringList) std::list<std::string>;
%template(BoolArray4) std::array<bool, 4>;

// The samples of an ADC channel and the words of a GPIO buffer (streaming_improvements 4.15): a
// numpy array of their own copy, one memcpy (the buffer of the client is reused after the
// callback); numpy is imported at the first one
%{
static PyObject* rpNumpyArray(const void* data, size_t bytes, const char* dtype) {
    static PyObject* frombuffer = nullptr;
    if (!frombuffer) {
        PyObject* np = PyImport_ImportModule("numpy");
        if (!np)
            return nullptr;
        frombuffer = PyObject_GetAttrString(np, "frombuffer");
        Py_DECREF(np);
        if (!frombuffer)
            return nullptr;
    }
    // a bytearray: the array is writable and owns the data
    PyObject* owner = PyByteArray_FromStringAndSize(data && bytes ? static_cast<const char*>(data) : "", data ? static_cast<Py_ssize_t>(bytes) : 0);
    if (!owner)
        return nullptr;
    PyObject* array = PyObject_CallFunction(frombuffer, "Os", owner, dtype);
    Py_DECREF(owner);
    return array;
}
%}

%typemap(out) ADCSamples* %{
    $result = rpNumpyArray($1->data, (size_t)$1->count * ($1->bits == 8 ? 1 : 2), $1->bits == 8 ? "int8" : "int16");
    if (!$result)
        SWIG_fail;
%}

%typemap(out) GPIOWords* %{
    $result = rpNumpyArray($1->data, (size_t)$1->count * 4, "uint32");
    if (!$result)
        SWIG_fail;
%}

// C++ only: Python sees the arrays
%ignore ADCSamples;
%ignore GPIOWords;

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
%include "gpio_streaming.h"
%include "config_streaming.h"
%include "callbacks.h"
