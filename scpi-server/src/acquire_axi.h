/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server acquire axi SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


#ifndef ACQUIRE_AXI_H_
#define ACQUIRE_AXI_H_

#include "scpi/types.h"
#include "common.h"
#include "rp.h"

scpi_result_t RP_AcqAxiTriggerFillQ(scpi_t *context);
scpi_result_t RP_AcqAxiDecimation(scpi_t *context);
scpi_result_t RP_AcqAxiDecimationQ(scpi_t *context);
scpi_result_t RP_AcqAxiDecimationCh(scpi_t *context);
scpi_result_t RP_AcqAxiDecimationChQ(scpi_t *context);
scpi_result_t RP_AcqAxiTriggerDelay(scpi_t *context);
scpi_result_t RP_AcqAxiTriggerDelayQ(scpi_t *context);
scpi_result_t RP_AcqAxiWritePointerQ(scpi_t *context);
scpi_result_t RP_AcqAxiWritePointerAtTrigQ(scpi_t *context);
scpi_result_t RP_AcqAxiStartQ(scpi_t *context);
scpi_result_t RP_AcqAxiEndQ(scpi_t *context);
scpi_result_t RP_AcqAxiEnable(scpi_t *context);
scpi_result_t RP_AcqAxiSetAddres(scpi_t *context);

scpi_result_t RP_AcqAxiDataQ(scpi_t *context);
scpi_result_t RP_AcqAxiScpiDataUnits(scpi_t *context);
scpi_result_t RP_AcqAxiScpiDataUnitsQ(scpi_t *context);



#endif /* ACQUIRE_AXI_H_ */
