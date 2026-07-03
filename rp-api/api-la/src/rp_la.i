%module(directors="1") rp_la

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>
%include <std_map.i>
%include <std_string.i>
%include <std_vector.i>
%include <stl.i>

%apply int { la_Mode_t }
%apply int { la_Trigger_Mode_t }
%apply int { la_Trigger_Channel_t }

%apply bool *OUTPUT { bool * isTimeout };
%apply int *OUTPUT { uint32_t * value };
%apply float *OUTPUT { float * value };

%apply const std::string & { std::string & key };
%apply std::string *OUTPUT { std::string * value };
%apply const std::string & { std::string & value };

%{
#define SWIG_FILE_WITH_INIT
/* Includes the header in the wrapper code */
#include "rp_la.h"
%}

%include "numpy.i"

%init %{
import_array();
%}

%feature("director") CLACallback;

%numpy_typemaps(uint8_t,    NPY_UINT8   , short)
%apply (uint8_t* IN_ARRAY1, int DIM1) {(uint8_t* np_buffer, int size)};

%template(StringVector) std::vector<std::string>;

%typemap(out) std::vector<rp_la::OutputPacket> {
    size_t size = $1.size();
    PyObject* list = PyList_New(size);

    for (size_t i = 0; i < size; ++i) {
        PyObject* d = PyDict_New();

        PyObject* p_control     = PyLong_FromLong($1.at(i).control);
        PyObject* p_data        = PyLong_FromLong($1.at(i).data);
        PyObject* p_line_name   = PyUnicode_FromString($1.at(i).line_name.c_str());
        PyObject* p_sampleStart = PyFloat_FromDouble($1.at(i).sampleStart);
        PyObject* p_length      = PyFloat_FromDouble($1.at(i).length);
        PyObject* p_bitsInPack  = PyFloat_FromDouble($1.at(i).bitsInPack);

        PyDict_SetItemString(d, "control", p_control);
        PyDict_SetItemString(d, "data", p_data);
        PyDict_SetItemString(d, "line_name", p_line_name);
        PyDict_SetItemString(d, "sampleStart", p_sampleStart);
        PyDict_SetItemString(d, "length", p_length);
        PyDict_SetItemString(d, "bitsInPack", p_bitsInPack);

        Py_DECREF(p_control);
        Py_DECREF(p_data);
        Py_DECREF(p_line_name);
        Py_DECREF(p_sampleStart);
        Py_DECREF(p_length);
        Py_DECREF(p_bitsInPack);

        PyList_SetItem(list, i, d);
    }
    $result = list;
}

%typemap(out) std::map<uint8_t, std::string> {
    PyObject* dict = PyDict_New();
    if (!dict) {
        PyErr_SetString(PyExc_MemoryError, "Failed to create dict");
        SWIG_fail;
    }

    for (std::map<uint8_t, std::string>::const_iterator it = $1.begin(); it != $1.end(); ++it) {
        PyObject* key = PyLong_FromLong(it->first);
        PyObject* value = PyUnicode_FromString(it->second.c_str());

        if (!key || !value) {
            Py_XDECREF(key);
            Py_XDECREF(value);
            Py_DECREF(dict);
            SWIG_fail;
        }

        PyDict_SetItem(dict, key, value);
        Py_DECREF(key);
        Py_DECREF(value);
    }

    $result = dict;
}

/* Parse the header file to generate wrappers */
%include "rp_la.h"

