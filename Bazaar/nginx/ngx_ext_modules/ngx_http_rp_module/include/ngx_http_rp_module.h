/**
 * $Id$
 *
 * @brief Red Pitaya Nginx module.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __NGX_HTTP_RP_MODULE_H
#define __NGX_HTTP_RP_MODULE_H

#include <ngx_log.h>

#include "rp_bazaar_app.h"

/* Helper log functions */
#define rp_debug(log, args...)                                                    \
    ngx_log_error(NGX_LOG_DEBUG, log, 0, args);
#define rp_notice(log, args...)                                                   \
    ngx_log_error(NGX_LOG_NOTICE, log, 0, args);
#define rp_warn(log, args...)                                                     \
    ngx_log_error(NGX_LOG_WARN, log, 0, args);
#define rp_error(log, args...)                                                    \
    ngx_log_error(NGX_LOG_ERR, log, 0, args);

extern const char *json_content_str;

typedef int (*rp_parse_body_func)(ngx_http_request_t *r);

typedef struct ngx_http_rp_loc_conf_s {
    /* Parameters */
    ngx_str_t       bazaar_dir;
    ngx_str_t       bazaar_server;
    ngx_str_t       tmp_dir;
    /* Internal structures */
    /* Be careful to use this only in local modules (it must be NULL all other
     * time.
     */

    /* Input buffers */
    char           *in_buffer;
    ngx_int_t       in_buffer_length;
    int             in_status;
} ngx_http_rp_loc_conf_t;

/* TODO: Figure out where to clean this */
typedef struct ngx_http_rp_ctx_s {
    ngx_log_t      *log;

    /* Application structure - define din rp_bazaar_app.h */
    rp_bazaar_app_t app;
} ngx_http_rp_module_ctx_t;

extern ngx_http_rp_module_ctx_t rp_module_ctx;

/* Function pointer used for easier parsing/calling Bazaar & Data cmds
 *  void **args optional additional input parameters.
 */
typedef int (*rp_module_cmd_func)(ngx_http_request_t *r, 
                                  cJSON **root, int argc,
                                  char **argv);

/* the following structure defines available commands for Bazaar command given 
 * in the header to the 'rp_bazaar' module.
 */
typedef struct rp_module_cmd_s {
    /* Name must have 'arg_' appended to it's original name - Nginx parses the
     * arguments this way.
     */
    const char         *name;
    const char         *params;
    const char         *desc;
    rp_module_cmd_func  func;
} rp_module_cmd_t;

/* Helper function which parses the argument from the command.
 * The output parameter argv must be free externally!
 */
int rp_module_cmd_parse_args(const char *args, int len,
                             char ***argv);

/* helper functions - to form JSON output with status commands */
int rp_module_cmd_error(cJSON **json_root, const char *reason, 
                        const char *stderror, ngx_pool_t *pool);
int rp_module_cmd_ok(cJSON **json_root, ngx_pool_t *pool);
int rp_module_cmd_again(cJSON **json_root, ngx_pool_t *pool);

ngx_int_t rp_module_redirect(ngx_http_request_t *r, const char *location);
ngx_int_t rp_module_send_response(ngx_http_request_t *r, cJSON **json_root);

extern ngx_module_t ngx_http_rp_module;

#endif /*__NGX_HTTP_RP_MODULE_H*/
