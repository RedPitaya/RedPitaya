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


#ifndef APIN_H_
#define APIN_H_

int RP_ApinSetDefaultValues();

scpi_result_t RP_AnalogPinGetValue(scpi_t * context);
scpi_result_t RP_AnalogPinSetValue(scpi_t * context);

#endif /* APIN_H_ */
