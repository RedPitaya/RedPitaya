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

#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/types.h"

typedef enum {
    RP_SCPI_VOLTS,
    RP_SCPI_RAW,
} rp_scpi_acq_unit_t;

typedef enum {
    RP_SCPI_FLAOT,
    RP_SCPI_ADCII,
} rp_scpi_acq_format_t;

int RP_AcqSetDefaultValues();

scpi_result_t RP_AcqSetDataFormat(scpi_t *context);
scpi_result_t RP_AcqStart(scpi_t * context);
scpi_result_t RP_AcqReset(scpi_t * context);
scpi_result_t RP_AcqSetDecimation(scpi_t * context);
scpi_result_t RP_AcqGetDecimation(scpi_t * context);
scpi_result_t RP_AcqSetSamplingRate(scpi_t * context);
scpi_result_t RP_AcqGetSamplingRate(scpi_t * context);
scpi_result_t RP_AcqGetSamplingRateHz(scpi_t * context);
scpi_result_t RP_AcqSetAveraging(scpi_t * context);
scpi_result_t RP_AcqGetAveraging(scpi_t * context);
scpi_result_t RP_AcqSetTriggerSrc(scpi_t * context);
scpi_result_t RP_AcqGetTriggerSrc(scpi_t * context);
scpi_result_t RP_AcqSetTriggerDelay(scpi_t * context);
scpi_result_t RP_AcqGetTriggerDelay(scpi_t * context);
scpi_result_t RP_AcqSetTriggerDelayNs(scpi_t * context);
scpi_result_t RP_AcqGetTriggerDelayNs(scpi_t * context);
scpi_result_t RP_AcqSetChannel1Gain(scpi_t * context);
scpi_result_t RP_AcqSetChannel2Gain(scpi_t * context);
scpi_result_t RP_AcqGetChannel1Gain(scpi_t * context);
scpi_result_t RP_AcqGetChannel2Gain(scpi_t * context);
scpi_result_t RP_AcqSetTriggerLevel(scpi_t *context);
scpi_result_t RP_AcqGetTriggerLevel(scpi_t *context);
scpi_result_t RP_AcqGetWritePointer(scpi_t * context);
scpi_result_t RP_AcqGetWritePointerAtTrig(scpi_t * context);
scpi_result_t RP_AcqScpiDataUnits(scpi_t * context);
scpi_result_t RP_AcqScpiDataFormat(scpi_t * context);
scpi_result_t RP_AcqGetChanel1DataPos(scpi_t * context);
scpi_result_t RP_AcqGetChanel2DataPos(scpi_t * context);
scpi_result_t RP_AcqGetChanel1Data(scpi_t * context);
scpi_result_t RP_AcqGetChanel2Data(scpi_t * context);
scpi_result_t RP_AcqGetChanel1OldestDataAll(scpi_t * context);
scpi_result_t RP_AcqGetChanel2OldestDataAll(scpi_t * context);
scpi_result_t RP_AcqGetChanel1OldestData(scpi_t * context);
scpi_result_t RP_AcqGetChanel2OldestData(scpi_t * context);
scpi_result_t RP_AcqGetChanel1LatestData(scpi_t * context);
scpi_result_t RP_AcqGetChanel2LatestData(scpi_t * context);
scpi_result_t RP_AcqGetBufferSize(scpi_t * context);

scpi_result_t RP_AcqSetGain(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_AcqGetGain(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_AcqGetLatestData(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_AcqGetOldestDataAll(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_AcqGetOldestData(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_AcqGetDataPos(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_AcqGetData(rp_channel_t channel, scpi_t * context);

#endif /* ACQUIRE_H_ */
