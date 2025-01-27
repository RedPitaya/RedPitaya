%module(directors="1") rp_la

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>
%include <std_string.i>
%include <std_vector.i>
%include <stl.i>

%apply int { la_Mode_t }
%apply int { la_Trigger_Mode_t }
%apply int { la_Trigger_Channel_t }

%apply bool *OUTPUT { bool * isTimeout };
%apply int *OUTPUT { uint32_t * value };
%apply float *OUTPUT { float * value };


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

%typemap(out) std::vector<rp_la::OutputPacket> {
    PyObject* list = PyList_New( $1.size() );
    for (size_t i = 0; i < $1.size(); ++i) {
        PyObject* d = PyDict_New();
        PyDict_SetItemString(d, "control", PyInt_FromLong($1.at(i).control));
        PyDict_SetItemString(d, "data", PyInt_FromLong($1.at(i).data));
        PyDict_SetItemString(d, "line_name", PyString_FromString($1.at(i).line_name.c_str()));
        PyDict_SetItemString(d, "sampleStart", PyFloat_FromDouble($1.at(i).sampleStart));
        PyDict_SetItemString(d, "length", PyFloat_FromDouble($1.at(i).length));
        PyDict_SetItemString(d, "bitsInPack", PyFloat_FromDouble($1.at(i).bitsInPack));
        PyList_SetItem(list, i, d);
    }
    $result = list;
}

/* Parse the header file to generate wrappers */
%include "rp_la.h"

