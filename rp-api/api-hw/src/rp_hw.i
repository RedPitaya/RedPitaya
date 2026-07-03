%module rp_hw

%{
#define SWIG_FILE_WITH_INIT
#include "rp_hw.h"
%}

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>
%include "numpy.i"

%init %{
  import_array();
%}

%apply int { rp_uart_bits_size_t, rp_uart_stop_bits_t, rp_uart_parity_t, rp_spi_mode_t, rp_spi_state_t, rp_spi_cs_mode_t, rp_spi_order_bit_t }
%apply int *OUTPUT { rp_uart_bits_size_t * _out_value, rp_uart_stop_bits_t * _out_value, rp_uart_parity_t * _out_value, rp_spi_mode_t * _out_value, rp_spi_state_t * _out_value, rp_spi_cs_mode_t * _out_value, rp_spi_order_bit_t * _out_value, int *_out_value }
%apply bool *OUTPUT { bool * _out_value };
%apply unsigned char *OUTPUT { uint8_t *_out_value };
%apply unsigned short *OUTPUT { uint16_t *_out_value };
%apply unsigned int *OUTPUT { uint32_t *_out_value, size_t *_out_value, size_t *_out_len, uint32_t *raw };
%apply float *OUTPUT { float *value };

/* --- NUMPY TYPEMAPS --- */

%apply (unsigned char* IN_ARRAY1, int DIM1) {
    (uint8_t* buffer, int size),
    (const uint8_t* tx_buffer, size_t len),
    (uint8_t* buffer, int len)
};

%typemap(in, numinputs=1) (uint8_t* buffer, int* _in_out_size),
                          (unsigned char* buffer, int* _in_out_size) {
  int temp_len = (int)PyLong_AsLong($input);
  if (PyErr_Occurred()) SWIG_fail;

  $2 = (int*) malloc(sizeof(int));
  *$2 = temp_len;
  $1 = ($1_type) malloc(temp_len * sizeof($*1_type));

  if (!$1 || !$2) {
      PyErr_SetString(PyExc_MemoryError, "Out of memory");
      SWIG_fail;
  }
}

%typemap(argout) (uint8_t* buffer, int* _in_out_size),
                 (unsigned char* buffer, int* _in_out_size) {
    npy_intp dims[1];
    dims[0] = (npy_intp)(*$2);

    PyObject* array = PyArray_SimpleNewFromData(1, dims, NPY_UINT8, (void*)$1);
    if (array) {
        PyArray_ENABLEFLAGS((PyArrayObject*)array, NPY_ARRAY_OWNDATA);
    }
    $result = SWIG_Python_AppendOutput($result, array);

    free($2);
}

%typemap(in, numinputs=0) (const uint8_t** _out_buffer, size_t* _out_len) (uint8_t* temp_ptr, size_t temp_len) {
    $1 = &temp_ptr;
    $2 = &temp_len;
}

%typemap(argout) (const uint8_t** _out_buffer, size_t* _out_len) {
    npy_intp dims[1] = { (npy_intp)*$2 };
    PyObject* array = PyArray_SimpleNewFromData(1, dims, NPY_UINT8, (void*)*$1);
    $result = SWIG_Python_AppendOutput($result, array);
}

%include "rp_hw.h"
