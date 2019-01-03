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


int __real_printf(const char *format, ...);
int __wrap_printf(const char *format, ...) {
	return 0;
}

char * copystring(char* text){
	char *input = (char*) malloc((strlen(text)+1)*sizeof(char));
	strcpy(input, text);
	printf("%s", text);
	return input;
}

int __real_getline(char **lineptr, size_t *n, FILE *stream);
int __wrap_getline(char **lineptr, size_t *n, FILE *stream) {
	int mock = mock();
	if (*lineptr != NULL)
	{
		return 1;
	}
	switch (mock) {
		case 0:
			*lineptr = copystring("0\n");
			return 0;
		case 1:
			*lineptr = copystring("4\n");
			return 0;
		default:
			*lineptr = copystring("\n");
			return 0;
	}
}

void test_ensure_input_first_try(void ** state) {
	(void) state;
	int ret;

	will_return(__wrap_getline, 0);
	ret = ensure_input(3);
	assert_int_equal(ret, 0);
}

void test_ensure_input_empty_input(void ** state) {
	(void) state;
	int ret;

	will_return(__wrap_getline, 2);
	will_return(__wrap_getline, 0);
	ret = ensure_input(3);
	assert_int_equal(ret, 0);
}

void test_ensure_input_too_high_input(void ** state) {
	(void) state;
	int ret;

	will_return(__wrap_getline, 1);
	will_return(__wrap_getline, 0);
	ret = ensure_input(3);
	assert_int_equal(ret, 0);
}

void test_ensure_input_thrice(void ** state) {
	(void) state;
	int ret;

	will_return(__wrap_getline, 3);
	will_return(__wrap_getline, 3);
	will_return(__wrap_getline, 3);
	will_return(__wrap_getline, 0);
	ret = ensure_input(3);
	assert_int_equal(ret, 0);
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
		cmocka_unit_test (test_ensure_input_first_try),
		cmocka_unit_test (test_ensure_input_empty_input),
		cmocka_unit_test (test_ensure_input_too_high_input),
		cmocka_unit_test (test_ensure_input_thrice),
	};

	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);

	return count_fail_tests;
}
