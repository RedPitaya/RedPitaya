#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <libwebsockets.h>

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_http_rp_module.h"

#include "websocket.h"


#define PARAM_LEN 100
#define WAVE_LEN  50
#define SIG_NUM    5

#define MIN(a, b)    ( (a) < (b) ? (a) : (b) )

extern ngx_http_rp_module_ctx_t rp_module_ctx;

struct session_data {
    int   payload;
    int   pos;
};

pthread_t *sw_thread_handler = NULL;
void *ws_worker_thread(void *args);
int   rp_websocket(int *stop);
int   ws_stop;

static float **rp_signals = NULL;
float waveform[WAVE_LEN];
char  *buf  = NULL;
float *fbuf = NULL;


int websocket_init(void)
{
    int ret = 0;

    sw_thread_handler = (pthread_t *)malloc(sizeof(pthread_t));
    if(sw_thread_handler == NULL) {
        return -1;
    }

    ws_stop = 0;
    ret = pthread_create(sw_thread_handler, NULL, ws_worker_thread, &ws_stop);
    if(ret != 0) {
        fprintf(stderr, "%s: pthread_create() failed: %s\n", __FUNCTION__,
                strerror(errno));
        return -1;
    }

    return 0;
}


int websocket_exit()
{
    int ret = 0;
    ws_stop = 1;
    if(sw_thread_handler) {
        ret = pthread_join(*sw_thread_handler, NULL);
        free(sw_thread_handler);
        sw_thread_handler = NULL;
    }
    if(ret != 0) {
        fprintf(stderr, "%s: pthread_join() failed: %s\n", __FUNCTION__,
                strerror(errno));
    }

    return 0;
}


void *ws_worker_thread(void *arg)
{
    fprintf(stderr, "Starting websocket server...\n");
    rp_websocket(arg);

    return 0;
}


int align(size_t in, size_t size) {

    int ret = in;

    int remainder = in % size;
    if (remainder) {
        ret += size - remainder;
    }

    return ret;
}


int ws_interpret_params(void *in, int len)
{
    int param_cnt = MIN(len/sizeof(float), PARAM_LEN);
    rp_app_params_t *rp_params;
    float *in_params;
    int ret = 0;
    int i;

    if (len % sizeof(float)) {
        fprintf(stderr, "WARNING: Websocket RX data size (%d) not aligned to float size.\n", len);
    }

    /* The void *in is not float (4-byte) aligned. Therefore, realign by copying. */
    in_params = (float *)malloc(param_cnt*sizeof(float));
    if(in_params == NULL) {
        return -1;
    }
    memcpy(in_params, in, param_cnt*sizeof(float));

    rp_params = (rp_app_params_t *)malloc((PARAM_LEN+1) * sizeof(rp_app_params_t));
    if(rp_params == NULL) {
        if (in_params)
            free(in_params);
        return -1;
    }

    fprintf(stderr, "Param RX: ");
    for(i = 0; i < param_cnt; i++) {
        rp_params[i].name = "redpitaya-binary";
        rp_params[i].value = in_params[i];
        fprintf(stderr, "%7.3f ", rp_params[i].value);
    }
    rp_params[param_cnt].name = NULL;
    fprintf(stderr, "\n");

    /* call application specific function for handling POST requests */
    if(rp_module_ctx.app.set_params_func(rp_params, param_cnt) < 0) {
        ret = -1;
    }

    if(rp_params)
        free(rp_params);
    if (in_params)
        free(in_params);

    return ret;
}


void ws_prepare_data_synth(float *fbuf, struct session_data *data)
{
    int i, j;
    int rp_sig_len = 50;
    float ampl;
    int k = 0;

    //printf("WS responding (prepare data)...\n");

    for (i = 0; i < PARAM_LEN; i++) {
        fbuf[k++] = 0.0;
    }

    ampl = (float)rand()/RAND_MAX * 50.0;
    for (i = 0; i < rp_sig_len; i++) {
        fbuf[k++] = ampl * waveform[i];
    }

    ampl = (float)rand()/RAND_MAX * 50.0;
    for (i = 0; i < rp_sig_len; i++) {
        fbuf[k++] = ampl * waveform[i];
    }

    ampl = (float)rand()/RAND_MAX * 50.0;
    for (j = 0; j < 3; j++) {
        for (i = 0; i < rp_sig_len; i++) {
            fbuf[k++] = ampl * waveform[i];
        }
    }

    data->payload = (PARAM_LEN + 5*rp_sig_len) * sizeof(float);

}


void ws_prepare_data(float *fbuf, struct session_data *data)
{
    rp_app_params_t *rp_params = NULL;
    int rp_params_cnt;

    int rp_sig_num, rp_sig_len;
    int ret;
    int i, j;

    data->payload = 0;

    /* Get the parameters */
    rp_params_cnt = rp_module_ctx.app.get_params_func(&rp_params);
    if(rp_params == NULL) {
        fprintf(stderr, "Error retrieving RP parameters.\n");
        return;
    }

    /* Get the signals */
    int retries = 200; // ms
    do {
        ret = rp_module_ctx.app.get_signals_func((float ***)&rp_signals, &rp_sig_num, &rp_sig_len);

        if(ret == -2)
            break;
        if(retries-- <= 0) {
            /* Use old signals */
            break;
        } else {
            usleep(1000);
        }
    } while (ret == -1);

    //fprintf(stderr, "Got RP data after %d ms.\n", 200 - retries);

    /* Populate arrayBuffer */
    rp_sig_len = 50;
    int k = 0;
    for (i = 0; i < PARAM_LEN; i++) {
        if (i < rp_params_cnt) {
            fbuf[k++] = rp_params[i].value;
        } else {
            fbuf[k++] = 0.0;
        }
    }
    /* X vector */
    for (i = 0; i < rp_sig_len; i++) {
        fbuf[k++] = rp_signals[0][i];
    }
    /* All the selected Y vectors follow */
    for (i = 0; i < rp_sig_len; i++) {
        fbuf[k++] = rp_signals[1][i];
    }
    for (j = 0; j < 3; j++) {
        for (i = 0; i < rp_sig_len; i++) {
            fbuf[k++] = rp_signals[2][i];
        }
    }

    data->payload = k * sizeof(float);

}

static int callback_redpitaya_params(struct libwebsocket_context *context,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason,
        void *user, void *in, size_t len)
{

    switch (reason) {

    case LWS_CALLBACK_ESTABLISHED:
        fprintf(stderr, "PARAMS_CALLBACK_ESTABLISHED\n");
        break;

    case LWS_CALLBACK_RECEIVE: {

        ws_interpret_params(in, len);
        break;
    }

    case LWS_CALLBACK_CLOSED:
        fprintf(stderr, "PARAMS_CALLBACK_CLOSED\n");
        break;

    default:
        break;
    }

    return 0;
}

static int callback_redpitaya_data(struct libwebsocket_context *context,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason,
        void *user, void *in, size_t len)
{
    struct session_data *data = user;

    switch (reason) {

    case LWS_CALLBACK_ESTABLISHED:
        fprintf(stderr, "DATA_CALLBACK_ESTABLISHED\n");
        break;

    case LWS_CALLBACK_RECEIVE: {

        //printf("Received data (%u)\n", len);

        ws_prepare_data(fbuf, data);

        /* TODO: Fix libwebsocket writing of more than ~1kB data.
         *
         * According to the documentation, the libwebsocket_write() should return the number of
         * successfully written bytes to the socket. However, it does not! All I have seen (traced)
         * it is to return 0 on success or -1 on failure. And it always fails with large payload (> ~1kB).
         *
         * There is a different implementation "websocket-multi.c" - adapted from Andraž Vrhovec for RP use,
         * however, it cannot work with libwebsocket_write() not returning the number of successfully written
         * bytes to the socket...
         *
         * This most probably is due not using the most recent libwebsocket library. RP is using the libwebsocket
         * from buildroot in ../../../OS. The most probable solution is therefore to replace the libwebsocket
         * in the currently used buildroot, or replace the buildroot with the newest one (2014.11), which
         * does have the latest libwebsocket included.
         *
         */
        unsigned char *fbp = (unsigned char *)fbuf;

        //int n = libwebsocket_write(wsi, &buf[preamble], payload, LWS_WRITE_BINARY);
        libwebsocket_write(wsi, fbp, data->payload, LWS_WRITE_BINARY);

        //printf("libwebsocket_write() returned: %d\n", n);

        break;
    }

    case LWS_CALLBACK_CLOSED:
        fprintf(stderr, "DATA_CALLBACK_CLOSED\n");
        break;

    default:
        break;
    }

    return 0;
}


static int callback_http(struct libwebsocket_context *context,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason,
        void *user, void *in, size_t len)
{
    return 0;
}


static struct libwebsocket_protocols protocols[] = {
        {
                "http-only",
                callback_http,
                0,
                0,
                NULL,
                0
        },
        {
                "redpitaya-params",
                callback_redpitaya_params,
                0,
                0,
                NULL,
                0
        },
        {
                "redpitaya-data",
                callback_redpitaya_data,
                sizeof(struct session_data),
                0,
                NULL,
                0
        },
        {
                NULL, NULL, 0, 0, NULL, 0   /* End of list */
        }
};



int rp_websocket(int *stop) {

    struct libwebsocket_context *context;   
    struct lws_context_creation_info info;
    int i;

    memset(&info, 0, sizeof info);

    info.port = 8080;
    info.protocols = protocols;
#ifndef LWS_NO_EXTENSIONS
    info.extensions = libwebsocket_get_internal_extensions();
#endif
    info.gid = -1;
    info.uid = -1;
    info.options = 0;

    context = libwebsocket_create_context(&info);

    if (context == NULL) {
        fprintf(stderr, "libwebsocket init failed\n");
        return -1;
    }

    // Make sure the effective start of buffer is basic array type aligned
    int preamble = align(LWS_SEND_BUFFER_PRE_PADDING, sizeof(float));

    int payload = (PARAM_LEN + WAVE_LEN * SIG_NUM) * sizeof(float);
    unsigned char *buf = (unsigned char*) malloc(preamble + payload + LWS_SEND_BUFFER_POST_PADDING);
    fbuf = (float *)&buf[preamble];

    rp_signals = (float **)malloc(SIG_NUM * sizeof(float *));
    for(i = 0; i < SIG_NUM; i++) {
        rp_signals[i] = (float *)malloc(2048 * sizeof(float));
    }

    for(i = 0; i < WAVE_LEN; i++) {
        waveform[i] = sin((float)i/30.0);
    }

    fprintf(stderr, "Websocket starting server... \n");

    int n = 0;
    while (!n && !(*stop)) {
        n = libwebsocket_service(context, 10);
    }

    libwebsocket_context_destroy(context);

    free(buf);

    for(i = 0; i < SIG_NUM; i++) {
        free(rp_signals[i]);
    }
    free(rp_signals);

    return 0;
}

