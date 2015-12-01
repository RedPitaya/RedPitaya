/**
 * $Id$
 *
 * @brief Red Pitaya Nginx module - Bazaar interface.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_log.h>

#include "redpitaya/http.h"
#include "ngx_http_rp_module.h"
#include "rp_bazaar_cmd.h"
#include "rp_bazaar_app.h"
#include "cJSON.h"
#include <ws_server.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

/** The list of available Bazaar commands */
rp_module_cmd_t bazaar_cmds[] = {
    { "arg_help", "",
      "Returns the command descriptions.",
      &rp_bazaar_help },
    { "arg_apps", "",
      "Returns the list of installed applications.",
      &rp_bazaar_apps },
    { "arg_start", "<app_name>",
      "Starts the application defined by <app_name>.",
      &rp_bazaar_start },
    { "arg_stop", "",
      "Stops the currently running application.",
      &rp_bazaar_stop },
    { "arg_install", "<app_name>",
      "Installs the application <app_name> from Bazaar.",
      NULL },
    { "arg_remove", "<app_name>",
      "Removes the application <app_name>.",
      NULL },

    /* Must be last*/
    { NULL, NULL, NULL, NULL }
}; /* bazaar_cmds */

/** Bazaar actions mapping */
bazaar_acts_t bazaar_acts[] = {
    { eInstall, "install" },
    { eUpgrade, "upgrade" },
    { eRemove,  "remove"  },
    { eUnknown, "unknown" }
};

/** Redirection URL max length */
#define c_redirect_len 256

/** Bazaar token string */
#define c_token_len    256
char g_token[c_token_len];

/*----------------------------------------------------------------------------*/
/* request private context, used to share data between different callback functions
 * over the same request
 */
typedef struct rp_bazaar_ctx_s {
    cJSON *json_root;
    int    finalize_on_post_handler;
    char  *redirect;
    char  *in_buffer;
    size_t in_buffer_len;
    int    in_status;
} rp_bazaar_ctx_t;


/*----------------------------------------------------------------------------*/
ngx_int_t rp_bazaar_cmd_handler(ngx_http_request_t *r)
{
    ngx_http_rp_loc_conf_t *lc;
    rp_bazaar_ctx_t *ctx;
    size_t i = 0;

    ctx = ngx_pcalloc(r->pool, sizeof(rp_bazaar_ctx_t));
    if (ctx == NULL) {
        return NGX_ERROR;
    }
    ctx->redirect = ngx_pcalloc(r->pool, c_redirect_len);
    if (ctx->redirect == NULL) {
        return NGX_ERROR;
    }
    ctx->json_root = NULL;
    ctx->finalize_on_post_handler = 0;
    ctx->in_buffer = NULL;
    ctx->in_buffer_len = 0;
    ctx->in_status = 0;
    ngx_http_set_ctx(r, ctx, ngx_http_rp_module);

    lc = ngx_http_get_module_loc_conf(r, ngx_http_rp_module);

    /* Just test local directory here - we would need to check it for almost
     * all calls anyway
     */
    if(lc->bazaar_dir.data == NULL) {
        rp_error(r->connection->log, "Bazaar local directory not found");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ctx->json_root = cJSON_CreateObject(r->pool);
    if(ctx->json_root == NULL) {
        rp_error(r->connection->log, "Cannot allocate cJSON object");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    /* Bazaar commands */
    for(i = 0; bazaar_cmds[i].name != NULL; i++) {

        ngx_str_t arg_name = { strlen(bazaar_cmds[i].name),
                               (u_char *)bazaar_cmds[i].name };
        ngx_uint_t arg_key = ngx_hash_key(arg_name.data, arg_name.len);
        ngx_http_variable_value_t *arg_val = NULL;

        int rc;
        char **arg_argv = NULL;
        int    arg_argc = 0;

        arg_val = ngx_http_get_variable(r, &arg_name, arg_key);

        /* check validity of the specified http variables
         * note: not all attributes of arg_value are set within ngx_http_get_variable, thus
         * the order of the following checks are important otherwise it could come to the usage
         * of uninitialized value what would lead to invalid processing
         */
        if (!arg_val)                continue;
        if (arg_val->not_found == 1) continue;
        if (arg_val->valid == 0)     continue;

        arg_val->data[arg_val->len] = '\0';
        arg_argc = rp_module_cmd_parse_args((const char *)arg_val->data,
                                            arg_val->len,
                                            &arg_argv);

        rp_debug(r->connection->log, "Calling application: %s\n", &bazaar_cmds[i].name[4]);

        /* Install/Remove special case */
        if ( !strncmp(&bazaar_cmds[i].name[4], "install", 7) ||
             !strncmp(&bazaar_cmds[i].name[4], "remove",  6) ) {
            if(arg_argv) {
                for(i = 0; i < (size_t)arg_argc; i++) {
                    if(arg_argv[i]) {
                        free(arg_argv[i]);
                        arg_argv[i] = NULL;
                    }
                }
                free(arg_argv);
            }
            return rp_bazaar_install(r);
        }

        if((rc = bazaar_cmds[i].func(r, &ctx->json_root, arg_argc, arg_argv)) < 0) {
            /* error - fill the output buffer and send it back */
            rp_error(r->connection->log, "Application %s failed: %d\n",
                     bazaar_cmds[i].name, rc);
        }

        if(arg_argv) {
            for(i = 0; i < (size_t)arg_argc; i++) {
                if(arg_argv[i]) {
                    free(arg_argv[i]);
                    arg_argv[i] = NULL;
                }
            }
            free(arg_argv);
        }

        /* Prepare response header & body */
        return rp_module_send_response(r, &ctx->json_root);
    }

    int ret = 0;

    /* Unknown command response */
    if (i >= sizeof(bazaar_cmds)/sizeof(rp_module_cmd_t)) {
        rp_module_cmd_error(&ctx->json_root, "Unknown command.", NULL, r->pool);
        return rp_module_send_response(r, &ctx->json_root);
    }

    /* Default Bazaar entry point - list of applications with versions */
    cJSON *json_tok = NULL;

    char *host = (char *)r->headers_in.server.data;
    char mac[18];
    sprintf(mac, "00:00:00:00:00:00");
    if (rp_bazaar_get_mac("/sys/class/net/eth0/address", mac)) {
        rp_error(r->connection->log, "Cannot obtain MAC address.");
    }
    unsigned long long dna = 0;
    if (rp_bazaar_get_dna(&dna)) {
        rp_error(r->connection->log, "Cannot obtain DNA number.");
    }
    char dna_s[64];
    sprintf(dna_s, "%016llx", dna);

    /* Get Ecosystem version */
    char ecoversion[64];
    sprintf(ecoversion, "unknown");
    cJSON *ecoinfo = NULL;
    if (!get_info(&ecoinfo, (const char *)lc->bazaar_dir.data,
                  "", r->pool)) {
        rp_error(r->connection->log, "Cannot obtain Ecosystem version.");
    } else {
        if (ecoinfo != NULL) {

            cJSON *j_ver = cJSON_GetObjectItem(ecoinfo, "version");
            if(j_ver == NULL) {
                rp_error(r->connection->log, "Cannot get version from ecoinfo JSON.\n");
            } else {
                strncpy(ecoversion, j_ver->valuestring, sizeof ecoversion);
                cJSON_Delete(j_ver, r->pool);
            }

            cJSON_Delete(ecoinfo, r->pool);
        }
    }

    /* Populate JSON */
    cJSON_AddItemToObject(ctx->json_root, "version",
                          cJSON_CreateString(ecoversion, r->pool),
                          r->pool);

    cJSON_AddItemToObject(ctx->json_root, "dna",
                          cJSON_CreateString(dna_s, r->pool), r->pool);

    cJSON_AddItemToObject(ctx->json_root, "mac",
                          cJSON_CreateString(mac, r->pool), r->pool);

    // TODO: Serial number?

    cJSON_AddItemToObject(ctx->json_root, "host",
                          cJSON_CreateString(host, r->pool),
                          r->pool);

    cJSON *apps_root;
    cJSON_AddItemToObject(ctx->json_root, "apps",
                          apps_root=cJSON_CreateObject(r->pool), r->pool);
    if(apps_root == NULL) {
        rp_error(r->connection->log, "Can not allocate cJSON object");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    /* Add the non-verbose list of apps */
    rp_bazaar_app_get_local_list((const char *)lc->bazaar_dir.data,
                                 &apps_root, r->pool, 0);

    /* Issue a POST with JSON defined above to Bazaar server */
    char *js = cJSON_Print(ctx->json_root, r->pool);
    cJSON_Minify(js);

    rp_debug(r->connection->log, "Bazaar handshake:\n%s", js);

    char *jse = NULL;
    char *jsp = NULL;
    char *bazaar_dv = NULL;
    const char *c_payload = "payload=";
    const char *c_device = "/device";

    jse = url_encode(js);

    jsp = malloc(strlen(jse) + strlen(c_payload) + 1);
    if (!jsp) {
        rp_error(r->connection->log, "Cannot malloc() payload.");
        ret = NGX_HTTP_INTERNAL_SERVER_ERROR;
        goto out;
    }
    sprintf(jsp, "%s%s", c_payload, jse);

    bazaar_dv = (char *)malloc(strlen((char *)lc->bazaar_server.data) +
                               strlen(c_device) + 1);
    if (!bazaar_dv) {
        rp_error(r->connection->log, "Cannot malloc() device.");
        ret = NGX_HTTP_INTERNAL_SERVER_ERROR;
        goto out;
    }
    sprintf(bazaar_dv, "%s%s", lc->bazaar_server.data, c_device);

    post_resp_t resp = { NULL, 0 };
    if (post(jsp, bazaar_dv, &resp)) {
        if (resp.data) free(resp.data);
        /* Redirect to Bazaar access error */
        rp_error(r->connection->log, "Cannot access %s.\n", bazaar_dv);
        snprintf(ctx->redirect, c_redirect_len,
                 "http://%s/error_bazaar_access.html", host);
        goto out;
    }

    /* Get token from POST response */
    if (resp.data) {

        rp_debug(r->connection->log,"Bazaar handshake response:\n%s", resp.data);

        json_tok = cJSON_Parse(resp.data, r->pool);
        if (!json_tok) {
            /* Redirect to Bazaar protocol error */
            rp_error(r->connection->log, "Bazaar protocol error: ",
                     "No JSON found in the following response:\n\"%s\"\n", resp.data);
            snprintf(ctx->redirect, c_redirect_len,
                     "http://%s/error_bazaar_proto.html", host);
        } else {

            cJSON *jtok = cJSON_GetObjectItem(json_tok, "token");

            if (!jtok) {
                /* Redirect to Bazaar protocol error */
                rp_error(r->connection->log, "Bazaar protocol error: ",
                         "No token found in the following JSON:\n\"%s\"\n", resp.data);
                snprintf(ctx->redirect, c_redirect_len,
                         "http://%s/error_bazaar_proto.html", host);
            } else {
                /* Redirect to Bazaar with token */
                snprintf(ctx->redirect, c_redirect_len, "%s/token/%s",
                        lc->bazaar_server.data, jtok->valuestring);
                /* Store token for session control */
                strncpy(g_token, jtok->valuestring, c_token_len);
                g_token[c_token_len - 1] = '\0';
            }
        }

        free(resp.data);
    }

 out:
    if (jsp) free(jsp);
    if (jse) free(jse);
    if (bazaar_dv) free(bazaar_dv);
    //TODO: Is ctx->json_root handled by the pool deallocator?
    //if (ctx->json_root) cJSON_Delete(ctx->json_root, r->pool);
    if (json_tok) cJSON_Delete(json_tok, r->pool);

    if (ret) return ret;

    rp_debug(r->connection->log, "Redirecting to: %s", ctx->redirect);
    return rp_module_redirect(r, ctx->redirect);
}


/*----------------------------------------------------------------------------*/
int rp_bazaar_help(ngx_http_request_t *r, cJSON **json_root,
                   int argc, char **argv)
{
    cJSON *root = *json_root;
    int i;

    if(argc != 0) {
        return rp_module_cmd_error(json_root,
                                "Incorrect number of arguments (should be 0)",
                                   NULL, r->pool);
    }

    for(i = 0; bazaar_cmds[i].name != NULL; i++) {
        /* omit arg_ in the name */
        const char *strings[] = { (const char *)&bazaar_cmds[i].name[4],
                                  (const char *)bazaar_cmds[i].params,
                                  (const char *)bazaar_cmds[i].desc };

        cJSON_AddItemToObject(root, "command",
                              cJSON_CreateStringArray((const char **)strings,
                                                      3, r->pool),
                              r->pool);
    }

    return rp_module_cmd_ok(json_root, r->pool);
}

/*----------------------------------------------------------------------------*/
int rp_bazaar_apps(ngx_http_request_t *r,
                             cJSON **json_root, int argc, char **argv)
{
    ngx_http_rp_loc_conf_t *lc =
        ngx_http_get_module_loc_conf(r, ngx_http_rp_module);

    /* Get the verbose list of apps */
    return rp_bazaar_app_get_local_list((const char *)lc->bazaar_dir.data,
                                        json_root, r->pool, 1);
}

/*----------------------------------------------------------------------------*/
int rp_bazaar_start(ngx_http_request_t *r,
                    cJSON **json_root, int argc, char **argv)
{
    int demo = 0;
    char* url = strstr(argv[0], "?type=demo");
    if (url)
    {
        *url = '\0';
        demo = 1;
    }
    else
    {
       url = strstr(argv[0], "?type=run");
       if(url)
            *url = '\0';
    }

    int unsigned len;
    ngx_http_rp_loc_conf_t *lc =
        ngx_http_get_module_loc_conf(r, ngx_http_rp_module);

    if(argc != 1) {
        return rp_module_cmd_error(json_root,
                                "Incorrect number of arguments (should be 1)",
                                   NULL, r->pool);
    }

    /* Check if application is already running and unload it if so. */
    if(rp_module_ctx.app.handle != NULL) {
        if(rp_bazaar_app_unload_module(&rp_module_ctx.app)) {
            return rp_module_cmd_error(json_root,
                                       "Can not unload existing application.",
                                       NULL, r->pool);
        }
    }

    /* Application id string */
    len = strlen(argv[0]) + 1;
    rp_module_ctx.app.id = (char *)malloc(len);
    if(rp_module_ctx.app.id == NULL) {
        return rp_module_cmd_error(json_root, "Can not allocate memory",
                                   strerror(errno), r->pool);
    }
    strcpy(rp_module_ctx.app.id, argv[0]);

    /* Assemble the application and FPGA filename: <app_dir>/<app_id>/controllerhf.so */
    len = strlen((char *)lc->bazaar_dir.data) + strlen(argv[0]) + strlen("/controllerhf.so") + 2;
    char app_name[len];
    sprintf(app_name, "%s/%s/controllerhf.so", lc->bazaar_dir.data, argv[0]);
    app_name[len-1]='\0';

    /* Unload existing application before, new fpga load */
    if(rp_module_ctx.app.handle != NULL){
        if(rp_bazaar_app_unload_module(&rp_module_ctx.app)){
            return rp_module_cmd_error(json_root,
                                       "Cannot unload existing application.",
                                       NULL, r->pool);
        }
    }

    /* Get FPGA config file in <app_dir>/<app_id>/fpga.conf */
    char *fpga_name = NULL;
    if(get_fpga_path((const char *)argv[0], (const char *)lc->bazaar_dir.data, &fpga_name) == 0) {
        /* Here we do not have application running anymore - load new FPGA */
        rp_debug(r->connection->log, "Loading specific FPGA from: '%s'\n", fpga_name);
        /* Try loading FPGA code
         *    - Test if fpga loaded correctly
         *    - Read/write permissions
         *    - File exists/not exists */
        switch (rp_bazaar_app_load_fpga(fpga_name)) {
            case FPGA_FIND_ERR:
                if (fpga_name)  free(fpga_name);
                return rp_module_cmd_error(json_root, "Cannot find fpga file.", NULL, r->pool);
            case FPGA_READ_ERR:
                if (fpga_name)  free(fpga_name);
                return rp_module_cmd_error(json_root, "Unable to read FPGA file.", NULL, r->pool);
            case FPGA_WRITE_ERR:
                if (fpga_name)  free(fpga_name);
                return rp_module_cmd_error(json_root, "Unable to write FPGA file into memory.", NULL, r->pool);
            /* App is a new app and doesn't need custom fpga.bit */
            case FPGA_NOT_REQ:
                if (fpga_name)  free(fpga_name);
                break;
            case FPGA_OK:
                if (fpga_name)  free(fpga_name);
                break;
            default:
                if (fpga_name)  free(fpga_name);
                return rp_module_cmd_error(json_root, "Unknown error.", NULL, r->pool);
        }
    } else {
        rp_debug(r->connection->log, "Not loading specific FPGA, since no fpga.conf file was found.\n");
    }

    /* Load new application. */
    rp_debug(r->connection->log, "Loading application: '%s'\n", app_name);
    if(rp_bazaar_app_load_module(&app_name[0], &rp_module_ctx.app) < 0) {
        rp_bazaar_app_unload_module(&rp_module_ctx.app);
        return rp_module_cmd_error(json_root, "Can not load application.",
                                   NULL, r->pool);
    }

    if(rp_module_ctx.app.init_func() < 0) {
        rp_module_cmd_error(json_root,
                            "Application init failed, aborting",
                            NULL, r->pool);
        rp_bazaar_app_unload_module(&rp_module_ctx.app);
        return -1;
    }
    rp_module_ctx.app.initialized=1;
    rp_debug(r->connection->log, "Application %s loaded succesfully!");

    //start web socket server
    if(rp_module_ctx.app.ws_api_supported)
    {
        struct server_parameters params;
        ngx_memset(&params, 0, sizeof(struct server_parameters));

        params.set_params_interval_func = rp_module_ctx.app.ws_set_params_interval_func;
        params.set_signals_interval_func = rp_module_ctx.app.ws_set_signals_interval_func;
        params.get_params_interval_func = rp_module_ctx.app.ws_get_params_interval_func;
        params.get_signals_interval_func = rp_module_ctx.app.ws_get_signals_interval_func;
        params.get_params_func = rp_module_ctx.app.ws_get_params_func;
        params.set_params_func = rp_module_ctx.app.ws_set_params_func;
        params.get_signals_func = rp_module_ctx.app.ws_get_signals_func;
        params.set_signals_func = rp_module_ctx.app.ws_set_signals_func;
        params.gzip_func = rp_module_ctx.app.ws_gzip_func;
        fprintf(stderr, "Starting WS-server\n");

        if (rp_module_ctx.app.verify_app_license_func)
            if (rp_module_ctx.app.verify_app_license_func(argv[0]))
                demo = 1;

        if (demo)
        {
            fprintf(stderr, "Run in demo mode\n");
            rp_module_ctx.app.ws_set_params_demo_func(1);
        }

        start_ws_server(&params);
    }

    return rp_module_cmd_ok(json_root, r->pool);
}

/*----------------------------------------------------------------------------*/
int rp_bazaar_stop(ngx_http_request_t *r,
                   cJSON **json_root, int argc, char **argv)
{
/*    if(argc != 0) {
        return rp_module_cmd_error(json_root,
                                "Incorrect number of arguments (should be 0)",
                                   NULL, r->pool);
    }*/

    if(rp_module_ctx.app.handle == NULL) {
        /* Ignore requests to unload the application controller, if none is loaded. */
        return rp_module_cmd_ok(json_root, r->pool);
    }
    if(rp_bazaar_app_unload_module(&rp_module_ctx.app) < 0) {
        return rp_module_cmd_error(json_root,
                                   "Can not unload application.", NULL, r->pool);
    }

    return rp_module_cmd_ok(json_root, r->pool);
}


/*----------------------------------------------------------------------------*/
int rp_bazaar_install(ngx_http_request_t *r)
{
    rp_bazaar_ctx_t *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_rp_module);
    if(ctx == NULL) {
        rp_error(r->connection->log,
                 "%s: Cannot get request context",
                 __FUNCTION__);
        return NGX_ERROR;
    }

    if(r->method & NGX_HTTP_POST) {

        ngx_int_t rc;
        ctx->finalize_on_post_handler = 0;

        rc = ngx_http_read_client_request_body(r, rp_bazaar_post_read);

        if(rc == NGX_AGAIN) {
            ctx->finalize_on_post_handler = 1;
        } else {
            return rp_module_redirect(r, ctx->redirect);
        }

        return rc;
    }

    return rp_module_cmd_error(&ctx->json_root, "Unsupported request method.",
                               NULL, r->pool);
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Handler function for POST operation.
 *
 *
 * @param r http request as defined by nxinx framework
 */
void rp_bazaar_post_read(ngx_http_request_t *r)
{
    int len = 0, buffers = 0;
    char *msg_pos;
    ngx_chain_t *chain_link;
    rp_bazaar_ctx_t *ctx;

    rp_debug(r->connection->log, "%s", __FUNCTION__);

    ctx = ngx_http_get_module_ctx(r, ngx_http_rp_module);
    if(ctx == NULL) {
        rp_error(r->connection->log,
                 "%s: Cannot get request context",
                 __FUNCTION__);
        goto done;
    }
    ctx->in_status = 0;
    ctx->in_buffer_len = 0;

    if((r->request_body == NULL) || (r->request_body->bufs == NULL)) {
        rp_error(r->connection->log,
                 "%s: body is empty",
                 __FUNCTION__);
        ctx->in_status = -1;
        goto done;
    }

    if(r->request_body->temp_file) {
        rp_error(r->connection->log, "%s: data in temp file "
                 "(not supported - check client_body_buffer_size parameter",
                 __FUNCTION__);
        ctx->in_status = -1;
        goto done;
    }

    /* check the size of the body */
    for(chain_link = r->request_body->bufs; chain_link != NULL;
        chain_link = chain_link->next) {
        len += chain_link->buf->last - chain_link->buf->pos;
        buffers++;
        if(chain_link->buf->in_file) {
            rp_error(r->connection->log, "%s: data in "
                     "file (not supported - check client_body_buffer_size"
                     "parameter",
                     __FUNCTION__);
            ctx->in_status = -1;
            goto done;
        }
    }

    /* allocate memory for the buffer of the body */
    ctx->in_buffer = (char *)ngx_palloc(r->pool, (len + 1)*sizeof(char));
    ctx->in_buffer_len = (len+1);
    if(ctx->in_buffer == NULL) {
        rp_error(r->connection->log, "%s: can not allocate memory",
                 __FUNCTION__);
        ctx->in_status = -1;
        goto done;
    }

    /* collect body into one buffer */
    msg_pos = ctx->in_buffer;
    for(chain_link = r->request_body->bufs; chain_link != NULL;
        chain_link = chain_link->next) {
        ngx_buf_t *buf = chain_link->buf;
        msg_pos =
            (char *)ngx_copy(msg_pos, (char *)buf->pos, buf->last - buf->pos);
    }
    ctx->in_buffer[len] = '\0';

    char *host = (char *)r->headers_in.server.data;
    action_e act = eInstall;
    int ret = rp_bazaar_interpret(r, &act);
    if(ret != 0) {

        /* Redirect to Bazaar installation error */
        rp_error(r->connection->log, "Bazaar %s failed (ret: %d)",
                 bazaar_acts[act].name, ret);
        snprintf(ctx->redirect, c_redirect_len,
                 "http://%s/error_bazaar_install.html", host);
    } else {

        /* Success - redirect back to Bazaar through Red Pitaya */
        snprintf(ctx->redirect, c_redirect_len,
                 "http://%s/bazaar", host);
    }

done:
    if (ctx->finalize_on_post_handler) {
        ngx_http_finalize_request(r, rp_module_redirect(r, ctx->redirect));
    } else {
        ngx_http_finalize_request(r, NGX_DONE);
    }

}



/*----------------------------------------------------------------------------*/
/**
 * @brief Handler function for operation.
 *
 * Function parses the POST request, which is defined by JSON packet and
 * installs/removes the archive.
 *
 * @param[in]  r          HTTP request as defined by NGINX framework
 * @param[out] act        Bazaar action
 * @retval     0          successful operation
 * @retval     !=0        failure
 */
int rp_bazaar_interpret(ngx_http_request_t *r, action_e *act)
{
    rp_bazaar_ctx_t *ctx;
    cJSON *req_body = NULL;
    cJSON *j_cmd = NULL;
    cJSON *j_url = NULL;
    cJSON *j_app = NULL;
    cJSON *j_md5 = NULL;
    cJSON *j_tok = NULL;
    char  *urld = NULL;
    char  *action = NULL;
    int ret = 0;


    rp_debug(r->connection->log, "%s", __FUNCTION__);

    *act = eUnknown;

    ctx = ngx_http_get_module_ctx(r, ngx_http_rp_module);
    if(ctx == NULL) {
        rp_error(r->connection->log,
                 "%s: Cannot get request context",
                 __FUNCTION__);
        return -1;
    }

    /* check if this is a POST operation */
    if(!(r->method & NGX_HTTP_POST)) {
        return rp_module_cmd_error(&ctx->json_root, "Expected POST method",
                                   NULL, r->pool);
    }

    /* check for payload buffers */
    if((ctx->in_buffer_len == 0) ||
       (ctx->in_buffer == NULL) ||
       (ctx->in_status != 0)) {
        rp_error(r->connection->log, "Body is empty, unknown error");
        return -1;
    }

    /* parse incoming body to cJSON and search for 'params' */
    rp_debug(r->connection->log, "Bazaar install/remove: Received body: %s",
             ctx->in_buffer);

    urld = url_decode(ctx->in_buffer);
    cJSON_Minify(urld);

    /* Get rid of "payload=" header */
    char *urld_strip = memchr(urld, '{', 10);

    req_body = cJSON_Parse(urld_strip, r->pool);
    if (urld) free (urld);

    if(req_body == NULL) {
        rp_error(r->connection->log, "Can not parse incoming body to JSON");
        return -1;
    }


    /* "command" object */
    j_cmd = cJSON_GetObjectItem(req_body, "command");
    if(j_cmd == NULL) {
        cJSON_Delete(req_body, r->pool);
        rp_error(r->connection->log, "Can not find 'command' in req body");
        return -1;
    }

    /* Install/Upgrade */
    if ( !strncmp(j_cmd->valuestring, "install", sizeof("install")) ) {
        *act = eInstall;
        action = j_cmd->valuestring;
    }

    if ( !strncmp(j_cmd->valuestring, "upgrade", sizeof("upgrade")) ) {
        *act = eUpgrade;
        action = j_cmd->valuestring;
    }

    if ( (*act == eInstall) || (*act == eUpgrade) ) {
        /* "archive" object */
        j_url = cJSON_GetObjectItem(req_body, "archive");
        if(j_url == NULL) {
            cJSON_Delete(req_body, r->pool);
            rp_error(r->connection->log, "Can not find 'archive' in req body");
            return -1;
        }

        /* "md5" object */
        j_md5 = cJSON_GetObjectItem(req_body, "md5");
        if(j_md5 == NULL) {
            cJSON_Delete(req_body, r->pool);
            rp_error(r->connection->log, "Can not find 'md5' in req body");
            return -1;
        }
    }

    /* Uninstall */
    if (!strncmp(j_cmd->valuestring, "uninstall", sizeof("uninstall"))) {
        *act = eRemove;
        action = "remove";
    }

    /* Unknown action */
    if (!action) {
        cJSON_Delete(req_body, r->pool);
        rp_error(r->connection->log, "Unknown action: %s",
                 j_cmd->valuestring);
        return -1;
    }

    /* "app_id" object */
    j_app = cJSON_GetObjectItem(req_body, "app_id");
    if(j_app == NULL) {
        cJSON_Delete(req_body, r->pool);
        rp_error(r->connection->log, "Can not find 'app_id' in req body");
        return -1;
    }

    /* "token" object */
    j_tok = cJSON_GetObjectItem(req_body, "token");
    if(j_tok == NULL) {
        cJSON_Delete(req_body, r->pool);
        rp_error(r->connection->log, "Can not find 'token' in req body");
        return -1;
    }
    if (strncmp(j_tok->valuestring, g_token, c_token_len)) {
        cJSON_Delete(req_body, r->pool);
        rp_error(r->connection->log, "Unauthorized Bazaar %s request", action);
        return -1;
    }

    /* Call bazaar script */
    char  cmd[256];
    switch (*act) {
    case eInstall:
    case eUpgrade:
        rp_debug(r->connection->log, "Installing: %s", j_url->valuestring);
        sprintf(cmd, "bazaar install %s %s %s",
                j_app->valuestring,
                j_url->valuestring,
                j_md5->valuestring);
        break;

    case eRemove:
        rp_debug(r->connection->log, "Removing: %s", j_app->valuestring);
        sprintf(cmd, "bazaar remove %s",
                j_app->valuestring);
        break;

    default:
        rp_error(r->connection->log, "Unknown Bazaar action: %s",
                 j_cmd->valuestring);
        ret = -1;
        goto out;

    }

    ret = system(cmd);

 out:
    /* release our resources */
    cJSON_Delete(req_body, r->pool);

    return ret;
}
