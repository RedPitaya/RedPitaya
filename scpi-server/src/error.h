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

using namespace std;

void rp_resetErrorList(scpi_t * context);
void rp_addError(scpi_t * context,string &str);
string rp_popError(scpi_t * context);
size_t rp_errorCount(scpi_t * context);
void rp_errorPush(scpi_t * context, string err);

#endif /* RP_ERROR_H_ */
