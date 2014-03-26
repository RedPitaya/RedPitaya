/**
 * @brief Red Pitaya PID FPGA controller.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef _FPGA_PID_H_
#define _FPGA_PID_H_

#include <stdint.h>

/** @defgroup fpga_pid_h PID Controller
 * @{
 */

/** Base PID FPGA address */
#define PID_BASE_ADDR 0x40300000
/** Base PID FPGA core size */
#define PID_BASE_SIZE 0x100

/** Number of PID controllers */
#define NUM_OF_PIDS 4


/** PID Controller parameters */
typedef struct {
    /** @brief Relative offset 0x0 - Set-point
     *
     *  bits [31:14] - Reserved
     *  bit  [13: 0] - Set-point (signed)
     */
    uint32_t setpoint;
    /** @brief Relative offset 0x4 - Proportional gain
     *
     *  bits [31:14] - Reserved
     *  bit  [13: 0] - Proportional gain (signed)
     */
    uint32_t kp;
    /** @brief Relative offset 0x8 - Integral gain
     *
     *  bits [31:14] - Reserved
     *  bit  [13: 0] - Integral gain (signed)
     */
    uint32_t ki;
    /** @brief Relative offset 0xC - Derivative gain
     *
     *  bits [31:14] - Reserved
     *  bit  [13: 0] - Derivative gain (signed)
     */
    uint32_t kd;
} pid_param_t;


/** @brief PID FPGA registry structure.
 *
 * This structure is direct image of physical FPGA memory. When accessing it all
 * reads/writes are performed directly from/to FPGA PID registers.
 */
typedef struct pid_reg_t {
    /** @brief Offset 0x00 - Configuration
     *
     *  bits [31:4]  - Reserved
     *  bit  [   3] -  PID22 integrator reset
     *  bit  [   2] -  PID21 integrator reset
     *  bit  [   1] -  PID12 integrator reset
     *  bit  [   0] -  PID11 integrator reset
     */
    uint32_t configuration;
    /** @brief Offset 0x04 - Reserved */
    uint32_t res0;
    /** @brief Offset 0x08 - Reserved */
    uint32_t res1;
    /** @brief Offset 0x0c - Reserved */
    uint32_t res2;

    /** @brief Offset 0x10 - Four consecutive PID Controller parameter blocks
     *
     *  pid[0] ... PID11
     *  pid[1] ... PID12
     *  pid[2] ... PID21
     *  pid[3] ... PID22
     */
    pid_param_t pid[NUM_OF_PIDS];

} pid_reg_t;

/** @} */

/* Description of the following variables or function declarations is in 
 * fpga_pid.c
 */
extern pid_reg_t    *g_pid_reg;

int fpga_pid_init(void);
int fpga_pid_exit(void);

#endif // _FPGA_PID_H_
