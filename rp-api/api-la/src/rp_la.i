%module(directors="1") rp_la

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>
%include <std_string.i>
%include <stl.i>

%apply int { la_Mode_t }
%apply int { la_Trigger_Mode_t }
%apply bool *OUTPUT { bool * isTimeout };


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

/* Parse the header file to generate wrappers */
%include "rp_la.h"
