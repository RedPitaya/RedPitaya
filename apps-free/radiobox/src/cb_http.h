/**
 * @brief CallBack functions of the HTTP GET/POST parameter transfer system
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 *  Created on: 09.10.2015
 *      Author: espero
 */

#ifndef APPS_CONTRIB_RADIOBOX_SRC_CB_HTTP_H_
#define APPS_CONTRIB_RADIOBOX_SRC_CB_HTTP_H_

#include "main.h"


/** @defgroup cb_http_h CallBack functions of the HTTP GET/POST parameter transfer system
 * @{
 */

/* module entry points */

/** @brief When entering the application the web-server does this call-back for init'ing the application
 *
 * @retval       0    Success.
 * @retval       -1   Failure, worker thread did not start up.
 */
int rp_app_init(void);

/** @brief When leaving the application page the web-server calls this function to exit the application
 *
 * @retval       0    Success, always.
 */
int rp_app_exit(void);


/** @brief When the front-end POSTs data, that is delivered to this call-back to process the parameters
 *
 * @param[in]    p    Parameter list of data received from the web front-end.
 * @param[in]    len  The count of parameters in the list p.
 * @retval       0    Success.
 * @retval       -1   Failed due to bad parameter.
 */
int rp_set_params(rp_app_params_t* p, int len);

/** @brief After POSTing from the front-end the web-server does a request for current parameters and
 *  calls this function. By doing that a current parameter list is returned by p.
 *
 * The returned parameter vector has to be free'd by the caller!
 *
 * @param[inout] p    The parameter vector is returned. Do free the resources before dropping.
 * @retval       int  Number of parameters in the vector.
 */
int rp_get_params(rp_app_params_t** p);

int rp_get_signals(float*** s, int* sig_num, int* sig_len);

/** @} */


#endif /* APPS_CONTRIB_RADIOBOX_SRC_CB_HTTP_H_ */
