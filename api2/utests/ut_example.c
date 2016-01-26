/*
 *  Simple example of a CUnit unit test.
 */

#include <stdio.h>
#include <string.h>

#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"
#include "CUnit/CUCurses.h"

#include "ut_main.h"

/** Example Suite */

int init_example_suite(void){
    return 0;
    // return -1; in case of error
}

int clean_example_suite(void){
    return 0;
    // return -1; in case of error
}

void pass_assertions_test(void)
{
    // Notice: FATAL versions of assertions - failure will abort the original
    // test function and its entire call chain.

    /* Assert that expression is TRUE (non-zero) */
    CU_ASSERT(0==0);
    CU_ASSERT_FATAL(1==1);
    CU_TEST(2==2);
    CU_TEST_FATAL(3==3);
    /* Assert that value is TRUE (non-zero); */
    CU_ASSERT_TRUE(1);
    CU_ASSERT_TRUE_FATAL(2.0);
    /* Assert that value is FALSE (zero) */
    CU_ASSERT_FALSE(0);
    CU_ASSERT_FALSE_FATAL(0.0);
    /* Assert that actual = = expected */
    CU_ASSERT_EQUAL(0, 0)
    CU_ASSERT_EQUAL_FATAL(1, 1)
    /* Assert that actual != expected */
    CU_ASSERT_NOT_EQUAL(1, 0);
    CU_ASSERT_NOT_EQUAL_FATAL(4, 3);
    /* Assert that pointers actual = = expected */
    int a=10, b=6;
    int *ptr=&a;
    CU_ASSERT_PTR_EQUAL(&a, ptr);
    CU_ASSERT_PTR_EQUAL_FATAL(&a, ptr);
    /* Assert that pointers actual != expected */
    CU_ASSERT_PTR_NOT_EQUAL(&a, &b);
    CU_ASSERT_PTR_NOT_EQUAL_FATAL(&a, &b);
    /* Assert that pointer value == NULL */
    ptr=NULL;
    CU_ASSERT_PTR_NULL(ptr);
    CU_ASSERT_PTR_NULL_FATAL(ptr);
    /* Assert that pointer value != NULL */
    ptr=&a;
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_PTR_NOT_NULL_FATAL(ptr);
    /* Assert that strings actual and expected are equivalent */
    CU_ASSERT_STRING_EQUAL("test", "test");
    CU_ASSERT_STRING_EQUAL_FATAL("test", "test");
    /* Assert that strings actual and expected are equivalent */
    CU_ASSERT_STRING_NOT_EQUAL("test", "rest");
    CU_ASSERT_STRING_NOT_EQUAL_FATAL("test", "rest");
    /* Assert that 1st count chars of actual and expected are the same */
    CU_ASSERT_NSTRING_EQUAL("testxxx", "testyyy", 4);
    CU_ASSERT_NSTRING_EQUAL_FATAL("testxxx", "testyyy", 4);
    /* Assert that 1st count chars of actual and expected differ */
    CU_ASSERT_NSTRING_NOT_EQUAL("xxxtest", "yyytest", 3);
    CU_ASSERT_NSTRING_NOT_EQUAL_FATAL("xxxtest", "yyytest", 3);
    /* Assert that |actual - expected| <= |granularity|
       Math library must be linked in for this assertion. */
    CU_ASSERT_DOUBLE_EQUAL(1.0, 1.01, 0.02);
    CU_ASSERT_DOUBLE_EQUAL_FATAL(-1.0, -1.1, 0.1000001);
    /* Assert that |actual - expected| > |granularity|
    Math library must be linked in for this assertion. */
    CU_ASSERT_DOUBLE_NOT_EQUAL(1, 1.1, 0.05);
    CU_ASSERT_DOUBLE_NOT_EQUAL_FATAL(1, 1.1, 0.1);
    /* Register a passing assertion with the specified message.
       No logical test is performed. */
    CU_PASS("Test has passed..");
}

void fail_assertions_test(void)
{
    /* other */
    CU_FAIL("Problem here!");
    CU_FAIL_FATAL("This one will abort this test..");
}
