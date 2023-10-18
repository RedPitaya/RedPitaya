/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#include <string>

#include "scpi/config.h"
#include "scpi/types.h"

#ifndef RP_ERROR_H_
#define RP_ERROR_H_

#define RP_ERR_CODE 9000
#define RP_ERR_CODE_FATAL 9500

typedef struct{
    int baseCode = 0;
    int errorCode = 0;
    std::string msg;
} rp_error_t;

using namespace std;

auto rp_resetErrorList(scpi_t * context) -> void;
auto rp_addError(scpi_t * context, rp_error_t &err) -> void;
auto rp_popError(scpi_t * context) -> rp_error_t;
auto rp_errorCount(scpi_t * context) -> size_t;
auto rp_errorPush(scpi_t * context, rp_error_t &err) -> void;

#endif /* RP_ERROR_H_ */
