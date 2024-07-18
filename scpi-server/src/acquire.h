/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server acquire SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


#ifndef ACQUIRE_H_
#define ACQUIRE_H_

#include "scpi/types.h"
#include "common.h"
#include "rp.h"


int RP_AcqSetDefaultValues();

scpi_result_t RP_AcqSplitTrigger(scpi_t * context);
scpi_result_t RP_AcqSplitTriggerQ(scpi_t * context);

scpi_result_t RP_AcqDataFormat(scpi_t *context);
scpi_result_t RP_AcqDataFormatQ(scpi_t *context);
scpi_result_t RP_AcqStart(scpi_t * context);
scpi_result_t RP_AcqStartCh(scpi_t *context);
scpi_result_t RP_AcqStop(scpi_t *context);
scpi_result_t rp_AcqStopCh(scpi_t *context);
scpi_result_t RP_AcqReset(scpi_t * context);
scpi_result_t RP_AcqResetCh(scpi_t *context);
scpi_result_t RP_AcqDecimation(scpi_t * context);
scpi_result_t RP_AcqDecimationCh(scpi_t *context);
scpi_result_t RP_AcqDecimationQ(scpi_t * context);
scpi_result_t RP_AcqDecimationChQ(scpi_t * context);
scpi_result_t RP_AcqDecimationFactor(scpi_t * context);
scpi_result_t RP_AcqDecimationFactorCh(scpi_t * context);
scpi_result_t RP_AcqDecimationFactorQ(scpi_t * context);
scpi_result_t RP_AcqDecimationFactorChQ(scpi_t * context);
scpi_result_t RP_AcqSamplingRateHzQ(scpi_t * context);
scpi_result_t RP_AcqSamplingRateHzChQ(scpi_t * context);
scpi_result_t RP_AcqAveraging(scpi_t * context);
scpi_result_t RP_AcqAveragingQ(scpi_t * context);
scpi_result_t RP_AcqAveragingCh(scpi_t * context);
scpi_result_t RP_AcqAveragingChQ(scpi_t * context);
scpi_result_t RP_AcqTriggerSrc(scpi_t * context);
scpi_result_t RP_AcqTriggerSrcCh(scpi_t * context);
scpi_result_t RP_AcqTriggerStateQ(scpi_t *context);
scpi_result_t RP_AcqTriggerStateChQ(scpi_t *context);
scpi_result_t RP_AcqTriggerDelay(scpi_t * context);
scpi_result_t RP_AcqTriggerDelayCh(scpi_t *context);
scpi_result_t RP_AcqTriggerDelayQ(scpi_t * context);
scpi_result_t RP_AcqTriggerDelayChQ(scpi_t *context);
scpi_result_t RP_AcqTriggerDelayNs(scpi_t * context);
scpi_result_t RP_AcqTriggerDelayNsCh(scpi_t *context);
scpi_result_t RP_AcqTriggerDelayNsQ(scpi_t * context);
scpi_result_t RP_AcqTriggerDelayNsChQ(scpi_t *context);
scpi_result_t RP_AcqTriggerHyst(scpi_t *context);
scpi_result_t RP_AcqTriggerHystQ(scpi_t *context);
scpi_result_t RP_AcqTriggerFillQ(scpi_t *context);
scpi_result_t RP_AcqTriggerFillChQ(scpi_t *context);
scpi_result_t RP_AcqGain(scpi_t * context);
scpi_result_t RP_AcqGainQ(scpi_t * context);
scpi_result_t RP_AcqTriggerLevel(scpi_t *context);
scpi_result_t RP_AcqTriggerLevelQ(scpi_t *context);
scpi_result_t RP_AcqTriggerLevelCh(scpi_t *context);
scpi_result_t RP_AcqTriggerLevelChQ(scpi_t *context);
scpi_result_t RP_AcqWritePointerQ(scpi_t * context);
scpi_result_t RP_AcqWritePointerChQ(scpi_t *context);
scpi_result_t RP_AcqWritePointerAtTrigQ(scpi_t * context);
scpi_result_t RP_AcqWritePointerAtTrigChQ(scpi_t *context);
scpi_result_t RP_AcqScpiDataUnits(scpi_t * context);
scpi_result_t RP_AcqScpiDataUnitsQ(scpi_t *context);
scpi_result_t RP_AcqScpiDataFormat(scpi_t * context);
scpi_result_t RP_AcqDataPosQ(scpi_t * context);
scpi_result_t RP_AcqDataQ(scpi_t * context);
scpi_result_t RP_AcqDataOldestAllQ(scpi_t * context);
scpi_result_t RP_AcqOldestDataQ(scpi_t *context);
scpi_result_t RP_AcqLatestDataQ(scpi_t *context);
scpi_result_t RP_AcqTriggerDataQ(scpi_t *context);
scpi_result_t RP_AcqBufferSizeQ(scpi_t * context);

scpi_result_t RP_AcqGetLatestData(rp_channel_t channel, scpi_t * context);


scpi_result_t RP_AcqAC_DC(scpi_t * context);
scpi_result_t RP_AcqAC_DCQ(scpi_t * context);

scpi_result_t RP_ExtTriggerLevel(scpi_t *context);
scpi_result_t RP_ExtTriggerLevelQ(scpi_t *context);


#endif /* ACQUIRE_H_ */
