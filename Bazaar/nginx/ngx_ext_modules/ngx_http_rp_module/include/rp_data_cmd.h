/**
 * $Id$
 *
 * @brief Red Pitaya Nginx module - App interface.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_DATA_CMD_H
#define __RP_DATA_CMD_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ngx_http_rp_module.h"
#include "cJSON.h"

/* Main handler */
ngx_int_t rp_data_cmd_handler(ngx_http_request_t *r);

/* Read out request body for POST */
void rp_data_post_read(ngx_http_request_t *r);

/* Command calls */
int rp_data_help(ngx_http_request_t *r, 
                 cJSON **json_root, int argc, char **argv);
int rp_data_get_params_list(ngx_http_request_t *r, 
                            cJSON **json_root, int argc, char **argv);
int rp_data_get_signals_list(ngx_http_request_t *r, 
                             cJSON **json_root, int argc, char **argv);
/* Called from callback rp_data_post_read() */
int rp_data_get_params(ngx_http_request_t *r, cJSON **json_root);
int rp_data_set_signals(ngx_http_request_t *r, cJSON **json_root);
int rp_data_get_signals(ngx_http_request_t *r, cJSON **json_root);
/* Clear dirty flag in case of re-send */
void rp_data_clear_signals_dirty();

/* Helper functions */
int rp_data_parse_and_set_params(cJSON *params_root);

#endif /* __RP_DATA_CMD_H */
