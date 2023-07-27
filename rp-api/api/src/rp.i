%module rp

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>

%apply int { rp_dpin_t }
%apply int { rp_pinState_t }
%apply int { rp_outTiggerMode_t }
%apply int { rp_pinDirection_t }
%apply int { rp_apin_t }
%apply int { rp_waveform_t }
%apply int { rp_gen_mode_t }
%apply int { rp_gen_sweep_dir_t }
%apply int { rp_gen_sweep_mode_t }
%apply int { rp_trig_src_t }
%apply int { rp_gen_gain_t }
%apply int { rp_channel_t }
%apply int { rp_channel_trigger_t }
%apply int { rp_eq_filter_cof_t }
%apply int { buffers_t }
%apply int { rp_acq_decimation_t }
%apply int { rp_acq_ac_dc_mode_t }
%apply int { rp_acq_trig_src_t }
%apply int { rp_acq_trig_state_t }
%apply int { rp_gen_gain_t }

%apply int *OUTPUT { rp_pinState_t * state }
%apply int *OUTPUT { rp_pinDirection_t * direction }
%apply int *OUTPUT { rp_outTiggerMode_t * mode }
%apply int *OUTPUT { rp_waveform_t * type }
%apply int *OUTPUT { rp_gen_sweep_mode_t * mode }
%apply int *OUTPUT { rp_gen_sweep_dir_t * mode }
%apply int *OUTPUT { rp_gen_mode_t * mode }
%apply int *OUTPUT { rp_trig_src_t * src }
%apply int *OUTPUT { rp_gen_gain_t * status }

%apply bool *OUTPUT { bool * status };
%apply bool *OUTPUT { bool * state };
%apply bool *OUTPUT { bool * enable };
%apply bool *OUTPUT { bool * value };

%apply float *OUTPUT { float *value };
%apply float *OUTPUT { float *min_val };
%apply float *OUTPUT { float *max_val };
%apply float *OUTPUT { float *amplitude };
%apply float *OUTPUT { float *offset };
%apply float *OUTPUT { float *frequency };
%apply float *OUTPUT { float *phase };
%apply float *OUTPUT { float *ratio };
%apply float *OUTPUT { float *time };
%apply float *INOUT { float *waveform };

%apply double *OUTPUT { double *value };

// %apply unsigned char *OUTPUT { uint8_t *_out_value };
// %apply unsigned short *OUTPUT { uint16_t *_out_value };

%apply unsigned int *OUTPUT { uint32_t *state };
%apply unsigned int *OUTPUT { uint32_t *direction };
%apply unsigned int *OUTPUT { uint32_t *id };
%apply unsigned int *OUTPUT { uint32_t *raw };
%apply unsigned int *OUTPUT { uint32_t *value };
%apply unsigned int *OUTPUT { uint32_t *period };
%apply unsigned int *OUTPUT { uint32_t *length };
%apply unsigned long long *OUTPUT { uint64_t *dna };

%apply int *OUTPUT { int * num };
%apply int *OUTPUT { int * repetitions };

%array_class(float, arbBuffer);

// %apply unsigned int *OUTPUT { size_t *_out_value };
// %apply unsigned int *OUTPUT { size_t *_out_len };

// %apply unsigned int *OUTPUT { uint32_t *raw };
// %apply float *OUTPUT { float *value };


// %apply int *OUTPUT { int *_out_value };

// %apply int *INOUT { int *_in_out_size };


%{
/* Includes the header in the wrapper code */
#include "rp.h"
%}



/* Parse the header file to generate wrappers */
%include "rp_enums.h"
%include "rp.h"
%include "rp_gen.h"