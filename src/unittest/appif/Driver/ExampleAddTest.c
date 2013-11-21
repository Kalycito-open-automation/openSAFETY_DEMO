/**
 * \file TST_addTests.c
 * Specifies the unittests for the section functions in the libosdd
 */

#include <assert.h>
#include <stdlib.h>

#include <cunit/CUnit.h>

#include <Driver/TST_config.h>

/* Initialization for the test, no effects at all*/

static int TST_status_init(void) { return 0; }

/* Cleanup function for tests */
static int TST_status_clean(void) { return 0; }

/* Tests for the asynchronous module */
static CU_TestInfo asyncTests[] = {
	{ "Test postPayload twice to signal busy", TST_asyncPostPayloadBusy }, 
	{ "This is an unimplemented async module testcase", TST_asyncWirTestenWasAnderes },
	CU_TEST_INFO_NULL,
};

/* Tests for the status module */
static CU_TestInfo statusTests[] = {
	{ "This is an unimplemented status module testcase", TST_statusWirTestenWas }, 
	{ "This is a second unimplemented status module testcase", TST_statusWirTestenWasAnderes },
	CU_TEST_INFO_NULL,
};

static CU_SuiteInfo suites[] = {
	{ "Asynchronous module Test Suite", TST_async_init, TST_async_clean, asyncTests }, 
    { "Status module Test Suite", TST_status_init, TST_status_clean, statusTests }, 
	CU_SUITE_INFO_NULL,
};

/*Add tests to the suites*/
void TST_AddTests(void)
{
    assert(NULL != CU_get_registry());
	assert(!CU_is_test_running());

	/* Register suites. */
    if (CU_register_suites(suites) != CUE_SUCCESS) {
            fprintf(stderr, "suite registration failed - %s\n",
                    CU_get_error_msg());
            exit(EXIT_FAILURE);
    }
}/*TST_AddTests()*/