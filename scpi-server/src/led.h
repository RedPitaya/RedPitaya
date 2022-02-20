/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server LED SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#ifndef RPHW_LED_H_
#define RPHW_LED_H_

#include "scpi/types.h"

scpi_result_t RP_LED_MMC(scpi_t * context);
scpi_result_t RP_LED_MMCQ(scpi_t * context);
scpi_result_t RP_LED_HB(scpi_t * context);
scpi_result_t RP_LED_HBQ(scpi_t * context);
scpi_result_t RP_LED_ETH(scpi_t * context);
scpi_result_t RP_LED_ETHQ(scpi_t * context);

#endif /* RPHW_LED_H_ */
