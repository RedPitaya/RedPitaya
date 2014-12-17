/**
 * $Id$
 *
 * @brief Red Pitaya Nginx module - Bazaar interface.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_BAZAAR_CMD_H
#define __RP_BAZAAR_CMD_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ngx_http_rp_module.h"
#include "cJSON.h"

/* TODOs:
 *    - make URI for application nicer
 *    - make nicer argument parsing 
 */

/** Bazaar actions */
typedef enum {
    eInstall,
    eUpgrade,
    eRemove,
    eUnknown
} action_e;

/** Bazaar actions with names */
typedef struct {
    action_e act;
    char *name;
} bazaar_acts_t;

ngx_int_t rp_bazaar_cmd_handler(ngx_http_request_t *r);

/* Command calls */
int rp_bazaar_help(ngx_http_request_t *r, 
                   cJSON **json_root, int argc, char **argv);
int rp_bazaar_apps(ngx_http_request_t *r,
                       cJSON **json_root, int argc, char **argv);
int rp_bazaar_start(ngx_http_request_t *r, 
                    cJSON **json_root, int argc, char **argv);
int rp_bazaar_stop(ngx_http_request_t *r, 
                   cJSON **json_root, int argc, char **argv);


int rp_bazaar_install(ngx_http_request_t *r);
int rp_bazaar_interpret(ngx_http_request_t *r, action_e *action);
void rp_bazaar_post_read(ngx_http_request_t *r);


#endif /*__RP_BAZAAR_H*/
