%module rp_hw_calib

%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <std_string.i>
%include <cpointer.i>

%apply unsigned int { rp_HPeModels_t }
%apply unsigned char { uint8_t }
%apply unsigned short { uint16_t }
%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }
%apply int { int32_t }


%apply float *OUTPUT { float *_out_value };
%apply unsigned char *OUTPUT { uint8_t *_out_value };
%apply unsigned short *OUTPUT { uint16_t *_out_size };
%apply unsigned int *OUTPUT { uint32_t *_out_value };

%apply bool *OUTPUT { bool *_out_value };
%apply std::string *OUTPUT { std::string *_name };
%apply double *OUTPUT { double *_out_gain };
%apply int *OUTPUT { int32_t *_out_offset };


#define __attribute__(x) 

%typemap(in,numinputs=0)  uint8_t** _out_data (uint8_t* temp) {
  $1 = &temp;
}

%typemap(argout) uint8_t** _out_data (PyObject* obj) {
    obj = SWIG_NewPointerObj(*$1, $descriptor(uint8_t*), 0);
    $result = SWIG_Python_AppendOutput($result,obj);
}

%typemap(in,numinputs=0) char** _out_no_free (char* tmp) %{
    $1 = &tmp;
%}

%typemap(argout) char** _out_no_free (PyObject* obj) %{
    obj = PyUnicode_FromString(*$1);
    $result = SWIG_Python_AppendOutput($result,obj);
%}

%{
/* Includes the header in the wrapper code */
#include "rp_hw_calib.h"

%}

%pointer_functions(rp_calib_params_t, p_rp_calib_params_t);
%pointer_functions(channel_filter_t, p_channel_filter_t);
%pointer_functions(uint_gain_calib_t, p_uint_gain_calib_t);

%pointer_functions(uint8_t, puint8);
%array_class(uint8_t, uint8Arr);
%array_functions(channel_calib_t,cCalibArr);
%array_functions(channel_filter_t,cFilterArr);
%array_functions(rp_eepromUniData_item_t,ceppromUniDataArr);


/* Parse the header file to generate wrappers */
%include "rp_hw_calib.h"