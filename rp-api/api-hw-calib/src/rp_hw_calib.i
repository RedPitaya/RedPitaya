%module rp_hw_calib

%include <typemaps.i>
%include <cstring.i>
%include <std_string.i>
%include <carrays.i>
%include <cpointer.i>

// Basic type mappings
%apply unsigned int { rp_HPeModels_t }
%apply unsigned char { uint8_t }
%apply unsigned short { uint16_t }
%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }
%apply int { int32_t }

// Output parameter mappings
%apply float *OUTPUT { float *_out_value };
%apply unsigned char *OUTPUT { uint8_t *_out_value };
%apply unsigned char *OUTPUT { uint8_t *version };
%apply unsigned short *OUTPUT { uint16_t *_out_size };
%apply unsigned int *OUTPUT { uint32_t *_out_value };
%apply bool *OUTPUT { bool *_out_value };
%apply double *OUTPUT { double *_out_gain };
%apply int *OUTPUT { int32_t *_out_offset };
%apply std::string *OUTPUT { std::string *_name };

#define __attribute__(x)

%{
#define SWIG_FILE_WITH_INIT
#include "rp_hw_calib.h"
%}

%include "numpy.i"
%init %{
import_array();
%}

// Pointer functions for memory management
%pointer_functions(uint8_t, puint8);
%pointer_functions(rp_calib_params_t, p_rp_calib_params_t);
%pointer_functions(channel_filter_t, p_channel_filter_t);
%pointer_functions(uint_gain_calib_t, p_uint_gain_calib_t);

// Array functions for structures
%array_class(uint8_t, uint8Arr);
%array_functions(channel_calib_t, cCalibArr);
%array_functions(channel_filter_t, cFilterArr);
%array_functions(rp_eepromUniData_item_t, ceppromUniDataArr);

// Typemap for uint8_t** output
%typemap(in,numinputs=0) uint8_t** _out_data (uint8_t* temp) {
    $1 = &temp;
}
%typemap(argout) uint8_t** _out_data {
    PyObject* obj = SWIG_NewPointerObj(*$1, $descriptor(uint8_t*), 0);
    $result = SWIG_Python_AppendOutput($result, obj);
}

// Numpy input for conversion function
%apply (uint8_t* IN_ARRAY1, int DIM1) {(uint8_t* data, uint16_t size)};

// Python wrappers with numpy support
%pythoncode %{
import numpy as np
import ctypes

def rp_CalibGetEEPROMNP(use_factory_zone=False):
    """Get EEPROM data as numpy array"""
    result = rp_CalibGetEEPROM(use_factory_zone)

    if result[0] == RP_HW_CALIB_OK and result[2] > 0:
        try:
            ptr = int(result[1])
            size = result[2]
            buf = (ctypes.c_uint8 * size).from_address(ptr)
            return result[0], np.frombuffer(buf, dtype=np.uint8).copy()
        except:
            pass
    return result[0], None

def rp_CalibConvertEEPROMNP(data_array):
    """Convert numpy array to calibration structure

    Args:
        data_array: numpy uint8 array with EEPROM data

    Returns:
        tuple: (status, calibration_object)
    """
    if not isinstance(data_array, np.ndarray):
        data_array = np.array(data_array, dtype=np.uint8)
    data = np.ascontiguousarray(data_array.astype(np.uint8))

    # Create output structure
    t = new_p_rp_calib_params_t()

    # numpy.i typemap converts (data, size) to single numpy array argument
    # So function expects exactly 2 args: (numpy_array, output_struct)
    status = rp_CalibConvertEEPROM(data, t)

    return status, t

def rp_CalibGetAndConvertNP(use_factory_zone=False):
    """Get and convert EEPROM calibration in one call"""
    status, data = rp_CalibGetEEPROMNP(use_factory_zone)
    if status == RP_HW_CALIB_OK and data is not None:
        return rp_CalibConvertEEPROMNP(data)
    return status, None
%}

%include "rp_hw_calib.h"

