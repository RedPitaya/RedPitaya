/**
 * @brief Red Pitaya Scpi server generate SCPI commands interface
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef SCPI_GEN_H_
#define SCPI_GEN_H_

#include "scpi/types.h"

const scpi_choice_def_t rpscpi_waveform_names[] = {
    {"SINusoid", 0},
    {"SQUare",   1},
    {"TRIangle", 2},
    {"USER",     7},
    SCPI_CHOICE_LIST_END
};

// These structure is a direct API mirror and should not be altered!
const scpi_choice_def_t scpi_RpGenTrig[] = {
    {"INT",     1},
    {"EXT_PE",  2},
    {"EXT_NE",  3},
    {"GATED",   4},
    SCPI_CHOICE_LIST_END
};

// These structure is a direct API mirror and should not be altered!
const scpi_choice_def_t scpi_RpGenMode[] = {
    {"CONTINUOUS",  0},
    {"BURST",       1},
    {"STREAM",      2},
    SCPI_CHOICE_LIST_END
};

scpi_result_t rpscpi_gen_reset           (scpi_t *context);
scpi_result_t rpscpi_gen_set_enable      (scpi_t *context);
scpi_result_t rpscpi_gen_get_enable      (scpi_t *context);
scpi_result_t rpscpi_gen_set_frequency   (scpi_t *context);
scpi_result_t rpscpi_gen_get_frequency   (scpi_t *context);
scpi_result_t rpscpi_gen_set_phase       (scpi_t *context);
scpi_result_t rpscpi_gen_get_phase       (scpi_t *context);
scpi_result_t rpscpi_gen_set_amplitude   (scpi_t *context);
scpi_result_t rpscpi_gen_get_amplitude   (scpi_t *context);
scpi_result_t rpscpi_gen_set_offset      (scpi_t *context);
scpi_result_t rpscpi_gen_get_offset      (scpi_t *context);
scpi_result_t rpscpi_gen_set_waveform_tag(scpi_t *context);
scpi_result_t rpscpi_gen_get_waveform_tag(scpi_t *context);

#endif
