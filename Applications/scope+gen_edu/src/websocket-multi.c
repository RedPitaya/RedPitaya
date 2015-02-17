#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <libwebsockets.h>

#include "websocket.h"
#include "main.h"

#define MIN(a, b)    ( (a) < (b) ? (a) : (b) )

pthread_t *sw_thread_handler = NULL;
void *ws_worker_thread(void *args);
int rp_websocket(int *stop);
int   ws_stop;

static float **rp_signals = NULL;

char *buf = NULL;
float *fbuf = NULL;

int ws_init(void)
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


int ws_exit()
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



#define PARAM_LEN 10
#define WAVE_LEN  50
#define SIG_NUM    5
float waveform[WAVE_LEN];

int align(size_t in, size_t size) {

    int ret = in;

    int remainder = in % size;
    if (remainder) {
        ret += size - remainder;
    }

    return ret;
}

struct session_data {
    int   payload;
    int   pos;
};


void rp_prepare_data_synth(struct session_data *data)
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

    data->payload = PARAM_LEN + 5 * rp_sig_len;

}


void rp_prepare_data(struct session_data *data)
{
    int rp_sig_num, rp_sig_len;
    int ret;
    int i, j;

    int retries = 200; // ms
    do {
        ret = rp_get_signals((float ***)&rp_signals, &rp_sig_num, &rp_sig_len);

        if(ret == -2)
            break;
        if(retries-- <= 0) {
            /* Use old signals */
            break;
        } else {
            usleep(1000);
        }
    } while (ret == -1);

    rp_sig_len = 50;
    //printf("WS responding (prepare data)...\n");
    int k = 0;
    for (i = 0; i < PARAM_LEN; i++) {
        fbuf[k++] = 0.0;
    }
    for (i = 0; i < rp_sig_len; i++) {
        fbuf[k++] = rp_signals[0][i];
    }
    for (i = 0; i < rp_sig_len; i++) {
        fbuf[k++] = rp_signals[1][i];
    }
    for (j = 0; j < 3; j++) {
        for (i = 0; i < rp_sig_len; i++) {
            fbuf[k++] = rp_signals[2][i];
        }
    }

    data->payload = PARAM_LEN + 5 * rp_sig_len;

}

static int callback_redpitaya_binary(struct libwebsocket_context *context,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason,
        void *user, void *in, size_t len)
{

    struct session_data *data = user;
    int write_mode = LWS_WRITE_CONTINUATION;

    switch (reason) {

    case LWS_CALLBACK_ESTABLISHED:
        fprintf(stderr, "LWS_CALLBACK_ESTABLISHED\n");
        data->pos = -1;
        break;

    case LWS_CALLBACK_RECEIVE:
        fprintf(stderr, "LWS_CALLBACK_RECEIVE %d\n", data->pos);

        if (data->pos == -1) {
            data->pos = 0;
        }

        rp_prepare_data_synth(data);

        libwebsocket_callback_on_writable(context, wsi);
        break;


    case LWS_CALLBACK_SERVER_WRITEABLE: {

        fprintf(stderr, "LWS_CALLBACK_SERVER_WRITEABLE %d\n", data->pos);

        if (data->pos > -1 && data->pos < data->payload) {
            fprintf(stderr, "LWS_CALLBACK_SERVER_WRITEABLE %d\n", data->pos);

            size_t len = MIN((data->payload - data->pos), 1024);

            if (data->pos == 0) {
                write_mode = LWS_WRITE_BINARY;
            }
            if (data->pos < 40000-1024) {
                write_mode |= LWS_WRITE_NO_FIN;
            }

            unsigned char *bp = (unsigned char *)fbuf + data->pos;

            int ret = libwebsocket_write(wsi, bp, len, write_mode);

            // Ideally if libwebsocket behaves as it should...
            //
            //if (ret < 0) {
            //    // Error
            //} else {
            //    data->pos +=ret;
            //}
            // Otherwise:
            if (!ret) {
                data->pos += len;
                fprintf(stderr, "Total bytes written %d 0x%x\n", data->pos, write_mode);
            } else {
                fprintf(stderr, "error:%s %d\n", __FUNCTION__, ret);
            }
        } else {
            data->pos = -1;
        }

        libwebsocket_callback_on_writable(context, wsi);

        break;
    }

    case LWS_CALLBACK_CLOSED:
        fprintf(stderr, "LWS_CALLBACK_CLOSED\n");
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
                0
        },
        {
                "redpitaya-binary",
                callback_redpitaya_binary,
                sizeof(struct session_data)

        },
        {
                NULL, NULL, 0   /* End of list */
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

    rp_signals = (float **)malloc(3 * sizeof(float *));
    for(i = 0; i < 3; i++) {
        rp_signals[i] = (float *)malloc(2048 * sizeof(float));
    }

    for(i = 0; i < WAVE_LEN; i++) {
        waveform[i] = sin((float)i/30.0);
    }

    fprintf(stderr, "Websocket starting server... \n");
    fprintf(stderr, "buf:%p, fbuf:%p\n", buf, fbuf);

    int n = 0;
    while (!n && !(*stop)) {
        n = libwebsocket_service(context, 10);
    }

    fprintf(stderr, "Websocket terminated: %d %d\n", n, *stop);

    libwebsocket_context_destroy(context);

    free(buf);

    for(i = 0; i < 3; i++) {
        free(rp_signals[i]);
    }
    free(rp_signals);

    return 0;
}

