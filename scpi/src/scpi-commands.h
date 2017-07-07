/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server SCPI configuration and general command interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef SCPI_COMMANDS_H_
#define SCPI_COMMANDS_H_

#include "scpi/scpi.h"

#define SCPI_INPUT_BUFFER_LENGTH 538688
#define SCPI_ERROR_QUEUE_SIZE 17
#define SCPI_IDN1 "REDPITAYA"
#define SCPI_IDN2 "INSTR2017"
#define SCPI_IDN3 NULL
#define SCPI_IDN4 "01-02"

extern const scpi_command_t scpi_commands[];
extern scpi_interface_t scpi_interface;
extern char scpi_input_buffer[];
extern scpi_error_t scpi_error_queue_data[];
extern scpi_t scpi_context;

#endif /* SCPI_COMMANDS_H_ */
