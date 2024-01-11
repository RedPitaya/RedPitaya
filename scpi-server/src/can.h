/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server CAN SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


#ifndef SCPI_CAN_H_
#define SCPI_CAN_H_

#include "scpi/types.h"

scpi_result_t RP_CAN_FpgaEnable(scpi_t * context);
scpi_result_t RP_CAN_FpgaEnableQ(scpi_t * context);

scpi_result_t RP_CAN_Start(scpi_t * context);
scpi_result_t RP_CAN_Stop(scpi_t * context);
scpi_result_t RP_CAN_Restart(scpi_t * context);
scpi_result_t RP_CAN_StateQ(scpi_t * context);

scpi_result_t RP_CAN_Bitrate(scpi_t * context);
scpi_result_t RP_CAN_BitrateSamplePoint(scpi_t * context);
scpi_result_t RP_CAN_BitrateSamplePointQ(scpi_t * context);

scpi_result_t RP_CAN_BitTiming(scpi_t * context);
scpi_result_t RP_CAN_BitTimingQ(scpi_t * context);
scpi_result_t RP_CAN_BitTimingLimitsQ(scpi_t * context);

scpi_result_t RP_CAN_ClockFreqQ(scpi_t * context);
scpi_result_t RP_CAN_BusErrorCountersQ(scpi_t * context);

scpi_result_t RP_CAN_RestartTime(scpi_t * context);
scpi_result_t RP_CAN_RestartTimeQ(scpi_t * context);

scpi_result_t RP_CAN_ControllerMode(scpi_t * context);
scpi_result_t RP_CAN_ControllerModeQ(scpi_t * context);

scpi_result_t RP_CAN_Open(scpi_t * context);
scpi_result_t RP_CAN_Close(scpi_t * context);

scpi_result_t RP_CAN_Send(scpi_t * context);
scpi_result_t RP_CAN_ReadQ(scpi_t * context);

scpi_result_t RP_CAN_AddFilter(scpi_t * context);
scpi_result_t RP_CAN_RemoveFilter(scpi_t * context);
scpi_result_t RP_CAN_ClearFilter(scpi_t * context);
scpi_result_t RP_CAN_SetFilter(scpi_t * context);

scpi_result_t RP_CAN_ShowErrorFrames(scpi_t * context);


#endif /* SCPI_CAN_H_ */
