#ifndef ASG_BST_H
#define ASG_BST_H

#include <stdint.h>

#include "redpitaya/util.h"

#define CWR 14
#define CWL 32
#define CWN 16

typedef struct {
    uint32_t cfg_bdr;  // burst data   repetitions
    uint32_t cfg_bdl;  // burst data   length
    uint32_t cfg_bpl;  // burst period length (data+pause)
    uint32_t cfg_bpn;  // burst period number
    uint32_t sts_bln;  // length (current position inside burst length)
    uint32_t sts_bnm;  // number (current burst counter)
} rp_asg_bst_regset_t;

typedef struct {
    volatile rp_asg_bst_regset_t *regset;
    double FS;
    int unsigned buffer_size;
    // burst counter fixed point types
    fixp_t bdr_t; // burst data repetition
    fixp_t bpl_t; // burst period length
    fixp_t bpn_t; // burst period number
} rp_asg_bst_t;

void     rp_asg_bst_init                 (rp_asg_bst_t *handle, volatile rp_asg_bst_regset_t *regset, double FS, int unsigned buffer_size, const fixp_t bdr_t, const fixp_t bpl_t, const fixp_t bpn_t);
void     rp_asg_bst_default              (rp_asg_bst_t *handle);
void     rp_asg_bst_print                (rp_asg_bst_t *handle);

uint32_t rp_asg_bst_get_data_repetitions (rp_asg_bst_t *handle);
int      rp_asg_bst_set_data_repetitions (rp_asg_bst_t *handle, uint32_t value);
uint32_t rp_asg_bst_get_data_length      (rp_asg_bst_t *handle);
int      rp_asg_bst_set_data_length      (rp_asg_bst_t *handle, uint32_t value);
uint32_t rp_asg_bst_get_period_length    (rp_asg_bst_t *handle);
int      rp_asg_bst_set_period_length    (rp_asg_bst_t *handle, uint32_t value);
uint32_t rp_asg_bst_get_period_number    (rp_asg_bst_t *handle);
int      rp_asg_bst_set_period_number    (rp_asg_bst_t *handle, uint32_t value);

#endif

