//#define TESTING 1

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "../config.h"
#include "../github.h"

void test_valid_port (void ** state) {
	assert_int_equal(-1, valid_port("-1"));
	assert_int_equal(-1, valid_port("65536"));
	assert_int_equal(65535, valid_port("65535"));
	assert_int_equal(-1, valid_port("0"));
	assert_int_equal(1, valid_port("1"));
}

int setup (void ** state)
{
	return 0;
}

int teardown (void ** state)
{
	return 0;
}

int main (void)
{
	const struct CMUnitTest tests [] =
	{
		cmocka_unit_test (test_valid_port),
	};

	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);

	return count_fail_tests;
}
