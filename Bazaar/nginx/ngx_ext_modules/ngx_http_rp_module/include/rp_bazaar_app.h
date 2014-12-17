/**
 * $Id$
 *
 * @brief Red Pitaya Nginx module - Bazaar.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_BAZAAR_APP_H
#define __RP_BAZAAR_APP_H

#include <stdio.h>
#include "cJSON.h"

/** Structure which describes parameters supported by the application.
 * Each application includes an parameters table which includes the following
 * information:
 *    - name - name of the parameter, which can be accessed from the client
 *    - value - value of the parameter
 *    - fpga_update - whether this parameter changes the FPGA content
 *    - read_only - if this parameter is read only all changes from the client
 *                  will be ignored
 *    - min_val - minimal valid value for the parameter
 *    - max_val - maximal valid value for the parameter
 *
 * NOTE: During set_params() only value can be changed, other parameters are
 *       read-only and can not be changed from the client.
 * TODO: Make the header accessible to the both - application & webserver
 **/
typedef struct rp_app_params_s {
    char  *name;
    float  value;
    int    fpga_update;
    int    read_only;
    float  min_val;
    float  max_val;
} rp_app_params_t;

/* Functions & Structure which defines the application interface.
 * In application function with the same name and the same declaration must
 * be provided. For example: int rp_app_init(void);
 */
/* Mandatory functions: */
typedef int          (*rp_app_init_func)(void);
typedef int          (*rp_app_exit_func)(void);
typedef const char  *(*rp_app_desc_func)(void);
typedef int          (*rp_set_params_func)(rp_app_params_t *p, int len);
typedef int          (*rp_get_params_func)(rp_app_params_t **p);
typedef int          (*rp_get_signals_func)(float ***s, int *sig_num, 
                                            int *sig_len);

typedef struct rp_bazaar_app_s {
    /* Initialization function - called when app. is loaded */
    rp_app_init_func init_func;
    /* Exit/cleanup function - called when app. is unloaded */
    rp_app_exit_func exit_func;
    /* Description function - provides short description of the application
     */
    rp_app_desc_func desc_func;

    /* Application ID (application's top directory name) */
    char            *id;

    /* Sets the parameter in the application */
    rp_set_params_func       set_params_func;
    /* Retrieves current parameter settings from the application */
    rp_get_params_func       get_params_func;
    /* Retrieves last good signals from the application */
    rp_get_signals_func      get_signals_func;

    /* Dynamic library handle */
    void            *handle;
    char            *file_name;

    /* flag indicating init_func has been called  */
    int              initialized;
} rp_bazaar_app_t;

/* FPGA housekeeping structure */
typedef struct hk_fpga_reg_mem_s {
    /* configuration:
     * bit   [3:0] - hw revision
     * bits [31:4] - reserved
     */
    uint32_t rev;
    /* DNA low word */
    uint32_t dna_lo;
    /* DNA high word */
    uint32_t dna_hi;
} hk_fpga_reg_mem_t;


int rp_bazaar_app_get_local_list(const char *dir, cJSON **json_root, 
                                 ngx_pool_t *pool, int verbose);
int rp_bazaar_app_load_module(const char *app_file, rp_bazaar_app_t *app);
int rp_bazaar_app_unload_module(rp_bazaar_app_t *app);
int rp_bazaar_app_load_fpga(const char *fpga_file);
int rp_bazaar_get_mac(const char* nic, char *mac);
int rp_bazaar_get_dna(unsigned long long *dna);
int get_info(cJSON **info, const char *dir, const char *app_id, ngx_pool_t *pool);

#endif /*__RP_BAZAAR_APP_H*/
