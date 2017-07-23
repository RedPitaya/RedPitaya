/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server apin SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef API_CMD_H_
#define API_CMD_H_

#include "scpi/types.h"

scpi_result_t RP_InitAll(scpi_t *context);
scpi_result_t RP_ResetAll(scpi_t *context);
scpi_result_t RP_ReleaseAll(scpi_t *context);
scpi_result_t RP_FpgaBitStream(scpi_t *context);
scpi_result_t RP_EnableDigLoop(scpi_t *context);

#endif /* API_CMD_H_ */
