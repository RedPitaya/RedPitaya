/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server oscilloscope application SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#ifndef SPECTROMETER_APP_H_
#define SPECTROMETER_APP_H_

#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/types.h"
#include "../../api-mockup/rpbase/src/rp.h"
#include "../../api-mockup/rpApplications/src/rpApp.h"

scpi_result_t RP_APP_SpecRun(scpi_t *context); // :RUN
scpi_result_t RP_APP_SpecStop(scpi_t *context); // :STOP
scpi_result_t RP_APP_SpecReset(scpi_t *context); // :RST
scpi_result_t RP_APP_SpecAutoscale(scpi_t *context); // :AUTOSCALE
scpi_result_t RP_APP_SpecRunning(scpi_t *context); // :RUNNING

scpi_result_t RP_APP_SpecChannel1GetViewData(scpi_t *context); // :CH1:DATA?
scpi_result_t RP_APP_SpecChannel2GetViewData(scpi_t *context); // :CH2:DATA?
scpi_result_t RP_APP_SpecGetViewSize(scpi_t *context); // :DATA:SIZE?
scpi_result_t RP_APP_SpecSetViewSize(scpi_t *context); // :DATA:SIZE

scpi_result_t RP_APP_SpecChannel1GetPeak(scpi_t *context); // :CH1:PEAK
scpi_result_t RP_APP_SpecChannel2GetPeak(scpi_t *context); // :CH2:PEAK
scpi_result_t RP_APP_SpecChannel1GetPeakFreq(scpi_t *context); // :CH1:PEAK:FREQ
scpi_result_t RP_APP_SpecChannel2GetPeakFreq(scpi_t *context); // :CH2:PEAK:FREQ
scpi_result_t RP_APP_SpecChannel1Freeze(scpi_t *context); // :CH1:FREEZE
scpi_result_t RP_APP_SpecChannel2Freeze(scpi_t *context); // :CH2:FREEZE

scpi_result_t RP_APP_SpecGetFreqMin(scpi_t *context); // :FREQ:MIN
scpi_result_t RP_APP_SpecGetFreqMax(scpi_t *context); // :FREQ:MAX
scpi_result_t RP_APP_SpecSetFreqMin(scpi_t *context); // :FREQ:MIN
scpi_result_t RP_APP_SpecSetFreqMax(scpi_t *context); // :FREQ:MAX
scpi_result_t RP_APP_SpecGetFpgaFreq(scpi_t *context); // :FPGA:FREQ

#endif /* OSCILLOSCOPE_APP_H_ */
