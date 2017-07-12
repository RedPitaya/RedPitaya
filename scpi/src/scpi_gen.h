/**
 * @brief Red Pitaya Scpi server generate SCPI commands interface
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef SCPI_GEN_H_
#define SCPI_GEN_H_

#include "scpi/types.h"

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
//scpi_result_t rpscpi_gen_get_waveform_tag(scpi_t *context);

#endif
