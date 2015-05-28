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

#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/types.h"
#include "../../api-mockup/rpbase/src/rp.h"

scpi_result_t RP_GenChannel1SetState(scpi_t * context);
scpi_result_t RP_GenChannel2SetState(scpi_t * context);
scpi_result_t RP_GenChannel1GetState(scpi_t * context);
scpi_result_t RP_GenChannel2GetState(scpi_t * context);
scpi_result_t RP_GenReset(scpi_t * context);
scpi_result_t RP_GenChannel1SetFrequency(scpi_t * context);
scpi_result_t RP_GenChannel2SetFrequency(scpi_t * context);
scpi_result_t RP_GenChannel1GetFrequency(scpi_t * context);
scpi_result_t RP_GenChannel2GetFrequency(scpi_t * context);
scpi_result_t RP_GenChannel1SetWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel2SetWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel1GetWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel2GetWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel1SetAmplitude(scpi_t * context);
scpi_result_t RP_GenChannel2SetAmplitude(scpi_t * context);
scpi_result_t RP_GenChannel1GetAmplitude(scpi_t * context);
scpi_result_t RP_GenChannel2GetAmplitude(scpi_t * context);
scpi_result_t RP_GenChannel1SetOffset(scpi_t * context);
scpi_result_t RP_GenChannel2SetOffset(scpi_t * context);
scpi_result_t RP_GenChannel1GetOffset(scpi_t * context);
scpi_result_t RP_GenChannel2GetOffset(scpi_t * context);
scpi_result_t RP_GenChannel1SetPhase(scpi_t * context);
scpi_result_t RP_GenChannel2SetPhase(scpi_t * context);
scpi_result_t RP_GenChannel1GetPhase(scpi_t * context);
scpi_result_t RP_GenChannel2GetPhase(scpi_t * context);
scpi_result_t RP_GenChannel1SetDutyCycle(scpi_t * context);
scpi_result_t RP_GenChannel2SetDutyCycle(scpi_t * context);
scpi_result_t RP_GenChannel1GetDutyCycle(scpi_t * context);
scpi_result_t RP_GenChannel2GetDutyCycle(scpi_t * context);
scpi_result_t RP_GenChannel1SetArbitraryWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel2SetArbitraryWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel1GetArbitraryWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel2GetArbitraryWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel1SetGenerateMode(scpi_t * context);
scpi_result_t RP_GenChannel2SetGenerateMode(scpi_t * context);
scpi_result_t RP_GenChannel1GetGenerateMode(scpi_t * context);
scpi_result_t RP_GenChannel2GetGenerateMode(scpi_t * context);
scpi_result_t RP_GenChannel1SetBurstCount(scpi_t * context);
scpi_result_t RP_GenChannel2SetBurstCount(scpi_t * context);
scpi_result_t RP_GenChannel1GetBurstCount(scpi_t * context);
scpi_result_t RP_GenChannel2GetBurstCount(scpi_t * context);
scpi_result_t RP_GenChannel1SetBurstRepetitions(scpi_t * context);
scpi_result_t RP_GenChannel2SetBurstRepetitions(scpi_t * context);
scpi_result_t RP_GenChannel1GetBurstRepetitions(scpi_t * context);
scpi_result_t RP_GenChannel2GetBurstRepetitions(scpi_t * context);
scpi_result_t RP_GenChannel1SetBurstPeriod(scpi_t * context);
scpi_result_t RP_GenChannel2SetBurstPeriod(scpi_t * context);
scpi_result_t RP_GenChannel1GetBurstPeriod(scpi_t * context);
scpi_result_t RP_GenChannel2GetBurstPeriod(scpi_t * context);
scpi_result_t RP_GenChannel1SetTriggerSource(scpi_t * context);
scpi_result_t RP_GenChannel2SetTriggerSource(scpi_t * context);
scpi_result_t RP_GenChannel1GetTriggerSource(scpi_t * context);
scpi_result_t RP_GenChannel2GetTriggerSource(scpi_t * context);
scpi_result_t RP_GenChannel1Trigger(scpi_t *context);
scpi_result_t RP_GenChannel2Trigger(scpi_t *context);
scpi_result_t RP_GenChannelAllTrigger(scpi_t *context);

scpi_result_t RP_GenSetState(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetState(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetFrequency(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetFrequency(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetWaveForm(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenGetWaveForm(rp_channel_t channel, scpi_t * context);
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
