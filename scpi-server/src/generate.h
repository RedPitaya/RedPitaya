/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server generate SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#ifndef GENERATE_H_
#define GENERATE_H_

#include "scpi/types.h"
#include "../../api/rpbase/src/rp.h"

scpi_result_t RP_GenState(scpi_t * context);
scpi_result_t RP_GenStateQ(scpi_t * context);
scpi_result_t RP_GenReset(scpi_t * context);
scpi_result_t RP_GenFrequency(scpi_t * context);
scpi_result_t RP_GenFrequencyQ(scpi_t * context);
scpi_result_t RP_GenWaveForm(scpi_t * context);
scpi_result_t RP_GenWaveFormQ(scpi_t * context);
scpi_result_t RP_GenAmplitude(scpi_t * context);
scpi_result_t RP_GenAmplitudeQ(scpi_t * context);
scpi_result_t RP_GenOffset(scpi_t * context);
scpi_result_t RP_GenOffsetQ(scpi_t * context);
scpi_result_t RP_GenPhase(scpi_t * context);
scpi_result_t RP_GenPhaseQ(scpi_t * context);
scpi_result_t RP_GenDutyCycle(scpi_t * context);
scpi_result_t RP_GenDutyCycleQ(scpi_t * context);
scpi_result_t RP_GenArbitraryWaveForm(scpi_t * context);
scpi_result_t RP_GenArbitraryWaveFormQ(scpi_t * context);
scpi_result_t RP_GenGenerateMode(scpi_t * context);
scpi_result_t RP_GenGenerateModeQ(scpi_t * context);
scpi_result_t RP_GenBurstCount(scpi_t * context);
scpi_result_t RP_GenBurstCountQ(scpi_t * context);
scpi_result_t RP_GenBurstRepetitions(scpi_t * context);
scpi_result_t RP_GenBurstRepetitionsQ(scpi_t * context);
scpi_result_t RP_GenBurstPeriod(scpi_t * context);
scpi_result_t RP_GenBurstPeriodQ(scpi_t * context);
scpi_result_t RP_GenTriggerSource(scpi_t * context);
scpi_result_t RP_GenTriggerSourceQ(scpi_t * context);
scpi_result_t RP_GenTrigger(scpi_t *context);

scpi_result_t RP_GenSetOffset(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetOffset(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetPhase(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetPhase(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetDutyCycle(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetDutyCycle(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetArbitraryWaveForm(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetArbitraryWaveForm(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetGenerateMode(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetGenerateMode(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetBurstCount(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetBurstCount(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetBurstRepetitions(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_GenGetBurstRepetitions(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_GenSetBurstPeriod(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_GenGetBurstPeriod(rp_channel_t channel, scpi_t *context);
scpi_result_t RP_GenSetTriggerSource(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetTriggerSource(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetTrigger(int channel, scpi_t * context);

#endif /* GENERATE_H_ */
