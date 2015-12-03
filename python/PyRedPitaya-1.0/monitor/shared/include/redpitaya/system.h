/**
 * $Id$
 *
 * @brief Red Pitaya system utility library.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef REDPITAYA_SYSTEM_H
#define REDPITAYA_SYSTEM_H

#include <netinet/in.h>


/* FPGA housekeeping structure - Bazaar relevant part */
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


int get_mac(const char* nic, char *mac);
int get_ip(const char *nic, struct in_addr *ip);
int get_xilinx_dna(unsigned long long *dna);


#endif /* REDPITAYA_SYSTEM_H */
