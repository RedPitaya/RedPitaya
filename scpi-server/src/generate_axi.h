/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server generate axi SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#ifndef GENERATE_AXI_H_
#define GENERATE_AXI_H_

#include "common.h"
#include "rp.h"
#include "scpi/types.h"

scpi_result_t RP_GenAxiStartQ(scpi_t* context);
scpi_result_t RP_GenAxiEndQ(scpi_t* context);
scpi_result_t RP_GenAxiReserveMemory(scpi_t* context);
scpi_result_t RP_GenAxiReleaseMemory(scpi_t* context);
scpi_result_t RP_GenAxiSetEnable(scpi_t* context);
scpi_result_t RP_GenAxiGetEnable(scpi_t* context);
scpi_result_t RP_GenAxiSetDecimationFactor(scpi_t* context);
scpi_result_t RP_GenAxiGetDecimationFactor(scpi_t* context);
scpi_result_t RP_GenSetAmplitudeAndOffsetOrigin(scpi_t* context);
scpi_result_t RP_GenAxiWriteWaveform(scpi_t* context);

#endif /* GENERATE_AXI_H_ */
