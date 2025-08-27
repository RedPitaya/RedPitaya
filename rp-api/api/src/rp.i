%module rp

%include <stdint.i>
%include <std_vector.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>
%include <stl.i>

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
%apply int { rp_acq_decimation_t }
%apply int { rp_acq_ac_dc_mode_t }
%apply int { rp_acq_trig_src_t }
%apply int { rp_acq_trig_state_t }
%apply int { rp_gen_gain_t }
%apply int { rp_gen_load_mode_t }

%apply int *OUTPUT { rp_pinState_t * state }
%apply int *OUTPUT { rp_pinDirection_t * direction }
%apply int *OUTPUT { rp_outTiggerMode_t * mode }
%apply int *OUTPUT { rp_waveform_t * type }
%apply int *OUTPUT { rp_gen_sweep_mode_t * mode }
%apply int *OUTPUT { rp_gen_sweep_dir_t * mode }
%apply int *OUTPUT { rp_gen_mode_t * mode }
%apply int *OUTPUT { rp_trig_src_t * src }
%apply int *OUTPUT { rp_gen_gain_t * mode }
%apply int *OUTPUT { rp_acq_decimation_t * decimation }
%apply int *OUTPUT { rp_acq_trig_src_t * source }
%apply int *OUTPUT { rp_acq_trig_state_t * state }
%apply int *OUTPUT { rp_acq_ac_dc_mode_t * status }
%apply int *OUTPUT { rp_gen_load_mode_t * mode }

%apply bool *OUTPUT { bool * status };
%apply bool *OUTPUT { bool * state };
%apply bool *OUTPUT { bool * enable };
%apply bool *OUTPUT { bool * value };

%apply float *OUTPUT { float *value };
%apply float *OUTPUT { float *min_val };
%apply float *OUTPUT { float *max_val };
%apply float *OUTPUT { float *amplitude };
%apply float *OUTPUT { float *offset };
%apply float *OUTPUT { float *voltage };
%apply float *OUTPUT { float *frequency };
%apply float *OUTPUT { float *phase };
%apply float *OUTPUT { float *ratio };
%apply float *OUTPUT { float *time };
%apply float *OUTPUT { float *sampling_rate };

%apply float *INOUT { float *waveform };

%apply double *OUTPUT { double *value };

%apply int16_t *INOUT { int16_t *buffer };
%apply float *INOUT { float *buffer };

%apply unsigned int *OUTPUT { uint32_t *state };
%apply unsigned int *OUTPUT { uint32_t *direction };
%apply unsigned int *OUTPUT { uint32_t *id };
%apply unsigned int *OUTPUT { uint32_t *raw };
%apply unsigned int *OUTPUT { uint32_t *value };
%apply unsigned int *OUTPUT { uint32_t *period };
%apply unsigned int *OUTPUT { uint32_t *length };
%apply unsigned int *OUTPUT { uint32_t *decimation };
%apply unsigned int *OUTPUT { uint32_t *pos };
%apply unsigned int *OUTPUT { uint32_t *coef_aa };
%apply unsigned int *OUTPUT { uint32_t *coef_bb };
%apply unsigned int *OUTPUT { uint32_t *coef_kk };
%apply unsigned int *OUTPUT { uint32_t *coef_pp };
%apply unsigned int *OUTPUT { uint32_t *_start };
%apply unsigned int *OUTPUT { uint32_t *_size };
%apply unsigned int *OUTPUT { uint32_t *size_out };

%apply unsigned int *INOUT { uint32_t *size };
%apply unsigned int *INOUT { uint32_t *buffer_size };

%apply unsigned long long *OUTPUT { uint64_t *dna };
%apply long long *OUTPUT { int64_t *time_ns };
%apply unsigned long long *OUTPUT { uint64_t *time_ns };

%apply int *OUTPUT { int * num };
%apply int *OUTPUT { int * repetitions };
%apply int *OUTPUT { int * decimated_data_num };
%apply unsigned int *OUTPUT { unsigned int * decimated_data_num };

%array_class(float, arbBuffer);
%array_class(int16_t, i16Buffer);
%array_class(float, fBuffer);
%array_class(double, dBuffer);

%array_functions(int16_t*,pi16Arr);
%array_functions(float*,pfArr);
%array_functions(double*,pdArr);

%{
#define SWIG_FILE_WITH_INIT
/* Includes the header in the wrapper code */
#include "rp.h"
%}

%typemap(in, numinputs=0) std::vector<std::span<int16_t>>* data (std::vector<std::span<int16_t>> temp) {
  $1 = &temp;
}

%typemap(argout) std::vector<std::span<int16_t>>* data {
  PyObject* pyList = PyList_New($1->size());
  for (size_t i = 0; i < $1->size(); ++i) {
    auto& span = (*$1)[i];
    PyObject* memview = PyMemoryView_FromMemory(
      reinterpret_cast<char*>(span.data()),
      span.size() * sizeof(int16_t),
      PyBUF_READ);
    PyList_SET_ITEM(pyList, i, memview);
  }
  $result = SWIG_Python_AppendOutput($result, pyList);
}

%include "numpy.i"

%init %{
import_array();
%}

%pointer_functions(buffers_t, p_buffers_t);

%numpy_typemaps(int16_t,    NPY_INT16   , short)

%apply (int16_t* IN_ARRAY1, int DIM1) {(int16_t* np_buffer, int size)};
%apply (float* IN_ARRAY1, int DIM1) {(float* np_buffer, int size)};
%apply (float* IN_ARRAY1, int DIM1) {(float* np_buffer, int size)};

/* Parse the header file to generate wrappers */
%include "rp_enums.h"
%include "rp.h"
%include "rp_gen.h"
%include "rp_acq.h"
%include "rp_acq_axi.h"
%include "rp_asg_axi.h"