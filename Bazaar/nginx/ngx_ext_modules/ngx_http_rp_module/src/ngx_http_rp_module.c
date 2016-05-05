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

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ngx_http_rp_module.h"
#include "rp_bazaar_cmd.h"
#include "rp_data_cmd.h"

/* Be careful not to include system headers before Nginx ones!!! */
#include <ctype.h>
#include <unistd.h>

#define XSTR(s) STR(s)
#define STR(s) #s

#ifndef VERSION
  #define VERSION_STR "0.00-0000"
#else
  #define VERSION_STR XSTR(VERSION)
#endif

#ifndef REVISION
  #define REVISION_STR "unknown"
#else
  #define REVISION_STR XSTR(REVISION)
#endif

/* constants */
const char *json_content_str = "application/json";
const char *c_bazaar_dir     = "/opt/redpitaya/www/apps";
const char *c_bazaar_server  = "http://bazaar.redpitaya.com/";
const char *c_tmp_dir        = "/tmp";

const char *c_bazaar_uri = "/bazaar";
const char *c_data_uri   = "/data";

ngx_http_rp_module_ctx_t rp_module_ctx;

/* internal callbacks */
ngx_int_t ngx_http_rp_init_module(ngx_cycle_t *cycle);
void     *ngx_http_rp_create_loc_conf(ngx_conf_t *cf);
char     *ngx_http_rp_merge_loc_conf(ngx_conf_t *cf, void *parent,
                                     void *child);
ngx_int_t ngx_http_rp_init_process(ngx_cycle_t *cycle);
void      ngx_http_rp_exit_process(ngx_cycle_t *cycle);

/* RP callbacks */
char *ngx_http_rp_bazaar_cmd(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


ngx_command_t ngx_http_rp_commands[] = {
    { ngx_string("rp_bazaar_dir"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_rp_loc_conf_t, bazaar_dir),
      NULL },
    { ngx_string("rp_bazaar_server"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_rp_loc_conf_t, bazaar_server),
      NULL },
    { ngx_string("rp_tmp_dir"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_rp_loc_conf_t, tmp_dir),
      NULL },
    { ngx_string("rp_module_cmd"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_MAIN_CONF|NGX_CONF_NOARGS,
      ngx_http_rp_bazaar_cmd,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    ngx_null_command
};

ngx_http_module_t ngx_http_rp_ctx = {
    NULL,                          /* preconfiguration */
    NULL,                          /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_http_rp_create_loc_conf,   /* create location configuration */
    ngx_http_rp_merge_loc_conf     /* merge location configuration */
};


ngx_module_t ngx_http_rp_module = {
    NGX_MODULE_V1,
    &ngx_http_rp_ctx,            /* module context */
    ngx_http_rp_commands,        /* module directives */
    NGX_HTTP_MODULE,             /* module type */
    NULL,                        /* init master */
    ngx_http_rp_init_module,     /* init module */
    NULL,                        /* init process */
    NULL,                        /* init thread */
    NULL,                        /* exit thread */
    NULL,                        /* exit process */
    NULL,                        /* exit master */
    NGX_MODULE_V1_PADDING
};

/*----------------------------------------------------------------------------*/
ngx_int_t ngx_http_rp_init_module(ngx_cycle_t *cycle)
{
    rp_notice(cycle->log, "Initialized ngx_http_rp_module version %s-%s",
        VERSION_STR, REVISION_STR);
    ngx_memset(&rp_module_ctx, 0, sizeof(ngx_http_rp_module_ctx_t));

    rp_module_ctx.log = cycle->log;

    return NGX_OK;
}


/*----------------------------------------------------------------------------*/
void *ngx_http_rp_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_rp_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_rp_loc_conf_t));
    if(conf == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_memset(conf, 0, sizeof(ngx_http_rp_loc_conf_t));

    return conf;
}


/*----------------------------------------------------------------------------*/
char *ngx_http_rp_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_rp_loc_conf_t *prev = parent;
    ngx_http_rp_loc_conf_t *conf = child;
    struct stat stat_buf;

    ngx_conf_merge_str_value(conf->bazaar_dir, prev->bazaar_dir, c_bazaar_dir);
    if ( !access ("/opt/redpitaya/www/conf/testbazaar", 0) )
    {
        FILE * fp;
        fp = fopen("/etc/motd", "r");
        if (fp == NULL)
        {
            ngx_conf_merge_str_value(conf->bazaar_server, prev->bazaar_server, c_bazaar_server);
        }
        else
        {
            char * line = NULL;
            size_t len = 0;
            ssize_t read;
            if((read = getline(&line, &len, fp)) != -1)
            {
                rp_error(cf->log, "Custom URL of bazaar was called!");
                ngx_conf_merge_str_value(conf->bazaar_server, prev->bazaar_server, line);
            }
            else
            {
                ngx_conf_merge_str_value(conf->bazaar_server, prev->bazaar_server, c_bazaar_server);
            }
            fclose(fp);
        }
    } else
    {
        ngx_conf_merge_str_value(conf->bazaar_server, prev->bazaar_server, c_bazaar_server);
    }

    ngx_conf_merge_str_value(conf->tmp_dir, prev->tmp_dir,
                             c_tmp_dir);

    if(stat((const char *)conf->bazaar_dir.data, &stat_buf) < 0) {
        rp_error(cf->log, "Can not open local Bazaar directory (%s): %s",
                 conf->bazaar_dir.data, strerror(errno));
        return NGX_CONF_ERROR;
    }

    if(((stat_buf.st_mode & S_IFMT) != S_IFDIR) ||
       !(stat_buf.st_mode & S_IRUSR)) {
        rp_error(cf->log, "Can not open local Bazaar directory (%s): "
                 "Permission denied", conf->bazaar_dir.data);
        return NGX_CONF_ERROR;
    }

    if(stat((const char *)conf->tmp_dir.data, &stat_buf) < 0) {
        rp_error(cf->log, "Can not open local tmp directory (%s): %s",
                 conf->tmp_dir.data, strerror(errno));
        return NGX_CONF_ERROR;
    }

    if(((stat_buf.st_mode & S_IFMT) != S_IFDIR) ||
       !(stat_buf.st_mode & S_IRUSR)) {
        rp_error(cf->log, "Can not open local tmp directory (%s): "
                 "Permission denied", conf->tmp_dir.data);
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


/*----------------------------------------------------------------------------*/
ngx_int_t ngx_http_rp_bazaar_cmd_handler(ngx_http_request_t *r)
{
    r->header_only = 0;

    /* Bazaar endpoint */
    if((r->uri.len >= strlen(c_bazaar_uri)) &&
       (ngx_strncmp(r->uri.data, c_bazaar_uri, strlen(c_bazaar_uri)) == 0)) {
        return rp_bazaar_cmd_handler(r);
    }

    /* Data endpoint */
    if((r->uri.len >= strlen(c_data_uri)) &&
       (ngx_strncmp(r->uri.data, c_data_uri, strlen(c_data_uri)) == 0)) {
        return rp_data_cmd_handler(r);
    }

    return NGX_HTTP_NOT_ALLOWED;
}


/*----------------------------------------------------------------------------*/
char *ngx_http_rp_bazaar_cmd(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_rp_bazaar_cmd_handler;

    return NGX_CONF_OK;
}


/*----------------------------------------------------------------------------*/
int rp_module_cmd_parse_args(const char *args, int len, char ***argv)
{
    char **l_argv;
    char *s;
    int i = 0;
    int j = 0;

    while(isblank(args[i++]));
    i--;

    if(i >= len) {
        *argv = NULL;
        return 0;
    }

    s = (char *)&args[i];

    l_argv = (char **)malloc(sizeof(char *) * 1);

    for(; i <= len; i++) {
        if(isblank(args[i]) || (i == len)) {
            l_argv = (char **)realloc(l_argv, sizeof(char *) * (j+1));
            l_argv[j] = (char *)malloc((&args[i]-s)+1);
            strncpy(l_argv[j], s, &args[i]-s);
            l_argv[j][&args[i]-s]='\0';
            j++;
            i++;
            while(isblank(args[i++]));
            i--;
            if(i >= len) {
                break;
            }
            s = (char *)&args[i];
        }
    }

    *argv = l_argv;

    return j;
}


/*----------------------------------------------------------------------------*/
int rp_module_cmd_error(cJSON **json_root, const char *reason,
                        const char *stderror, ngx_pool_t *pool)
{
    cJSON_AddItemToObject(*json_root, "status",
                          cJSON_CreateString("ERROR", pool), pool);
    cJSON_AddItemToObject(*json_root, "reason",
                          cJSON_CreateString(reason, pool), pool);
    if(stderror) {
        cJSON_AddItemToObject(*json_root, "stderror",
                              cJSON_CreateString(stderror, pool), pool);
    }
    return -1;
}


/*----------------------------------------------------------------------------*/
int rp_module_cmd_ok(cJSON **json_root, ngx_pool_t *pool)
{
    cJSON_AddItemToObject(*json_root, "status",
                          cJSON_CreateString("OK", pool), pool);
    return 0;
}


/*----------------------------------------------------------------------------*/
int rp_module_cmd_again(cJSON **json_root, ngx_pool_t *pool)
{
    cJSON_AddItemToObject(*json_root, "status",
                          cJSON_CreateString("AGAIN", pool), pool);
    return 0;
}


/*----------------------------------------------------------------------------*/
ngx_int_t rp_module_redirect(ngx_http_request_t *r, const char *loc)
{
    ngx_table_elt_t  *location;

    location = ngx_list_push(&r->headers_out.headers);
    if (location == NULL) {
        rp_error(r->connection->log, "Can not allocate memory");
        return NGX_ERROR;
    }

    location->hash = 1;
    ngx_str_set(&location->key, "Location");
    ngx_str_set(&location->value, loc);
    location->value.len = strlen(loc);

    ngx_http_clear_location(r);
    r->headers_out.location = location;

    return NGX_HTTP_MOVED_TEMPORARILY;
}


/*----------------------------------------------------------------------------*/
ngx_int_t rp_module_send_response(ngx_http_request_t *r, cJSON **json_root)
{
    ngx_buf_t   *b;
    ngx_chain_t  out;
    ngx_int_t    rc;
    char *out_buffer;
    int   out_buffer_len;
    cJSON *j_params;
    cJSON *j_status;

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if(b == NULL) {
        rp_error(r->connection->log, "Can not allocate memory");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    out.buf = b;
    out.next = NULL;
    r->headers_out.content_type_len = strlen(json_content_str);
    r->headers_out.content_type.len = strlen(json_content_str);
    r->headers_out.content_type.data = (u_char *)json_content_str;

    out_buffer = cJSON_PrintUnformatted(*json_root, r->pool);
    if(out_buffer == NULL) {
        rp_error(r->connection->log, "Creating output buffer failed");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out_buffer_len = strlen(out_buffer);

    b->pos = (u_char *)out_buffer;
    b->last =
        (u_char *)out_buffer + out_buffer_len;
    b->memory   = 1;
    b->last_buf = b->last_in_chain = 1;
    b->sync     = b->flush = 1;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = out_buffer_len;

    /* Debug purpopses - output params & status */
    j_params = cJSON_GetObjectItem(*json_root, "params");
    if(j_params != NULL) {
        int params_cnt = -1;
        int i;
        params_cnt = cJSON_GetArraySize(j_params);
        rp_debug(r->connection->log, "Output parameters: ");
        for(i = 0; i < params_cnt; i++) {
            rp_debug(r->connection->log, "\t%d - %f", i,
                     (float)cJSON_GetArrayItem(j_params, i)->valuedouble);
        }
    }
    j_status = cJSON_GetObjectItem(*json_root, "status");
    //if(j_status != NULL) {
    //    rp_debug(r->connection->log, "Output status (req :%d): %s", r->method,
    //             j_status->valuestring);
    //}

    cJSON_Delete(*json_root, r->pool);

    rc = ngx_http_send_header(r);

    /* If error while sending OK output we re-send it */
    if((rc == NGX_ERROR) && (r->method == NGX_HTTP_GET) && j_status &&
       (j_status->valuestring[0] = 'O') && (j_status->valuestring[1] == 'K')) {
        rp_data_clear_signals_dirty();
    }
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    /* TODO: Be sure that outputting is always flushed! Had some problems
     * at this part.
     */

    /* send the buffer chain of your response */
    /* Temp, got from ruby-forum.com - put socket to blocking */
    ngx_blocking(r->connection->fd);
    rc = ngx_http_output_filter(r, &out);
    while(rc == NGX_AGAIN) {
        r->connection->write->ready = 1;
        rc = ngx_http_output_filter(r, &out);
        if(rc == NGX_ERROR)
            break;
    }
    ngx_nonblocking(r->connection->fd);
    rc = ngx_http_output_filter(r, NULL);

    return NGX_DONE;
}

