%module rp_hw

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>

%apply int { rp_uart_bits_size_t }
%apply int { rp_uart_stop_bits_t }
%apply int { rp_uart_parity_t }
%apply int { rp_spi_mode_t }
%apply int { rp_spi_state_t }
%apply int { rp_spi_cs_mode_t }
%apply int { rp_spi_order_bit_t }


%apply int *OUTPUT { rp_uart_bits_size_t * _out_value }
%apply int *OUTPUT { rp_uart_stop_bits_t * _out_value }
%apply int *OUTPUT { rp_uart_parity_t * _out_value }
%apply int *OUTPUT { rp_spi_mode_t * _out_value }
%apply int *OUTPUT { rp_spi_state_t * _out_value }
%apply int *OUTPUT { rp_spi_cs_mode_t * _out_value }
%apply int *OUTPUT { rp_spi_order_bit_t * _out_value }

%apply bool *OUTPUT { bool * _out_value };

%apply unsigned char *OUTPUT { uint8_t *_out_value };
%apply unsigned short *OUTPUT { uint16_t *_out_value };
%apply unsigned int *OUTPUT { uint32_t *_out_value };
%apply unsigned int *OUTPUT { size_t *_out_value };
%apply unsigned int *OUTPUT { size_t *_out_len };

%apply unsigned int *OUTPUT { uint32_t *raw };
%apply float *OUTPUT { float *value };


%apply int *OUTPUT { int *_out_value };

%apply int *INOUT { int *_in_out_size };
%array_class(unsigned char, Buffer);

%typemap(in,numinputs=0)  uint8_t** _out_buffer (uint8_t* temp) {
  $1 = &temp;
}

%typemap(argout) uint8_t** _out_buffer (PyObject* obj) {
    obj = SWIG_NewPointerObj(*$1, $descriptor(uint8_t*), 0);
    $result = SWIG_Python_AppendOutput($result,obj);
}

%{
/* Includes the header in the wrapper code */
#include "rp_hw.h"

%}



/* Parse the header file to generate wrappers */
%include "rp_hw.h"