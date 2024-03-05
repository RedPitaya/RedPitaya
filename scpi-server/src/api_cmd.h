/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server apin SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#ifndef API_CMD_H_
#define API_CMD_H_

#include "scpi/types.h"

scpi_result_t RP_InitAll(scpi_t *context);
scpi_result_t RP_ResetAll(scpi_t *context);
scpi_result_t RP_ReleaseAll(scpi_t *context);
scpi_result_t RP_EnableDigLoop(scpi_t *context);

scpi_result_t RP_EnableDaisyChainTrigSync(scpi_t *context);
scpi_result_t RP_EnableDaisyChainTrigSyncQ(scpi_t *context);
scpi_result_t RP_EnableDaisyChainClockSync(scpi_t *context);
scpi_result_t RP_EnableDaisyChainClockSyncQ(scpi_t *context);
scpi_result_t RP_DpinEnableTrigOutput(scpi_t *context);
scpi_result_t RP_DpinEnableTrigOutputQ(scpi_t *context);
scpi_result_t RP_SourceTrigOutput(scpi_t *context);
scpi_result_t RP_SourceTrigOutputQ(scpi_t *context);

scpi_result_t RP_PLL(scpi_t *context);
scpi_result_t RP_PLLQ(scpi_t *context);
scpi_result_t RP_PLLStateQ(scpi_t *context);

#endif /* API_CMD_H_ */
