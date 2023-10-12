%module rp_hw_can

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>

%apply int { rp_can_interface_t }
%apply int { rp_can_state_t }
%apply int { rp_can_mode_t }

%apply bool *OUTPUT { bool * };
%apply unsigned int *OUTPUT { uint32_t * };
%apply unsigned short *OUTPUT { uint16_t * };
%apply float *OUTPUT { float * };
%apply int *OUTPUT { rp_can_state_t * };

%array_class(unsigned char, Buffer);
%array_functions(uint8_t, uintArr);

%{
/* Includes the header in the wrapper code */
#include "rp_hw_can.h"
%}

%pointer_functions(rp_can_bittiming_t, p_rp_can_bittiming_t);
%pointer_functions(rp_can_bittiming_limits_t, p_rp_can_bittiming_limits_t);
%pointer_functions(rp_can_frame_t, p_rp_can_frame_t);

/* Parse the header file to generate wrappers */
%include "rp_hw_can.h"