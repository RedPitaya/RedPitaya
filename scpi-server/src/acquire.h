/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server acquire SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#ifndef ACQUIRE_H_
#define ACQUIRE_H_

#include "scpi/types.h"
#include "redpitaya/rp.h"

typedef enum {
    RP_SCPI_VOLTS,
    RP_SCPI_RAW,
} rp_scpi_acq_unit_t;

int RP_AcqSetDefaultValues();
scpi_result_t RP_AcqSetDataFormat(scpi_t *context);
scpi_result_t RP_AcqStart(scpi_t * context);
scpi_result_t RP_AcqStop(scpi_t *context);
scpi_result_t RP_AcqReset(scpi_t * context);
scpi_result_t RP_AcqDecimation(scpi_t * context);
scpi_result_t RP_AcqDecimationQ(scpi_t * context);
scpi_result_t RP_AcqSamplingRateHzQ(scpi_t * context);
scpi_result_t RP_AcqAveraging(scpi_t * context);
scpi_result_t RP_AcqAveragingQ(scpi_t * context);
scpi_result_t RP_AcqTriggerSrc(scpi_t * context);
scpi_result_t RP_AcqTriggerSrcQ(scpi_t *context);
scpi_result_t RP_AcqTriggerDelay(scpi_t * context);
scpi_result_t RP_AcqTriggerDelayQ(scpi_t * context);
scpi_result_t RP_AcqTriggerDelayNs(scpi_t * context);
scpi_result_t RP_AcqTriggerDelayNsQ(scpi_t * context);
scpi_result_t RP_AcqTriggerHyst(scpi_t *context);
scpi_result_t RP_AcqTriggerHystQ(scpi_t *context);
scpi_result_t RP_AcqGain(scpi_t * context);
scpi_result_t RP_AcqGainQ(scpi_t * context);
scpi_result_t RP_AcqTriggerLevel(scpi_t *context);
scpi_result_t RP_AcqTriggerLevelQ(scpi_t *context);
scpi_result_t RP_AcqWritePointerQ(scpi_t * context);
scpi_result_t RP_AcqWritePointerAtTrigQ(scpi_t * context);
scpi_result_t RP_AcqScpiDataUnits(scpi_t * context);
scpi_result_t RP_AcqScpiDataUnitsQ(scpi_t *context);
scpi_result_t RP_AcqScpiDataFormat(scpi_t * context);
scpi_result_t RP_AcqDataPosQ(scpi_t * context);
scpi_result_t RP_AcqDataQ(scpi_t * context);
scpi_result_t RP_AcqDataOldestAllQ(scpi_t * context);
scpi_result_t RP_AcqOldestDataQ(scpi_t *context);
scpi_result_t RP_AcqLatestDataQ(scpi_t *context);
scpi_result_t RP_AcqBufferSizeQ(scpi_t * context);

scpi_result_t RP_AcqGetLatestData(rp_channel_t channel, scpi_t * context);

#endif /* ACQUIRE_H_ */
