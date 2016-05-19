/**
 * @brief Red Pitaya RadioBox worker.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __WORKER_H
#define __WORKER_H

#include "main.h"
#include "calib.h"


/** @defgroup worker_h FPGA RadioBox sub-module worker process for FPGA interaction
 * @{
 */

/** @brief FSM state of the worker */
typedef enum worker_state_e {
    /** @brief do nothing */
    worker_idle_state = 0,

    /** @brief shutdown worker */
    worker_quit_state,

    /** @brief abort current measurement */
    worker_abort_state,

    /** @brief normal mode */
    worker_normal_state,

    /** @brief must be last entry */
    worker_nonexisting_state
} worker_state_t;


/** @brief Sets-up a running worker thread
 *
 * @param[in]    params        The initial parameter list the worker thread will take a copy from.
 * @param[in]    params_len    Count of parameters that params holds.
 * @retval       int           The return value of the pthread_create() call.
 */
int worker_init(rb_app_params_t* params, int params_len);

/** @brief Shuts-down the running worker thread
 *
 * @retval       int   The return value of the pthread_join() call.
 */
int worker_exit(void);

/** @brief The worker thread that runs the state-machine for parameter handling
 *
 * All data transfered between this thread and outer context has to be handled
 * strictly by mutex access.
 *
 * @param[in]    args  @see pthread_create for details. Not used in this context.
 * @retval       void* @see pthread_create for details. Not used in this context.
 */
void* worker_thread(void* args);


/** @brief Marks all changed values for that entries which are having a fpga_update flag set
 *
 * This function marks all changed parameter entries which are having the fpga_update attribute set.
 * Additional the count of this modified parameter entries is returned.
 *
 * @param[in]    ref      Reference parameter list for old values taken as reference.
 * @param[inout] cmp      Comparison parameter list for new values to be compare against the reference.
 * @param[in]    do_init  If true all comparisons will indicate a changed state.
 * @retval       int      Number of parameters that changed the value AND their attribute fpga_update is set.
 */
int mark_changed_fpga_update_entries(const rb_app_params_t* ref, rb_app_params_t* cmp, int do_init);


/** @brief Removes 'dirty' flags */
int worker_clear_signals(void);

/** @brief Get signals
 *
 *  Returns:
 *  0  - new signals (dirty signal) are copied to the output
 *  -1 - no new signals available (dirty signal was not set - we need to wait)
 */
int worker_get_signals(float*** traces, int* trc_idx);

/** @brief Fills the output signal structure from temp one after calculation is done
 * and marks it dirty
 */
int worker_set_signals(float** source, int index);

/** @} */


#endif /* __WORKER_H*/
