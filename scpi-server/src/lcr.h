/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server LCR commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


#ifndef LCR_H
#define LCR_H

#include "scpi/types.h"


void stopLCR();


scpi_result_t RP_LCRStart(scpi_t * context);
scpi_result_t RP_LCRStartGen(scpi_t * context);
scpi_result_t RP_LCRStop(scpi_t * context);
scpi_result_t RP_LCRReset(scpi_t * context);
scpi_result_t RP_LCRMeasureQ(scpi_t * context);

scpi_result_t RP_LCRFrequency(scpi_t * context);
scpi_result_t RP_LCRFrequencyQ(scpi_t * context);
scpi_result_t RP_LCRAmplitude(scpi_t * context);
scpi_result_t RP_LCRAmplitudeQ(scpi_t * context);
scpi_result_t RP_LCROffset(scpi_t * context);
scpi_result_t RP_LCROffsetQ(scpi_t * context);

scpi_result_t RP_LCRShunt(scpi_t * context);
scpi_result_t RP_LCRShuntQ(scpi_t * context);

scpi_result_t RP_LCRCustomShunt(scpi_t * context);
scpi_result_t RP_LCRCustomShuntQ(scpi_t * context);

scpi_result_t RP_LCRShuntMode(scpi_t * context);
scpi_result_t RP_LCRShuntModeQ(scpi_t * context);

scpi_result_t RP_LCRShuntAuto(scpi_t * context);

scpi_result_t RP_LCRMeasSeries(scpi_t * context);
scpi_result_t RP_LCRMeasSeriesQ(scpi_t * context);

scpi_result_t RP_LCRCheckExtensionModuleConnectioQ(scpi_t * context);

#endif /* LCR_H */
