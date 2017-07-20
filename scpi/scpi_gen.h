/**
 * @brief Red Pitaya SCPI server, generator commands implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef SCPI_GEN_H_
#define SCPI_GEN_H_

#include "scpi/types.h"

#include "common.h"

// API init/release
scpi_result_t rpscpi_gen_init    (rpscpi_context_t *rp, int unsigned channels);
scpi_result_t rpscpi_gen_release (rpscpi_context_t *rp);

// evn
scpi_result_t rpscpi_gen_reset                (scpi_t *context);
scpi_result_t rpscpi_gen_start                (scpi_t *context);
scpi_result_t rpscpi_gen_stop                 (scpi_t *context);
scpi_result_t rpscpi_gen_trigger              (scpi_t *context);
scpi_result_t rpscpi_gen_set_mode             (scpi_t *context);
scpi_result_t rpscpi_gen_get_mode             (scpi_t *context);
// gen_out
scpi_result_t rpscpi_gen_set_enable           (scpi_t *context);
scpi_result_t rpscpi_gen_get_enable           (scpi_t *context);
scpi_result_t rpscpi_gen_set_amplitude        (scpi_t *context);
scpi_result_t rpscpi_gen_get_amplitude        (scpi_t *context);
scpi_result_t rpscpi_gen_set_offset           (scpi_t *context);
scpi_result_t rpscpi_gen_get_offset           (scpi_t *context);
// wave
scpi_result_t rpscpi_gen_set_waveform_shape   (scpi_t *context);
scpi_result_t rpscpi_gen_get_waveform_shape   (scpi_t *context);
scpi_result_t rpscpi_gen_set_waveform_data    (scpi_t *context);
scpi_result_t rpscpi_gen_get_waveform_data    (scpi_t *context);
// asg_per
scpi_result_t rpscpi_gen_set_frequency        (scpi_t *context);
scpi_result_t rpscpi_gen_get_frequency        (scpi_t *context);
scpi_result_t rpscpi_gen_set_phase            (scpi_t *context);
scpi_result_t rpscpi_gen_get_phase            (scpi_t *context);
// asg_bst
scpi_result_t rpscpi_gen_set_burst_mode       (scpi_t *context);
scpi_result_t rpscpi_gen_get_burst_mode       (scpi_t *context);
scpi_result_t rpscpi_gen_set_data_repetitions (scpi_t *context);
scpi_result_t rpscpi_gen_get_data_repetitions (scpi_t *context);
scpi_result_t rpscpi_gen_set_data_length      (scpi_t *context);
scpi_result_t rpscpi_gen_get_data_length      (scpi_t *context);
scpi_result_t rpscpi_gen_set_period_length    (scpi_t *context);
scpi_result_t rpscpi_gen_get_period_length    (scpi_t *context);
scpi_result_t rpscpi_gen_set_period_number    (scpi_t *context);
scpi_result_t rpscpi_gen_get_period_number    (scpi_t *context);

#endif
