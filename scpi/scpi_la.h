/**
 * @brief Red Pitaya SCPI server, laerator commands implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef SCPI_LA_H_
#define SCPI_LA_H_

#include "scpi/types.h"

#include "common.h"

// API init/release
scpi_result_t rpscpi_la_init    (rpscpi_context_t *rp);
scpi_result_t rpscpi_la_release (rpscpi_context_t *rp);

// trigger
scpi_result_t rpscpi_la_trg_set_mask     (scpi_t *context);
scpi_result_t rpscpi_la_trg_get_mask     (scpi_t *context);
scpi_result_t rpscpi_la_trg_set_value    (scpi_t *context);
scpi_result_t rpscpi_la_trg_get_value    (scpi_t *context);
scpi_result_t rpscpi_la_trg_set_edge_pos (scpi_t *context);
scpi_result_t rpscpi_la_trg_get_edge_pos (scpi_t *context);
scpi_result_t rpscpi_la_trg_set_edge_neg (scpi_t *context);
scpi_result_t rpscpi_la_trg_get_edge_neg (scpi_t *context);
// input
scpi_result_t rpscpi_la_msk_set_input_mask     (scpi_t *context);
scpi_result_t rpscpi_la_msk_get_input_mask     (scpi_t *context);
scpi_result_t rpscpi_la_msk_set_input_polarity (scpi_t *context);
scpi_result_t rpscpi_la_msk_get_input_polarity (scpi_t *context);

#endif
