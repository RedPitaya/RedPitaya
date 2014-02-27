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

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_log.h>

#include "ngx_http_rp_module.h"
#include "rp_data_cmd.h"
#include "cJSON.h"

/* last good result container */
static float **rp_signals = NULL;
static int     rp_signals_dirty = 0;

#define TRACE(args...) fprintf(stderr, args)


/*----------------------------------------------------------------------------*/
/* request private context, used to shared data between different callback functions
 * over the same request
 */
typedef struct rp_data_ctx_s {
    cJSON *json_root;
    int    finalize_on_post_handler;
} rp_data_ctx_t;


/*----------------------------------------------------------------------------*/
/**
 * @brief Handler function for /data GET & POST requests.
 *
 * This function is indirectly called by HTTP NGINX server to handle GET & POST operation
 * for the "data" request.
 *
 * @retval NGX_HTTP_NOT_ALLOWED            The required operation is not allowed
 * @retval NGX_HTTP_INTERNAL_SERVER_ERROR  Failure while allocating JSON package
 * @retval other                           GET: returned value from rp_module_send_response() function
 *                                         POST: returned value from ngx_http_read_client_request_body() or NGX_DONE
 */

ngx_int_t rp_data_cmd_handler(ngx_http_request_t *r)
{
    cJSON *json_root, *data_root, *app_root;
    int ret_val = 0;

    if(!(r->method & (NGX_HTTP_GET|NGX_HTTP_POST))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rp_debug(r->connection->log, "%s: %s", __FUNCTION__,
             (r->method & NGX_HTTP_GET) ? "GET" : "POST");

    json_root = cJSON_CreateObject(r->pool);
    if(json_root == NULL) {
        rp_error(r->connection->log, "Can not allocate cJSON object");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    cJSON_AddItemToObject(json_root, "app",
                          app_root=cJSON_CreateObject(r->pool), r->pool);
    if(app_root == NULL) {
        rp_error(r->connection->log, "Can not allocate cJSON object");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    cJSON_AddItemToObject(json_root, "datasets",
                          data_root=cJSON_CreateObject(r->pool), r->pool);
    if(data_root == NULL) {
        rp_error(r->connection->log, "Can not allocate cJSON object");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if(!rp_module_ctx.app.handle) {
        rp_error(r->connection->log, "Application not loaded");
        rp_module_cmd_error(&json_root, "Application not loaded", NULL, 
                            r->pool);
        return rp_module_send_response(r, &json_root);
    }

    char *app_id = rp_module_ctx.app.id;
    if (!app_id) {
        app_id = "unknown";
    }
    cJSON_AddItemToObject(app_root, "id",
                          cJSON_CreateString(app_id, r->pool), r->pool);
    
    if(r->method & NGX_HTTP_POST) {
        /* we continue with the answer in rp_data_post_read. the
         * json package into which the answer is stored is passed
         * through request private buffer.
         */
        ngx_int_t rc;
        rp_data_ctx_t *ctx;

        ctx = ngx_pcalloc(r->pool, sizeof(rp_data_ctx_t));
        if (ctx == NULL) {
              return NGX_ERROR;
        }
        ctx->json_root = json_root;
        ctx->finalize_on_post_handler = 0;
        ngx_http_set_ctx(r, ctx, ngx_http_rp_module);

        rc = ngx_http_read_client_request_body(r, rp_data_post_read);

        if(rc == NGX_AGAIN) {
            ctx->finalize_on_post_handler = 1;
        }
        return rc;
    }

    ret_val = rp_data_get_signals(r, &json_root);
    rp_data_get_params(r, &json_root);

    if(ret_val == 0) {
        rp_module_cmd_ok(&json_root, r->pool);
    } else {
        rp_module_cmd_again(&json_root, r->pool);
    }
    return rp_module_send_response(r,  &json_root);
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Invoke application specific parameter set operation.
 *
 * Function parses the POST request, which is defined by JSON packet. This packet
 * must contain "params" request, embedded within "datasets" entity. After the
 * request packet is successfully parsed, the request action is passed to the
 * rp_data_parse_and_set_params() function
 *
 * @param[in]  r          HTTP request as defined by NGINX framework
 * @param[in]  json_root  pointer to JSON root node, to which in case of error the reason description is appended
 * @param[in]  in_buffer  already parsed json package
 * @retval     0          successful operation
 * @retval     <0         failure, error code is defined by rp_module_cmd_error()
 */
static int rp_data_set_params(ngx_http_request_t *r, cJSON **json_root, char *in_buffer)
{
    cJSON *req_body, *j_params, *j_data;

    /* check if this is a POST operation */
    if(!(r->method & NGX_HTTP_POST)) {
        return rp_module_cmd_error(json_root, "Expected POST method",
                                   NULL, r->pool);
    }

    /* check for payload buffers */
    if(in_buffer == NULL) {
        return rp_module_cmd_error(json_root, "Body is empty, unknown error",
                                   NULL, r->pool);
    }

    rp_debug(r->connection->log, "Received body: %s", in_buffer);

    /* parse incoming body to cJSON and search for 'params' */
    req_body = cJSON_Parse((const char *)in_buffer, r->pool);
    if(req_body == NULL) {
        return rp_module_cmd_error(json_root,
                                   "Can not parse incoming body to JSON", NULL,
                                   r->pool);
    }

    /* pull out "datasets" object */
    j_data = cJSON_GetObjectItem(req_body, "datasets");
    if(j_data == NULL) {
        cJSON_Delete(req_body, r->pool);
        return rp_module_cmd_error(json_root,
                                   "Can not find 'data' in req body", NULL,
                                   r->pool);
    }

    /* and refer to its child object "params" */
    j_params = cJSON_GetObjectItem(j_data, "params");
    if(j_params == NULL) {
        cJSON_Delete(req_body, r->pool);
        return rp_module_cmd_error(json_root,
                                   "Can not find 'params' in req body", NULL,
                                   r->pool);
    }

    /* parse the "params" JSON object and execute the request */
    if(rp_data_parse_and_set_params(j_params) < 0) {
        cJSON_Delete(req_body, r->pool);
        return rp_module_cmd_error(json_root,
                                   "Setting new parameters failed", NULL,
                                   r->pool);
    }

    /* release our resources */
    cJSON_Delete(req_body, r->pool);

    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Handler function for POST operation.
 *
 *
 * @param r http request as defined by nxinx framework
 */
void rp_data_post_read(ngx_http_request_t *r)
{
    int len = 0, buffers = 0;
    char *msg_pos;
    ngx_chain_t *chain_link;
    rp_data_ctx_t *ctx;
    char *in_buffer;

    ctx = ngx_http_get_module_ctx(r, ngx_http_rp_module);

    if(ctx == NULL) {
        rp_error(r->connection->log, 
                 "rp_data_post_read() JSON root structure not set");
        goto done;
    }

    /* check if request body is specified */
    if((r->request_body == NULL) || (r->request_body->bufs == NULL)) {
        rp_error(r->connection->log, 
                 "rp_data_post_read() body is empty");
        goto done;
    }
    
    /* we do not support request body specified in a file */
    if(r->request_body->temp_file) {
        rp_error(r->connection->log, "rp_data_post_read() data in temp "
                 "file (not supported - check client_body_buffer_size parameter");
        goto done;
    }

    /* calculate the size of all buffers in the request body, since we have to
     * concatenate them in a single buffer */
    len=0;
    for(chain_link = r->request_body->bufs; chain_link != NULL; 
        chain_link = chain_link->next) {
        len += chain_link->buf->last - chain_link->buf->pos;
        buffers++;
        if(chain_link->buf->in_file) {
            rp_error(r->connection->log, "rp_data_post_read() data in "
                     "file (not supported - check client_body_buffer_size"
                     "parameter");
            goto done;
        }
    }

    /* allocate memory for the buffer of the body */
    in_buffer = (char *)ngx_palloc(r->pool, (len + 1)*sizeof(char));
    if(in_buffer == NULL) {
        rp_error(r->connection->log, "rp_data_post_read() can not "
                 "allocate memory");
        goto done;
    }

    /* collect body into one buffer */
    msg_pos = in_buffer;
    for(chain_link = r->request_body->bufs; chain_link != NULL; chain_link = chain_link->next) {
        ngx_buf_t *buf = chain_link->buf;
        msg_pos = (char *)ngx_copy(msg_pos, (char *)buf->pos, buf->last - buf->pos);
    }
    in_buffer[len] = '\0';

    if(rp_data_set_params(r, &ctx->json_root, in_buffer) < 0) {
        rp_error(r->connection->log, "rp_data_set_params() failed");
        goto done;
    }

    /* modify json package to return all actual parameters to the client */
    rp_data_get_params(r, &ctx->json_root);

    /* append succesful flag to the json package */
    rp_module_cmd_ok(&ctx->json_root, r->pool);

    /* and send the response to the client */
    rp_module_send_response(r, &ctx->json_root);
done:
    if (ctx->finalize_on_post_handler) {
        ngx_http_finalize_request(r, NGX_DONE);
    }
    return;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Handler function for POST operation.
 *
 * Function copies the specified list of parameters into temporary
 * allocated list of parameters and calls the application specific POST operation
 *
 * @param[in]  params_root  pointer to JSON object, holding a list of parameters to be applied
 * @retval     0            success
 * @retval    -1            failure
 */
int rp_data_parse_and_set_params(cJSON *params_root)
{
    int rp_params_cnt = 0, i, ret_val;
    rp_app_params_t *rp_params;
    struct cJSON *j_params;

    if(params_root == NULL)
        return -1;

    /* count the number of specified parameters to be set */
    j_params = params_root->child;
    while(j_params != NULL) {
        if(j_params->type != cJSON_Number)
            continue;
        j_params = j_params->next;
        rp_params_cnt++;
    }

    /* allocate temporary storage as an array of sequential parameters */
    rp_params = (rp_app_params_t *)malloc((rp_params_cnt+1) * sizeof(rp_app_params_t));
    if(rp_params == NULL)
        return -1;

    /* scan the specified JSON object and copy request parameters in our temporary storage */
    j_params = params_root->child;
    for(i = 0; i < rp_params_cnt || j_params != NULL; i++) {
        if((j_params->string == NULL) || (j_params->type != cJSON_Number)) {
            j_params = j_params->next;
            continue;
        }

        rp_params[i].name = (char *)malloc(strlen(j_params->string)+1);
        strncpy(&rp_params[i].name[0], &j_params->string[0],
                strlen(j_params->string));
        rp_params[i].name[strlen(j_params->string)] = '\0';
        rp_params[i].value = (float)j_params->valuedouble;

        j_params = j_params->next;
    }
    rp_params[rp_params_cnt].name = NULL;

    /* call application specific function for handling POST requests */
    ret_val = 0;
    if(rp_module_ctx.app.set_params_func(rp_params, rp_params_cnt) < 0) {
        ret_val = -1;
    }

    /* deallocate temporary storage */
    for(i = 0; i < rp_params_cnt; i++) {
        if(rp_params[i].name)
            free(rp_params[i].name);
    }
    if(rp_params)
        free(rp_params);

    /* ... and return the success or failure code */
    return ret_val;
}


/*----------------------------------------------------------------------------*/
int rp_data_get_params(ngx_http_request_t *r, cJSON **json_root)
{
    rp_app_params_t *rp_params = NULL;
    int rp_params_cnt;
    cJSON *j_params, *data_root;
    int i;
    
    data_root = cJSON_GetObjectItem(*json_root, "datasets");
    if(data_root == NULL) {
        return rp_module_cmd_error(json_root, 
                                   "Can not find 'data'", NULL, 
                                   r->pool);
    }

    /* Now prepare the answer with set parameters */
    rp_params_cnt = rp_module_ctx.app.get_params_func(&rp_params);
    if(rp_params == NULL) {
        return rp_module_cmd_error(json_root, "Can not retrieve parameters.",
                                   NULL, r->pool);
    }

    cJSON_AddItemToObject(data_root, "params",
                          j_params=cJSON_CreateObject(r->pool), r->pool);

    for(i = 0; i < rp_params_cnt; i++) {
        cJSON_AddItemToObject(j_params, rp_params[i].name,
                              cJSON_CreateNumber(rp_params[i].value, r->pool), 
                              r->pool);
    }

    for(i = 0; i < rp_params_cnt; i++) {
        if(rp_params[i].name)
            free((char *)rp_params[i].name);
    }
    if(rp_params)
        free(rp_params);

    return 0;
}


/*----------------------------------------------------------------------------*/
int rp_data_get_signals(ngx_http_request_t *r, cJSON **json_root)
{
    int rp_sig_num, rp_sig_len, ret_val;
    cJSON *data_root, *sig_root, *d1, *d2, *g1;
    /* TODO: Make it configurable */
    int retries = 200; /* Approx in [ms] */

    if(rp_signals == NULL) {
        int i;
        rp_signals = (float **)malloc(3 * sizeof(float *));
        for(i = 0; i < 3; i++) {
            rp_signals[i] = (float *)malloc(2048 * sizeof(float));
        }
    }

    data_root = cJSON_GetObjectItem(*json_root, "datasets");
    if(data_root == NULL) {
        return rp_module_cmd_error(json_root, 
                                   "Can not find 'data'", NULL, 
                                   r->pool);
    }
    
    ret_val =
        rp_module_ctx.app.get_signals_func((float ***)&rp_signals, &rp_sig_num, 
                                           &rp_sig_len);

    while(ret_val == -1) {
        ret_val =
            rp_module_ctx.app.get_signals_func((float ***)&rp_signals, 
                                               &rp_sig_num, &rp_sig_len);

        if(ret_val == -2) 
            break;
        if(retries-- <= 0) {
            /* Use old signals */
            break;
        } else {
            usleep(1000);
        }
    }
    ret_val = ret_val;
    /* In case we are repeating the transmission */
    if((rp_signals_dirty == 0) && (ret_val == -1))
        ret_val = 0;
    rp_signals_dirty = 1;

    cJSON_AddItemToObject(data_root, "g1",
                          g1=cJSON_CreateArray(r->pool), r->pool);

    cJSON_AddItemToObject(g1, "g1", 
                          sig_root=cJSON_CreateObject(r->pool), r->pool);
    cJSON_AddItemToObject(sig_root, "data",
                   d1=cJSON_Create2dFloatArray(&rp_signals[0][0], &rp_signals[1][0],
                                               rp_sig_len, r->pool),
                          r->pool);
    cJSON_AddItemToObject(g1, "g1", 
                          sig_root=cJSON_CreateObject(r->pool), r->pool);
    cJSON_AddItemToObject(sig_root, "data",
                   d2=cJSON_Create2dFloatArray(&rp_signals[0][0], &rp_signals[2][0],
                                               rp_sig_len, r->pool),
                          r->pool);

    return ret_val;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Clear Signal Dirty flag
 */
void rp_data_clear_signals_dirty()
{
    rp_signals_dirty = 0;
}
