
#ifndef __UT_MAIN_H
#define __UT_MAIN_H

#include <stdio.h>
#include <string.h>

#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"
#include "CUnit/CUCurses.h"

int suite_la_acq_init(void);
int suite_la_acq_cleanup(void);
void reg_rw_test(void);

#endif // __UT_MAIN_H

