/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server apin SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#ifndef APIN_H_
#define APIN_H_

#include "scpi/types.h"

scpi_result_t RP_AnalogPinReset(scpi_t* context);
scpi_result_t RP_AnalogPinValueQ(scpi_t* context);
scpi_result_t RP_AnalogPinValue(scpi_t* context);

#endif /* APIN_H_ */
