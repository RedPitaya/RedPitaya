/**
 * @brief Red Pitaya Oscilloscope worker.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "cb_http.h"
#include "fpga.h"

#include "worker.h"


/** @brief Thread handler for the worker */
static pthread_t*               s_worker_thread_handler = NULL;

/** @brief Thread parameter processing and run state */
static worker_state_t           s_worker_ctrl_state;
/** @brief Mutex for work_ctrl_state */
static pthread_mutex_t          s_worker_ctrl_mutex = PTHREAD_MUTEX_INITIALIZER;

/** @brief Parameter list for the worker thread */
static rb_app_params_t*         s_worker_params = NULL;

/** @brief CallBack copy of params to inform the worker */
extern rp_app_params_t*         g_rp_cb_in_params;
/** @brief Holds mutex to access on parameters from outside to the worker thread */
extern pthread_mutex_t          g_rp_cb_in_params_mutex;

/** @brief Current copy of params of the worker thread */
extern rb_app_params_t*         g_rb_info_worker_params;
/** @brief Holds mutex to access parameters from the worker thread to any other context */
extern pthread_mutex_t          g_rb_info_worker_params_mutex;

static pthread_mutex_t          s_worker_traces_mutex = PTHREAD_MUTEX_INITIALIZER;
static float**                  s_worker_traces;
static int                      s_worker_traces_dirty = 0;
static int                      s_worker_traces_lastIdx = 0;
static float**                  s_worker_traces_tmp;  /* used for calculation, only from worker */

/** @brief params initialized */
extern int                      g_params_init_done;  /* @see main.c */

/** @brief Holds last received transport frame index number and flag 0x80 for processing data */
extern unsigned char            g_transport_pktIdx;


/*----------------------------------------------------------------------------------*/
int worker_init(rb_app_params_t* params, int params_len)
{
    int ret_val;

    //fprintf(stderr, "worker_init: BEGIN\n");

    // make sure all previous data is vanished
    if (s_worker_thread_handler) {
        (void) worker_exit();
    }

    s_worker_ctrl_state = worker_idle_state;

    /* create a new parameter list to the worker context */
    rb_copy_params((rb_app_params_t**) &s_worker_params, params, params_len, 1);

    s_worker_thread_handler = (pthread_t*) malloc(sizeof(pthread_t));
    if (!s_worker_thread_handler) {
        worker_exit();
        return -1;
    }

    ret_val = pthread_create(s_worker_thread_handler, NULL, worker_thread, NULL);
    if (ret_val) {
        fprintf(stderr, "pthread_create() failed: %s\n", strerror(errno));
        free(s_worker_thread_handler);
        s_worker_thread_handler = NULL;
        worker_exit();
        return -1;
    }

    //fprintf(stderr, "worker_init: END\n");
    return 0;
}

/*----------------------------------------------------------------------------------*/
int worker_exit(void)
{
    int ret_val = 0;

    //fprintf(stderr, "worker_exit: BEGIN\n");

    //fprintf(stderr, "worker_exit: before signaling quit\n");
    pthread_mutex_lock(&s_worker_ctrl_mutex);
    s_worker_ctrl_state = worker_quit_state;
    pthread_mutex_unlock(&s_worker_ctrl_mutex);
    //fprintf(stderr, "worker_exit: after signaling quit\n");

    if (s_worker_thread_handler) {
        //fprintf(stderr, "worker_exit: before joining\n");
        ret_val = pthread_join(*s_worker_thread_handler, NULL);
        //fprintf(stderr, "worker_exit: after joining\n");
        //fprintf(stderr, "worker_exit: before freeing thread handler\n");
        free(s_worker_thread_handler);
        //fprintf(stderr, "worker_exit: after freeing thread handler\n");
        //fprintf(stderr, "worker_exit: before setting worker_thread_handler=NULL\n");
        s_worker_thread_handler = NULL;
        //fprintf(stderr, "worker_exit: after setting worker_thread_handler=NULL\n");
    }
    if (ret_val) {
        fprintf(stderr, "pthread_join() failed: %s\n", strerror(errno));
    }

    //fprintf(stderr, "worker_exit: before freeing traces\n");
    rp_free_traces(&s_worker_traces);
    rp_free_traces(&s_worker_traces_tmp);
    //fprintf(stderr, "worker_exit: after freeing traces\n");

    //fprintf(stderr, "worker_exit: before freeing worker_params\n");
    fprintf(stderr, "INFO pthread_join: freeing (1) ...\n");
    rb_free_params(&s_worker_params);
    //fprintf(stderr, "worker_exit: after freeing worker_params\n");

    fprintf(stderr, "worker_exit: END\n");
    return 0;
}

/*----------------------------------------------------------------------------------*/
void* worker_thread(void* args)
{
    rb_app_params_t* l_cb_in_copy_params  = NULL;
    rb_app_params_t* l_next_params        = NULL;
    worker_state_t l_state;
    int l_do_normal_state = 0;

    //fprintf(stderr, "worker_thread: BEGIN\n");

    pthread_mutex_lock(&s_worker_ctrl_mutex);
    s_worker_ctrl_state = l_state = worker_idle_state;
    pthread_mutex_unlock(&s_worker_ctrl_mutex);

    while (1) {
        pthread_mutex_lock(&s_worker_ctrl_mutex);
        l_state = s_worker_ctrl_state;
        pthread_mutex_unlock(&s_worker_ctrl_mutex);

        pthread_mutex_lock(&g_rp_cb_in_params_mutex);
        int l_params_init_done = g_params_init_done;
        if (!l_params_init_done) {
            /* copy default params in worker_params to cb_in_copy_params, also - the FPGA is going to be configured by these entries */
            fprintf(stderr, "INFO worker_thread: rp_cb_in_params - new data, copying ...\n");
            rb_copy_params(&l_cb_in_copy_params, s_worker_params, -1, 1);

            /* take FSM out of idle l_state */
            l_do_normal_state = 1;

        } else if (g_rp_cb_in_params) {
            /* check if new parameters are available */
            //fprintf(stderr, "DEBUG worker_thread: g_rp_cb_in_params - new data, copying ...\n");
            rp_copy_params_rp2rb(&l_cb_in_copy_params, g_rp_cb_in_params);

            //fprintf(stderr, "DEBUG worker_thread: g_rp_cb_in_params - freeing (2) ...\n");
            rp_free_params(&g_rp_cb_in_params);

            /* take FSM out of idle */
            l_do_normal_state = 1;

            //fprintf(stderr, "DEBUG worker_thread: rp_cb_in_params - printing ...\n");
            //print_rb_params(l_cb_in_copy_params);
            //fprintf(stderr, "DEBUG worker_thread: rp_cb_in_params - ... done\n");
        }
        pthread_mutex_unlock(&g_rp_cb_in_params_mutex);

        /* generate new parameter vectors */
        //fprintf(stderr, "DEBUG worker_thread: before rb_copy_params(&next_params, worker_params ...)\n");
        rb_copy_params(&l_next_params, s_worker_params, -1, 1);  // worker_params contain all attributes

        /* when new data is seen, purge old revisions at the interfaces */
        if (l_cb_in_copy_params) {
            /* update modified entries */
            rb_copy_params(&l_next_params, l_cb_in_copy_params, -1, 0);  // modify l_next_params data

            //fprintf(stderr, "DEBUG worker_thread: before print_rb_params(l_next_params)\n");
            //print_rb_params(l_next_params);

            /* drop outdated data */
            pthread_mutex_lock(&g_rb_info_worker_params_mutex);
            {
                //fprintf(stderr, "INFO worker_thread: g_rb_info_worker_params - freeing (5) ...\n");
                rb_free_params(&g_rb_info_worker_params);
            }
            pthread_mutex_unlock(&g_rb_info_worker_params_mutex);
        }

        pthread_mutex_lock(&g_rb_info_worker_params_mutex);
        if (!g_rb_info_worker_params) {  // the outer context removed the list, a current new one has to be created here
            //fprintf(stderr, "DEBUG worker_thread: UPDATE for rb_info_worker_params\n");
            rb_copy_params(&g_rb_info_worker_params, l_next_params, -1, 1);
            //print_rb_params(g_rb_info_worker_params);
        }
        pthread_mutex_unlock(&g_rb_info_worker_params_mutex);

        //fprintf(stderr, "INFO worker_thread: rp_cb_in_params - freeing (3) ...\n");
        rb_free_params(&l_next_params);

        if (l_do_normal_state) {
            pthread_mutex_lock(&s_worker_ctrl_mutex);
            s_worker_ctrl_state = l_state = worker_normal_state;
            pthread_mutex_unlock(&s_worker_ctrl_mutex);

            l_do_normal_state = 0;
        }

        /* request to stop worker thread, we will shut down */
        if (l_state == worker_quit_state) {
            //fprintf(stderr, "worker_thread: worker_quit_state received\n");
            //fprintf(stderr, "worker_thread: before freeing curr_params\n");
#if 1
            fprintf(stderr, "INFO worker_thread: rp_cb_in_params - freeing (9a) ...\n");
            rp_free_params(&g_rp_cb_in_params);
#else
            fprintf(stderr, "INFO worker_thread: rp_cb_in_params - NULLing (9b) ...\n");
            g_rp_cb_in_params = NULL;
#endif

            fprintf(stderr, "INFO worker_thread: rp_cb_in_params - freeing (9c) ...\n");
            rb_free_params(&l_cb_in_copy_params);
            fprintf(stderr, "INFO worker_thread: rp_cb_in_params - freeing (9d) ...\n");
            rb_free_params(&g_rb_info_worker_params);
            fprintf(stderr, "INFO worker_thread: rp_cb_in_params - freeing (9e) ...\n");
            rb_free_params(&l_next_params);
            //fprintf(stderr, "worker_thread: after freeing curr_params\n");
            break;

#if 0
        } else if (state == worker_abort_state) {  // TODO is worker_abort_state needed anymore?
            fprintf(stderr, "WARNING worker_thread - FSM in worker_abort_state\n");
            continue;
#endif

        } else if (l_state == worker_idle_state) {
            usleep(100000);  // request for a 100 ms delay
            continue;

        } else if (l_state == worker_normal_state) {
            if (l_cb_in_copy_params) {
                int fpga_update_count = mark_changed_fpga_update_entries(s_worker_params, l_cb_in_copy_params, !l_params_init_done);  // return count of modified FPGA update values

                fprintf(stderr, "INFO worker_thread: worker_normal_state, processing new data --> update_count = %d\n", fpga_update_count);
                if (fpga_update_count > 0) {
                    //fprintf(stderr, "DEBUG worker_thread: fpga_update: -->  delegate to fpga_rb_update_all_params()\n");
                    if (fpga_rb_update_all_params(l_cb_in_copy_params)) {
                        fprintf(stderr, "ERROR worker - RadioBox: setting of FPGA registers failed\n");
                    }

                    pthread_mutex_lock(&g_rp_cb_in_params_mutex);
                    g_params_init_done = 1;
                    pthread_mutex_unlock(&g_rp_cb_in_params_mutex);
                }

                /* update worker_params */
                //fprintf(stderr, "DEBUG worker_thread: updating worker_params\n");
                rb_copy_params(&s_worker_params, l_cb_in_copy_params, -1, 0);

                //fprintf(stderr, "INFO worker_thread: rp_cb_in_params - freeing (1) ...\n");
                rb_free_params(&l_cb_in_copy_params);
            }
            /* drop working flag */
            g_transport_pktIdx &= 0x7f;

            //fprintf(stderr, "DEBUG worker_thread: mutex - before l_state change to idle\n");
            pthread_mutex_lock(&s_worker_ctrl_mutex);
            s_worker_ctrl_state = worker_idle_state;
            pthread_mutex_unlock(&s_worker_ctrl_mutex);
            //fprintf(stderr, "DEBUG worker_thread: mutex - after l_state change to idle\n");

        } else {  // any unknown states are mapped to QUIT
            pthread_mutex_lock(&s_worker_ctrl_mutex);
            s_worker_ctrl_state = worker_quit_state;
            pthread_mutex_unlock(&s_worker_ctrl_mutex);
        }
    }  // while (1)

    //fprintf(stderr, "worker_thread: END\n");
    return 0;
}


/*----------------------------------------------------------------------------------*/
int mark_changed_fpga_update_entries(const rb_app_params_t* ref, rb_app_params_t* cmp, int do_init)
{
    //fprintf(stderr, "mark_changed_fpga_update_entries: BEGIN\n");
    if (!ref || !cmp) {
        fprintf(stderr, "ERROR mark_changed_fpga_update_entries - bad arguments: ref = %p, cmp = %p\n", ref, cmp);
        return -1;
    }

    //fprintf(stderr, "INFO mark_changed_fpga_update_entries: starting loop\n");
    int count = 0;
    int i = 0;
    while (cmp[i].name) {  // for each cmp parameter entry of the list do a check and mark
        //fprintf(stderr, "INFO mark_changed_fpga_update_entries: processing name = %s, value = %f\n", cmp[i].name, cmp[i].value);
        int idx = -1;
        int j = 0;
        while (ref[j].name) {
            if (!strcmp(ref[j].name, cmp[i].name)) {  // known parameter
                idx = j;
                //fprintf(stderr, "INFO mark_changed_fpga_update_entries: matching idx = %d\n", idx);
                break;
            }
            j++;
        }

        if (idx == -1) {  // ignore unknown parameter
            fprintf(stderr, "WARNING mark_changed_fpga_update_entries - unknown param: name = %s\n", cmp[i].name);
            i++;
            continue;
        }

        if (do_init || (ref[idx].value != cmp[i].value)) {
            //fprintf(stderr, "INFO mark_changed_fpga_update_entries: values differ for '%s' (do_init=%d, ref=%lf, cmp=%lf)\n", ref[idx].name, do_init, ref[idx].value, cmp[i].value);
            if (ref[idx].fpga_update & ~0x80) {  // if fpga_update is set but the MARKER is masked out before the comparison
                //fprintf(stderr, "INFO mark_changed_fpga_update_entries: fpga_update='1' and changed value --> MARK\n");
                cmp[i].fpga_update |= 0x80;  // add FPGA update MARKER
                count++;
            }
        }
        i++;
    }
    //fprintf(stderr, "INFO mark_changed_fpga_update_entries: FPGA update count = %d, do_init = %d\n", count, do_init);

    //fprintf(stderr, "mark_changed_fpga_update_entries: END\n");
    return count;
}


/*----------------------------------------------------------------------------------*/
int worker_get_signals(float*** traces, int* trc_idx)
{
    float** trc = *traces;

    fprintf(stderr, "worker_get_signals: BEGIN\n");

    pthread_mutex_lock(&s_worker_traces_mutex);
    *trc_idx = s_worker_traces_lastIdx;
    if (!s_worker_traces_dirty) {
        pthread_mutex_unlock(&s_worker_traces_mutex);
        return -1;
    }
    memcpy(&trc[0][0], &s_worker_traces[0][0], sizeof(float) * TRACE_LENGTH);
    memcpy(&trc[1][0], &s_worker_traces[1][0], sizeof(float) * TRACE_LENGTH);
    memcpy(&trc[2][0], &s_worker_traces[2][0], sizeof(float) * TRACE_LENGTH);
    s_worker_traces_dirty = 0;
    pthread_mutex_unlock(&s_worker_traces_mutex);

    fprintf(stderr, "worker_get_signals: END\n");
    return 0;
}

/*----------------------------------------------------------------------------------*/
int worker_set_signals(float** source, int index)
{
    fprintf(stderr, "worker_set_signals: BEGIN\n");

    pthread_mutex_lock(&s_worker_traces_mutex);
    memcpy(&s_worker_traces[0][0], &source[0][0], sizeof(float) * TRACE_LENGTH);
    memcpy(&s_worker_traces[1][0], &source[1][0], sizeof(float) * TRACE_LENGTH);
    memcpy(&s_worker_traces[2][0], &source[2][0], sizeof(float) * TRACE_LENGTH);
    s_worker_traces_lastIdx = index;
    s_worker_traces_dirty = 1;
    pthread_mutex_unlock(&s_worker_traces_mutex);

    fprintf(stderr, "worker_set_signals: END\n");
    return 0;
}
