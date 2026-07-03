%module rp_dsp

%include <stdint.i>
%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>

%include <std_vector.i>
%include <std_array.i>

%apply int { window_mode_t }
%apply int { mode_t }

%apply uint32_t *OUTPUT { uint32_t *value };
%apply bool *OUTPUT { bool *enable };
%apply double *OUTPUT { double *_amp1 };
%apply double *OUTPUT { double *_phase1 };
%apply double *OUTPUT { double *_amp2 };
%apply double *OUTPUT { double *_phase2 };

%inline %{
typedef double *double_ptr;
typedef float *float_ptr;
typedef int *int_ptr;
typedef int16_t *int16_ptr;
%}

%{
#include "rp_dsp.h"
#include "rp_math.h"

namespace rp_dsp_api {
    std::vector<cdsp_data_t>& data_t_getChannelData(data_t* d, int ch) {
        return d->m_in[ch];
    }
    bool data_t_isDataFiltered(data_t* d) {
        return d->m_is_data_filtred;
    }
    void data_t_setDataFiltered(data_t* d, bool val) {
        d->m_is_data_filtred = val;
    }
}

%}

%include "rp_dsp.h"

// Extend data_t with Python-friendly accessors using helper functions
%extend rp_dsp_api::data_t {
    std::vector<rp_dsp_api::cdsp_data_t>& getChannelData(int ch) {
        return rp_dsp_api::data_t_getChannelData(self, ch);
    }

    bool isDataFiltered() {
        return rp_dsp_api::data_t_isDataFiltered(self);
    }

    void setDataFiltered(bool val) {
        rp_dsp_api::data_t_setDataFiltered(self, val);
    }
}

%template(cdsp_data_vec_t) std::vector<rp_dsp_api::cdsp_data_t>;
%template(cdsp_data_ch_t) std::vector<std::vector<rp_dsp_api::cdsp_data_t>>;

%template(cdsp_result_array_t) std::array<std::vector<std::vector<rp_dsp_api::cdsp_data_t>>, rp_dsp_api::COUNT_DSP_MODE>;
%template(cdsp_peak_array_t) std::array<std::vector<rp_dsp_api::cdsp_data_t>, rp_dsp_api::COUNT_DSP_MODE>;
%typedef cdsp_peak_array_t cdsp_peak_power_array_t;
%typedef cdsp_peak_array_t cdsp_peak_freq_array_t;

%array_class(double, arrDouble);
%array_class(float, arrFloat);
%array_class(int, arrInt);
%array_class(int16_t, arrInt16);
%array_class(double_ptr, arrpDouble);
%array_class(float_ptr, arrpFloat);
%array_class(int_ptr, arrpInt);
%array_class(int16_ptr, arrpInt16);

%apply float *INOUT { volatile float *dst };
%apply float *INPUT { volatile const float *src };
%apply float *INPUT { volatile const float *src1 };
%apply float *INPUT { volatile const float *src2 };

%apply double *INOUT { volatile double *dst };
%apply double *INPUT { volatile const double *src };
%apply double *INPUT { volatile const double *src1 };
%apply double *INPUT { volatile const double *src2 };

%apply int *INOUT { volatile int *dst };
%apply int *INPUT { volatile const int *src };
%apply int *INPUT { volatile const int *src1 };
%apply int *INPUT { volatile const int *src2 };

%apply int16_t *INOUT { volatile int16_t *dst };
%apply int16_t *INPUT { volatile const int16_t *src };
%apply int16_t *INPUT { volatile const int16_t *src1 };
%apply int16_t *INPUT { volatile const int16_t *src2 };


%include "rp_math.h"