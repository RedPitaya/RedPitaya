/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server sweep mode commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#ifndef SWEEP_H_
#define SWEEP_H_

#include "scpi/types.h"

scpi_result_t RP_GenSweepDefault(scpi_t* context);
scpi_result_t RP_GenSweepState(scpi_t* context);
scpi_result_t RP_GenSweepStateQ(scpi_t* context);
scpi_result_t RP_GenSweepPause(scpi_t* context);
scpi_result_t RP_GenSweepReset(scpi_t* context);
scpi_result_t RP_GenSweepFreqStart(scpi_t* context);
scpi_result_t RP_GenSweepFreqStartQ(scpi_t* context);
scpi_result_t RP_GenSweepFreqStop(scpi_t* context);
scpi_result_t RP_GenSweepFreqStopQ(scpi_t* context);
scpi_result_t RP_GenSweepTime(scpi_t* context);
scpi_result_t RP_GenSweepTimeQ(scpi_t* context);
scpi_result_t RP_GenSweepMode(scpi_t* context);
scpi_result_t RP_GenSweepModeQ(scpi_t* context);
scpi_result_t RP_GenSweepRep(scpi_t* context);
scpi_result_t RP_GenSweepRepQ(scpi_t* context);
scpi_result_t RP_GenSweepDir(scpi_t* context);
scpi_result_t RP_GenSweepDirQ(scpi_t* context);

#endif /* SWEEP_H_ */
