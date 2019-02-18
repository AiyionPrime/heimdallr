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


char* __real_get_githubuser_dir(char* username);
char* __wrap_get_githubuser_dir(char* username) {
	return mock_ptr_type(char*);
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

extern DIR * __real_opendir(const char * path);
extern DIR * __wrap_opendir(const char * path) {
	return mock_ptr_type(DIR*);
}

struct dirent* __real_readdir(DIR * dirp);
struct dirent* __wrap_readdir(DIR * dirp) {
	return mock_ptr_type(struct dirent*);
}

int __real_closedir(DIR * dirp);
int __wrap_closedir(DIR * dirp) {
	return mock();
}

int __real_ssh_pki_import_pubkey_file(const char *filename, ssh_key *pkey);
int __wrap_ssh_pki_import_pubkey_file(const char *filename, ssh_key *pkey) {
	//TODO check_string_expected_ptr(filname);;
	int ret = mock();
	if (0==ret) {
		char * b64pub = mock_ptr_type(char*);
		enum ssh_keytypes_e keytype = SSH_KEYTYPE_RSA;
		ssh_pki_import_pubkey_base64(b64pub, keytype, pkey);
	}
	return ret;
}

struct json_object* fetch_jobj(char *url) {
	return mock_ptr_type(json_object*);
}


void test_concat_dir(void **state) {
	(void) state;
	char * ret = NULL;

	ret = concat_dir(0);
	assert_null(ret);

	ret = concat_dir(2, "somedir", "file");
	assert_string_equal("somedir/file", ret);
	free(ret);

	ret = concat_dir(3, "/test/", "somedir", "file");
	assert_string_equal("/test/somedir/file", ret);
	free(ret);

	ret = concat_dir(2, "/somedir", "/file");
	assert_string_equal("/somedir/file", ret);
	free(ret);
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


void test_read_githubkeys(void **state) {
	int ret;
	char **keys=NULL;
	char* fakepath = "/test/home/testuser/.config/heimdallr/githubkeys/agithubuser";
	char* fakepath_heap=malloc((strlen(fakepath)+1)*sizeof(char));
	struct dirent* directory;
	char * readkeyb64 = "AAAAB3NzaC1yc2EAAAADAQABAAAAgQDPKtx0gYki7FQ6Id/pzOOQKtAoOK+CB7Bz1yTySwLEXjiTDJd5NbUbUWY3xmrIS+rni5g7E3JFLZKDLYXg3diKCYCjgSKjZ07MQEBM7e4Jf8kQE4uuxyjp/6l4/r/nRgSrrkj08bY538OXliRV/0p5uJw5RLqwkmJj+V760L9Bkw==";
	directory = malloc(sizeof(struct dirent));
	strcpy(directory->d_name, "somepubfile");
	strcpy(fakepath_heap, fakepath);
	will_return(__wrap_get_githubuser_dir, fakepath_heap);
	will_return(__wrap_opendir, 1);

	will_return(__wrap_ssh_pki_import_pubkey_file, SSH_OK);
	will_return(__wrap_ssh_pki_import_pubkey_file, readkeyb64);

	will_return(__wrap_readdir, directory);
	will_return(__wrap_readdir, NULL);
	will_return(__wrap_closedir, 0);
	ret = read_githubkeys(keys, "testuser");
	free(directory);
	assert_int_equal(1, ret);
}

void test_read_githubkeys_many(void **state) {
	int ret;
	char **keys=NULL;
	char* fakepath = "/test/home/testuser/.config/heimdallr/githubkeys/agithubuser";
	char* fakepath_heap=malloc((strlen(fakepath)+1)*sizeof(char));
	struct dirent* directory;
	struct dirent* directory2;
	char * readkeyb64 = "AAAAB3NzaC1yc2EAAAADAQABAAAAgQDPKtx0gYki7FQ6Id/pzOOQKtAoOK+CB7Bz1yTySwLEXjiTDJd5NbUbUWY3xmrIS+rni5g7E3JFLZKDLYXg3diKCYCjgSKjZ07MQEBM7e4Jf8kQE4uuxyjp/6l4/r/nRgSrrkj08bY538OXliRV/0p5uJw5RLqwkmJj+V760L9Bkw==";
	char * readkey2b64 = "AAAAB3NzaC1yc2EAAAADAQABAAAAgQDPKtx0gYki7FQ6Id/pzOOQKtAoOK+CB7Bz1yTySwLEXjiTDJd5NbUbUWY3xmrIS+rni5g7E3JFLZKDLYXg3diKCYCjgSKjZ07MQEBM7e4Jf8kQE4uuxyjp/6l4/r/nRgSrrkj08bY538OXliRV/0p5uJw5RLqwkmJj+V760L9Bkw==";
	directory = malloc(sizeof(struct dirent));
	strcpy(directory->d_name, "somepubfile");
	strcpy(fakepath_heap, fakepath);
	directory2 = malloc(sizeof(struct dirent));
	strcpy(directory2->d_name, "somepubfile");
	strcpy(fakepath_heap, fakepath);
	will_return(__wrap_get_githubuser_dir, fakepath_heap);
	will_return(__wrap_opendir, 1);

	will_return(__wrap_readdir, directory);
	will_return(__wrap_ssh_pki_import_pubkey_file, SSH_OK);
	will_return(__wrap_ssh_pki_import_pubkey_file, readkeyb64);

	will_return(__wrap_readdir, directory2);
	will_return(__wrap_ssh_pki_import_pubkey_file, SSH_OK);
	will_return(__wrap_ssh_pki_import_pubkey_file, readkey2b64);

	will_return(__wrap_readdir, NULL);
	will_return(__wrap_closedir, 0);
	ret = read_githubkeys(keys, "testuser");
	free(directory);
	free(directory2);
	assert_int_equal(2, ret);
}

void test_read_githubkeys_unknown(void **state) {
	int ret;
	char **keys=NULL;
	char* fakepath = "/test/home/unknownuser/";
	char* fakepath_heap=malloc((strlen(fakepath)+1)*sizeof(char));
	strcpy(fakepath_heap, fakepath);
	will_return(__wrap_get_githubuser_dir, fakepath_heap);
	will_return(__wrap_opendir, NULL);
	ret = read_githubkeys(keys, "unknownuser");
	assert_int_equal(0, ret);
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
		cmocka_unit_test (test_concat_dir),
		cmocka_unit_test (test_ensure_input_first_try),
		cmocka_unit_test (test_ensure_input_empty_input),
		cmocka_unit_test (test_ensure_input_too_high_input),
		cmocka_unit_test (test_ensure_input_thrice),
		cmocka_unit_test (test_get_keys),
		cmocka_unit_test (test_find_user),
		cmocka_unit_test (test_read_githubkeys),
		cmocka_unit_test (test_read_githubkeys_many),
		cmocka_unit_test (test_read_githubkeys_unknown),
		cmocka_unit_test (test_reduce_slashes),
		cmocka_unit_test (test_validate_githubname),
	};

	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);

	return count_fail_tests;
}
