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
scpi_result_t RP_GenChannel1SetFrequency(scpi_t * context);
scpi_result_t RP_GenChannel2SetFrequency(scpi_t * context);
scpi_result_t RP_GenChannel1SetWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel2SetWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel1SetAmplitude(scpi_t * context);
scpi_result_t RP_GenChannel2SetAmplitude(scpi_t * context);
scpi_result_t RP_GenChannel1SetOffset(scpi_t * context);
scpi_result_t RP_GenChannel2SetOffset(scpi_t * context);
scpi_result_t RP_GenChannel1SetPhase(scpi_t * context);
scpi_result_t RP_GenChannel2SetPhase(scpi_t * context);
scpi_result_t RP_GenChannel1SetDutyCycle(scpi_t * context);
scpi_result_t RP_GenChannel2SetDutyCycle(scpi_t * context);
scpi_result_t RP_GenChannel1SetArbitraryWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel2SetArbitraryWaveForm(scpi_t * context);
scpi_result_t RP_GenChannel1SetGenerateMode(scpi_t * context);
scpi_result_t RP_GenChannel2SetGenerateMode(scpi_t * context);
scpi_result_t RP_GenChannel1SetBurstCount(scpi_t * context);
scpi_result_t RP_GenChannel2SetBurstCount(scpi_t * context);
scpi_result_t RP_GenChannel1SetTriggerSource(scpi_t * context);
scpi_result_t RP_GenChannel2SetTriggerSource(scpi_t * context);
scpi_result_t RP_GenChannel1SetTrigger(scpi_t * context);
scpi_result_t RP_GenChannel2SetTrigger(scpi_t * context);
scpi_result_t RP_GenChannel3SetTrigger(scpi_t * context);

scpi_result_t RP_GenSetState(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetFrequency(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetWaveForm(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetAmplitude(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetOffset(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetPhase(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetDutyCycle(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetArbitraryWaveForm(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetGenerateMode(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetBurstCount(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetTriggerSource(rp_channel_t channel, scpi_t * context);
scpi_result_t RP_GenSetTrigger(int channel, scpi_t * context);

#endif /* GENERATE_H_ */
