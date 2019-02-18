#include <stdlib.h>

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>

#include "../keys_fileops.c"

int __real_fileno(FILE *stream);
int __wrap_fileno(FILE *stream) {
	return mock();
}

FILE *__real_fopen(const char *pathname, const char *mode);
FILE *__wrap_fopen(const char *pathname, const char *mode) {
	if (!strcmp(pathname, "mocktestfile")){
		char *buffer = mock_ptr_type(char*);
		FILE *stream;
		stream = fmemopen(buffer, strlen(buffer), "r");
		return stream;
	}
	if (!strcmp(pathname, "emptymocktestfile")){
		return NULL;
	}
	return __real_fopen(pathname, mode);
}

size_t __real_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t __wrap_fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	int failval = mock();
	if (-1 != failval){
		return failval;
	}
	return __real_fread(ptr, size, nmemb, stream);
}

int __real_fstat(int fildes, struct stat *buf);
int __wrap_fstat(int fildes, struct stat *buf) {
	if (-1337 == fildes){
		errno = mock();
		buf->st_size = mock();
		return mock();
	}
	return __real_fstat(fildes, buf);
}

void *__real__test_malloc(const size_t size, const char* file, const int line);
void *__wrap__test_malloc(size_t size, const char* file, const int line) {
	int fail = (int) mock();
	if (fail) {
		return NULL;
	} else {
		return __real__test_malloc(size, file, line);
	}
}

char *__real_read_comment_oneline(const char* oneline);
char *__wrap_read_comment_oneline(const char* oneline){
	return mock_ptr_type(char*);
}

void test_import_pubkey_comment_empty_filename(void ** state){
	(void) state;
	assert_null(import_pubkey_comment(""));
	assert_null(import_pubkey_comment(NULL));
}

void test_import_pubkey_comment_empty_file(void ** state){
	(void) state;
	char *res=NULL;
	res = import_pubkey_comment("emptymocktestfile");
	assert_null(res);
	free(res);
}

void test_import_pubkey_comment_invalid_descriptor(void ** state){
	(void) state;
	char *res=NULL;
	will_return(__wrap_fileno, -1337);
	will_return(__wrap_fstat, NULL);
	will_return(__wrap_fstat, 0);
	will_return(__wrap_fstat, -1);
	will_return(__wrap_fopen, "ssh-rsa AAAABsmallkeystuff a comment");
	res = import_pubkey_comment("mocktestfile");
	assert_null(res);
	free(res);
}

void test_import_pubkey_comment_eacces(void ** state){
	(void) state;
	char *res=NULL;
	will_return(__wrap_fopen, "ssh-rsa AAAABsmallkeystuff a comment");
	will_return(__wrap_fileno, -1337);
	will_return(__wrap_fstat, EACCES);
	will_return(__wrap_fstat, 0);
	will_return(__wrap_fstat, -1);
	res = import_pubkey_comment("mocktestfile");
	assert_null(res);
	free(res);
}

void test_import_pubkey_comment_failed_malloc(void ** state){
	(void) state;
	char *res=NULL;
	will_return(__wrap_fopen, "ssh-rsa AAAABsmallkeystuff a comment");
	will_return(__wrap_fileno, 0);
	will_return(__wrap__test_malloc, 1);
	res = import_pubkey_comment("mocktestfile");
	assert_null(res);
	free(res);
}

void test_import_pubkey_comment_size_mismatch(void ** state){
	(void) state;
	char *res=NULL;
	will_return_always(__wrap__test_malloc, 0);
	will_return(__wrap_fopen, "ssh-rsa AAAABsmallkeystuff a comment");
	will_return(__wrap_fileno, -1337);
	will_return(__wrap_fstat, 0);
	will_return(__wrap_fstat, 0);
	will_return(__wrap_fstat, 0);
	will_return(__wrap_fread, 90);
	res = import_pubkey_comment("mocktestfile");
	assert_null(res);
}

void test_import_pubkey_comment_size_match(void ** state){
	(void) state;
	char *tc="testcomment";

	char *res=NULL;
	will_return_always(__wrap__test_malloc, 0);
	will_return(__wrap_fopen, "ssh-rsa AAAABsmallkeystuff a comment");
	will_return(__wrap_fileno, -1337);
	will_return(__wrap_fstat, 0);
	will_return(__wrap_fstat, 0);
	will_return(__wrap_fstat, 0);
	will_return(__wrap_fread, -1);
	will_return(__wrap_read_comment_oneline, tc);
	res = import_pubkey_comment("mocktestfile");
	assert_string_equal(res, tc);
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
		cmocka_unit_test(test_import_pubkey_comment_empty_filename),
		cmocka_unit_test(test_import_pubkey_comment_empty_file),
		cmocka_unit_test(test_import_pubkey_comment_invalid_descriptor),
		cmocka_unit_test(test_import_pubkey_comment_eacces),
		cmocka_unit_test(test_import_pubkey_comment_failed_malloc),
		cmocka_unit_test(test_import_pubkey_comment_size_mismatch),
		cmocka_unit_test(test_import_pubkey_comment_size_match),
	};

	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);

	will_return_always(__wrap_fread, -1);
	return count_fail_tests;
}
