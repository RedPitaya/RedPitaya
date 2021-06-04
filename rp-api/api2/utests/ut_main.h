
#ifndef __UT_MAIN_H
#define __UT_MAIN_H

#include <stdio.h>
#include <string.h>

#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"
#include "CUnit/CUCurses.h"

int init_example_suite(void);
int clean_example_suite(void);
void pass_assertions_test(void);
void fail_assertions_test(void);

int suite_la_acq_init(void);
int suite_la_acq_cleanup(void);
void reg_rw_test(void);
void la_acq_trig_test(void);

int suite_sig_gen_init(void);
int suite_sig_gen_cleanup(void);
void sig_gen_test(void);


#endif // __UT_MAIN_H

