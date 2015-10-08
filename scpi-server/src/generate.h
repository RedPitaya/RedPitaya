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
scpi_result_t RP_GenChannel1Amplitude(scpi_t * context);
scpi_result_t RP_GenChannel2Amplitude(scpi_t * context);
scpi_result_t RP_GenChannel1AmplitudeQ(scpi_t * context);
scpi_result_t RP_GenChannel2AmplitudeQ(scpi_t * context);
scpi_result_t RP_GenChannel1Offset(scpi_t * context);
scpi_result_t RP_GenChannel2Offset(scpi_t * context);
scpi_result_t RP_GenChannel1OffsetQ(scpi_t * context);
scpi_result_t RP_GenChannel2OffsetQ(scpi_t * context);
scpi_result_t RP_GenChannel1Phase(scpi_t * context);
scpi_result_t RP_GenChannel2Phase(scpi_t * context);
scpi_result_t RP_GenChannel1PhaseQ(scpi_t * context);
scpi_result_t RP_GenChannel2PhaseQ(scpi_t * context);
scpi_result_t RP_GenChannel1DutyCycle(scpi_t * context);
scpi_result_t RP_GenChannel2DutyCycle(scpi_t * context);
scpi_result_t RP_GenChannel1DutyCycleQ(scpi_t * context);
scpi_result_t RP_GenChannel2DutyCycleQ(scpi_t * context);
scpi_result_t RP_GenChannel1ArbitraryWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel2ArbitraryWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel1ArbitraryWaveFormQ(scpi_t * context);
scpi_result_t RP_GenChannel2ArbitraryWaveFormQ(scpi_t * context);
scpi_result_t RP_GenChannel1GenerateMode(scpi_t * context);
scpi_result_t RP_GenChannel2GenerateMode(scpi_t * context);
scpi_result_t RP_GenChannel1GenerateModeQ(scpi_t * context);
scpi_result_t RP_GenChannel2GenerateModeQ(scpi_t * context);
scpi_result_t RP_GenChannel1BurstCount(scpi_t * context);
scpi_result_t RP_GenChannel2BurstCount(scpi_t * context);
scpi_result_t RP_GenChannel1BurstCountQ(scpi_t * context);
scpi_result_t RP_GenChannel2BurstCountQ(scpi_t * context);
scpi_result_t RP_GenChannel1BurstRepetitions(scpi_t * context);
scpi_result_t RP_GenChannel2BurstRepetitions(scpi_t * context);
scpi_result_t RP_GenChannel1BurstRepetitionsQ(scpi_t * context);
scpi_result_t RP_GenChannel2BurstRepetitionsQ(scpi_t * context);
scpi_result_t RP_GenChannel1BurstPeriod(scpi_t * context);
scpi_result_t RP_GenChannel2BurstPeriod(scpi_t * context);
scpi_result_t RP_GenChannel1BurstPeriodQ(scpi_t * context);
scpi_result_t RP_GenChannel2BurstPeriodQ(scpi_t * context);
scpi_result_t RP_GenChannel1TriggerSource(scpi_t * context);
scpi_result_t RP_GenChannel2TriggerSource(scpi_t * context);
scpi_result_t RP_GenChannel1TriggerSourceQ(scpi_t * context);
scpi_result_t RP_GenChannel2TriggerSourceQ(scpi_t * context);
scpi_result_t RP_GenChannel1Trigger(scpi_t *context);
scpi_result_t RP_GenChannel2Trigger(scpi_t *context);
scpi_result_t RP_GenChannelAllTrigger(scpi_t *context);

scpi_result_t RP_GenSetState(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetState(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetFrequency(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetFrequency(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetAmplitude(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetAmplitude(rp_channel_t channel, scpi_t * context);
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
