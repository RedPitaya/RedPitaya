%module(directors="1") rp_updater

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>
%include <std_string.i>
%include <stl.i>
%include <std_vector.i>


%apply uint64_t* OUTPUT { uint64_t *size };
%apply uint32_t* OUTPUT { uint32_t *build_number };
%apply uint32_t* OUTPUT { uint32_t *count };


%apply bool* OUTPUT { bool *state };
%apply std::string *OUTPUT { std::string *name };
%apply std::string *OUTPUT { std::string *commit };
%apply std::string *OUTPUT { std::string *hash };

%{
#include "rp_updater.h"
#include <vector>
#include <string>
#include <Python.h>

PyObject* vector_to_pylist(const std::vector<std::string>& vec) {
    PyObject* py_list = PyList_New(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        PyList_SET_ITEM(py_list, i, PyUnicode_FromString(vec[i].c_str()));
    }
    return py_list;
}

%}

%feature("director") CUpdaterCallback;

%typemap(in, numinputs=0) std::vector<std::string>& files (std::vector<std::string> temp) {
    $1 = &temp;
}

%typemap(argout) std::vector<std::string>& files {
    $result = vector_to_pylist(*$1);
}

/* Parse the header file to generate wrappers */
%include "rp_updater.h"
%include "rp_updater_common.h"
