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

/** example test */
CU_TestInfo example_test_array[] = {
  { "pass assertions test", pass_assertions_test},
  { "fail assertions test", fail_assertions_test },
  CU_TEST_INFO_NULL,
};

/** la_acq test */
CU_TestInfo la_acq_test_array[] = {
 // { "la_acq", reg_rw_test},
  { "la_acq_trig_test", la_acq_trig_test},
  CU_TEST_INFO_NULL,
};

/** sig gen test */
CU_TestInfo sig_gen_test_array[] = {
  { "sig_gen", sig_gen_test},
  CU_TEST_INFO_NULL,
};

// add new tests here

/** suite table */
CU_SuiteInfo suites[] = {
//  { "SuiteExampleTest", init_example_suite, clean_example_suite, example_test_array},
  { "suite_la_acq_test", suite_la_acq_init, suite_la_acq_cleanup, la_acq_test_array},
//  { "suite_sig_gen_test", suite_sig_gen_init, suite_sig_gen_cleanup, sig_gen_test_array},
  // add new suite here
  CU_SUITE_INFO_NULL,
};

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   //CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* ADD SUITS BELOW THIS COMMENT */

   /* EXAMPLE SUIT */
   /* add a suite to the registry - give it unique name
    * init and clean functions allows the suite to set up and tear down temporary
    * fixtures to support running the tests.
    * These functions take no arguments and should return zero if they are
    * completed successfully (non-zero otherwise).
    * If a suite does not require one or both of these functions, pass NULL to CU_add_suite.
    * */
/*
   pSuite = CU_add_suite("SuiteExampleTest", init_example_suite, clean_example_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   // add the tests to the suite
   if ((NULL == CU_add_test(pSuite, "pass assertions test", pass_assertions_test)) ||
       (NULL == CU_add_test(pSuite, "fail assertions test", fail_assertions_test)) ||
       (NULL == CU_ADD_TEST(pSuite, cu_add_test_macro_test))) // simplified - name will be generated automatically
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   // during development, you can simply run a single suit or test
   CU_basic_run_suite(pSuite);
   CU_pTest pTest=CU_add_test(pSuite, "fail assertions test", fail_assertions_test);
   CU_basic_run_test(pSuite, pTest);
*/
   /** simplified solution - with tables - replaces all upper code */
   if(CUE_SUCCESS!=CU_register_suites(suites)){
       CU_cleanup_registry();
       return CU_get_error();
   }

   /* RUN TEST / SELECT INTERFACE  */
   /* Run all tests using the CUnit Basic interface */
   /*
   CU_BRM_NORMAL    Failures and run summary are printed.
   CU_BRM_SILENT    No output is printed except error messages.
   CU_BRM_VERBOSE    Maximum output of run details.
   */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_basic_show_failures(CU_get_failure_list());
   printf("\n");

   /* Run all tests using the console interface */
   // CU_console_run_tests();

   /* Run all tests using the curses interface */
   // CU_curses_run_tests();

   /* Run automated interface (non-interactive) that will generate XML file. */
   //CU_automated_run_tests();
   //CU_list_tests_to_file();

   /* CLEAN UP */
   CU_cleanup_registry();
   return CU_get_error();
}
