    // buffer parameters (fixed point number uM.F)
#ifndef ASG_BST_H
#define ASG_BST_H

#include <stdint.h>

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
    // burst counter parameters
    int unsigned CWR;  // counter width - burst data repetition
    int unsigned CWL;  // counter width - burst period length
    int unsigned CWN;  // counter width - burst period number
} rp_asg_bst_t;

void     rp_asg_bst_init (rp_asg_bst_t *handle, volatile rp_asg_bst_regset_t *regset, double FS, int unsigned buffer_size, int unsigned CWR, int unsigned CWL, int unsigned CWN);

uint32_t rp_asg_bst_get_data_repetitions (rp_asg_bst_t *handle);
int      rp_asg_bst_set_data_repetitions (rp_asg_bst_t *handle, uint32_t value);
uint32_t rp_asg_bst_get_data_length      (rp_asg_bst_t *handle);
int      rp_asg_bst_set_data_length      (rp_asg_bst_t *handle, uint32_t value);
uint32_t rp_asg_bst_get_period_length    (rp_asg_bst_t *handle);
int      rp_asg_bst_set_period_length    (rp_asg_bst_t *handle, uint32_t value);
uint32_t rp_asg_bst_get_period_number    (rp_asg_bst_t *handle);
int      rp_asg_bst_set_period_number    (rp_asg_bst_t *handle, uint32_t value);

#endif

