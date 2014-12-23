#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <libwebsockets.h>

#include "websocket.h"


pthread_t *sw_thread_handler = NULL;
void *ws_worker_thread(void *args);
int rp_websocket(int *stop);
int   ws_stop;


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


static int callback_http(struct libwebsocket_context * this,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason, void *user,
        void *in, size_t len)
{
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


static int callback_redpitaya_binary(struct libwebsocket_context * this,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason,
        void *user, void *in, size_t len)
{

    switch (reason) {

    case LWS_CALLBACK_ESTABLISHED: // just log message that someone is connecting
    printf("connection established\n");
    break;

    case LWS_CALLBACK_RECEIVE: {

        //printf("Received data (%u)\n", len);
        // the funny part
        // create a buffer to hold our response
        // it has to have some pre and post padding. You don't need to care
        // what comes there, libwebsockets will do everything for you. For more info see
        // http://git.warmcat.com/cgi-bin/cgit/libwebsockets/tree/lib/libwebsockets.h#n597

        // Make sure the effective start of buffer is basic array type aligned
        int preamble = align(LWS_SEND_BUFFER_PRE_PADDING, sizeof(float));

        int payload = (PARAM_LEN + WAVE_LEN * SIG_NUM) * sizeof(float);
        unsigned char *buf = (unsigned char*) malloc(preamble + payload +
                LWS_SEND_BUFFER_POST_PADDING);
        float *p = (float *)&buf[preamble];
        int i, j;


        //printf("WS responding (prepare data)...\n");

        // Some synthetic data...
        int index = 0;
        for (i = 0; i < PARAM_LEN; i++) {
            p[index++] = 0.0;
        }
        for (i = 0; i < SIG_NUM; i++) {
            float ampl = (float)rand()/RAND_MAX * 30.0;
            //printf("ampl(%d) = %4.3f\n", i, ampl);
            for (j = 0; j < WAVE_LEN; j++) {
                p[index++] = ampl * waveform[j];
            }
        }


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
        //printf("WS responding (write)...\n");
        //int n = libwebsocket_write(wsi, &buf[preamble], payload, LWS_WRITE_BINARY);
        libwebsocket_write(wsi, &buf[preamble], payload, LWS_WRITE_BINARY);

        //printf("libwebsocket_write() returned: %d\n", n);


        // release memory back into the wild
        free(buf);
        break;
    }

    default:
        break;
    }

    return 0;
}



static struct libwebsocket_protocols protocols[] = {
        /* first protocol must always be HTTP handler */
        {
                "http-only",   // name
                callback_http, // callback
                0              // per_session_data_size
        },
        {
                "redpitaya-binary",         // protocol name - very important!
                callback_redpitaya_binary,  // callback
                0                          // we don't use any per session data

        },
        {
                NULL, NULL, 0   /* End of list */
        }
};



int rp_websocket(int *stop) {

    struct libwebsocket_context *context;   
    struct lws_context_creation_info info;
    const char *iface = NULL;
    int i;

    memset(&info, 0, sizeof info);

    info.port = 8080;
    info.iface = iface;
    info.protocols = protocols;
#ifndef LWS_NO_EXTENSIONS
    info.extensions = libwebsocket_get_internal_extensions();
#endif
    info.ssl_cert_filepath = NULL;
    info.ssl_private_key_filepath = NULL;
    info.gid = -1;
    info.uid = -1;
    info.options = 0;


    // create libwebsocket context representing this server
    context = libwebsocket_create_context(&info);

    if (context == NULL) {
        fprintf(stderr, "libwebsocket init failed\n");
        return -1;
    }


    for(i = 0; i < WAVE_LEN; i++) {
        waveform[i] = sin((float)i/30.0);
    }

    printf("Websocket starting server... \n");

    // infinite loop, to end this server send SIGTERM. (CTRL+C)
    while (!(*stop)) {
        libwebsocket_service(context, 10);
        // libwebsocket_service will process all waiting events with their
        // callback functions and then wait 50 ms.
        // (this is a single threaded webserver and this will keep our server
        // from generating load while there are not requests to process)
    }

    libwebsocket_context_destroy(context);

    return 0;
}

