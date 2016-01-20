/*
 *  Simple example of a CUnit unit test.
 */

#include <stdio.h>
#include <string.h>

#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"
#include "CUnit/CUCurses.h"

/* Pointer to the file used by the tests. */
static FILE* temp_file = NULL;

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int init_suite1(void)
{
   if (NULL == (temp_file = fopen("temp.txt", "w+"))) {
      return -1;
   }
   else {
      return 0;
   }
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite1(void)
{
   if (0 != fclose(temp_file)) {
      return -1;
   }
   else {
      temp_file = NULL;
      return 0;
   }
}

/* Simple test of fprintf().
 * Writes test data to the temporary file and checks
 * whether the expected number of bytes were written.
 */
void testFPRINTF(void)
{
   int i1 = 10;

   if (NULL != temp_file) {
      CU_ASSERT(0 == fprintf(temp_file, " "));
      CU_ASSERT(2 == fprintf(temp_file, "Q\n"));
      CU_ASSERT(7 == fprintf(temp_file, "i1 = %d", i1));
   }
}

/* Simple test of fread().
 * Reads the data previously written by testFPRINTF()
 * and checks whether the expected characters are present.
 * Must be run after testFPRINTF().
 */
void testFREAD(void)
{
   char buffer[20];

   if (NULL != temp_file) {
      rewind(temp_file);
      CU_ASSERT(9 == fread(buffer, sizeof(char), 20, temp_file));
      CU_ASSERT(0 == strncmp(buffer, "Q\ni1 = 10", 9));
   }
}

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
	CU_FAIL("Problem here!")
	CU_FAIL_FATAL("This one will abort this test..");
}

void cu_add_test_macro_test(void)
{

}

/** simplified solution with tables */

CU_TestInfo example_test_array[] = {
  { "pass assertions test", pass_assertions_test},
  { "fail assertions test", fail_assertions_test },
  CU_TEST_INFO_NULL,
};

CU_SuiteInfo suites[] = {
  { "SuiteExampleTest", init_example_suite, clean_example_suite, example_test_array},
  //{ "suitename2", suite2_init-func, suite2_cleanup_func, test_array2 },
  CU_SUITE_INFO_NULL,
};

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   CU_pSuite pSuite = NULL;

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

   pSuite = CU_add_suite("SuiteExampleTest", init_example_suite, clean_example_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ((NULL == CU_add_test(pSuite, "pass assertions test", pass_assertions_test)) ||
       (NULL == CU_add_test(pSuite, "fail assertions test", fail_assertions_test)) ||
	   (NULL == CU_ADD_TEST(pSuite, cu_add_test_macro_test))) // simplified - name will be generated automatically
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* during development, you can simply run a single suit or test */
   CU_basic_run_suite(pSuite);
   CU_pTest pTest=CU_add_test(pSuite, "fail assertions test", fail_assertions_test);
   CU_basic_run_test(pSuite, pTest);

   /* simplified solution - with tables - replaces all upper code */
   /*
   if(CUE_SUCCESS!=CU_register_suites(suites)){
	   CU_cleanup_registry();
	   return CU_get_error();
   }
   */


   /* SUITE 1 */
   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
   if ((NULL == CU_add_test(pSuite, "test of fprintf()", testFPRINTF)) ||
       (NULL == CU_add_test(pSuite, "test of fread()", testFREAD)))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }



   /* RUN TEST / SELECT INTERFACE  */
   /* Run all tests using the CUnit Basic interface */
   /*
   CU_BRM_NORMAL	Failures and run summary are printed.
   CU_BRM_SILENT	No output is printed except error messages.
   CU_BRM_VERBOSE	Maximum output of run details.
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
