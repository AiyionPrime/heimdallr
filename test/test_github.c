#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>

#include "../github.h"


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

int __real_printf(const char *format, ...);
int __wrap_printf(const char *format, ...) {
	return 0;
}

void *__real__test_malloc(const size_t size, const char* file, const int line);
void *__wrap__test_malloc(size_t size) {
	int fail = (int) mock();
	if (fail) {
		return NULL;
	} else {
		return __real__test_malloc(size, __FILE__, __LINE__);
	}
}

struct json_object* fetch_jobj(char *url) {
	return mock_ptr_type(json_object*);
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

void test_free_keys(void **state) {
	char **keys;

	keys = malloc(2*sizeof(char*));
	keys[0] = malloc((70+1)*sizeof(char));
	keys[1] = malloc((70+1)*sizeof(char));

	size_t amount=2;
	free_keys(keys, amount);
}

void test_get_keys(void **state) {
	(void) state;
	int ret;
	struct json_object *fjobj;
	char * fake_response = "[ { \"id\": 13371337,\
				    \"key\": \"ssh-rsa c3NoLXJzYSBwdWI=\" }, \
				  { \"id\": 13371338, \"key\": \"ssh-rsa c3NoLXJzYSBwdWIyCg==\" } ]";
	fjobj = json_tokener_parse(fake_response);
	will_return(fetch_jobj, fjobj);	
	ret = get_keys("test_user");
	assert_int_equal(ret, 0);
}

void test_find_user(void **state) {
	(void) state;
	int ret;
	struct json_object *fjobj;
	struct json_object *fjobjkey;
	char * fake_response = "{ \"total_count\": 2,\
				  \"incomplete_results\": false ,\
				  \"items\": [ \
					{ \"login\": \"test1\", \"id\": 13 },\
					{ \"login\": \"test2\", \"id\": 37 } ] }";
	char * fake_keyresponse = "[ { \"id\": 13371337, \"key\": \"ssh-rsa c3NoLXJzYSBwdWI=\" },\
				     { \"id\": 13371338, \"key\": \"ssh-rsa c3NoLXJzYSBwdWIyCg==\" } ]";
	fjobj = json_tokener_parse(fake_response);
	fjobjkey = json_tokener_parse(fake_keyresponse);
	will_return(fetch_jobj, fjobj);	
	will_return(__wrap_getline, 0);
	will_return(fetch_jobj, fjobjkey);	
	ret = find_user("test");
	assert_int_equal(ret, 0);
}

void test_reduce_slashes(void **state) {
	(void) state;
	char *s1 = "//some///slashes////";
	char *s2 = "no/reduce";
	char *s3 = "nnoo/rreedduuccee/";
	char *r1;
	char *r2;
	char *r3;

	r1 = reduce_slashes(s1);
	assert_string_equal(r1, "/some/slashes/");
	free(r1);

	r2 = reduce_slashes(s2);
	assert_string_equal(r2, s2);
	free(r2);

	r3 = reduce_slashes(s3);
	assert_string_equal(r3, s3);
	free(r3);
}

void test_validate_githubname(void ** state) {
	(void) state;
	assert_int_equal(1, validate_githubname("test"));
	assert_int_equal(0, validate_githubname("-test"));
	assert_int_equal(1, validate_githubname("test-test"));
	assert_int_equal(1, validate_githubname("1234-test-test"));
	assert_int_equal(0, validate_githubname("0000000000000000000000000000000000000000"));
	assert_int_equal(1, validate_githubname("000000000000000000000000000000000000000"));
	assert_int_equal(0, validate_githubname(""));
	assert_int_equal(1, validate_githubname("B"));
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
		cmocka_unit_test (test_ensure_input_first_try),
		cmocka_unit_test (test_ensure_input_empty_input),
		cmocka_unit_test (test_ensure_input_too_high_input),
		cmocka_unit_test (test_ensure_input_thrice),
		cmocka_unit_test (test_get_keys),
		cmocka_unit_test (test_find_user),
		cmocka_unit_test (test_free_keys),
		cmocka_unit_test (test_reduce_slashes),
		cmocka_unit_test (test_validate_githubname),
	};

	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);

	return count_fail_tests;
}
