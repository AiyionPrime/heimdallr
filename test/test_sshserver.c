#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../sshserver.h"


FILE *__real_fopen(const char *pathname, const char *mode);
FILE *__wrap_fopen(const char *pathname, const char *mode) {
        if (!strcmp(pathname, "pubkeyfile")){
                static char buffer[] = "foo YmFyCg== comment\n";
                FILE *stream;
                stream = fmemopen(buffer, strlen(buffer), "r");
                return stream;
        }
        return __real_fopen(pathname, mode);
}

int __real_ssh_channel_read (ssh_channel channel, void * dest, uint32_t count, int is_stderr );
int __wrap_ssh_channel_read (ssh_channel channel, void * dest, uint32_t count, int is_stderr ) {
	int ret = mock();
	return ret;
}

int __real_ssh_pki_import_pubkey_file(const char * filename, ssh_key * pkey);
int __wrap_ssh_pki_import_pubkey_file(const char * filename, ssh_key * pkey) {
	int ret = mock();
	if (0==ret) {
		char * b64pub = "AAAAB3NzaC1yc2EAAAADAQABAAAAgQDPKtx0gYki7FQ6Id/pzOOQKtAoOK+CB7Bz1yTySwLEXjiTDJd5NbUbUWY3xmrIS+rni5g7E3JFLZKDLYXg3diKCYCjgSKjZ07MQEBM7e4Jf8kQE4uuxyjp/6l4/r/nRgSrrkj08bY538OXliRV/0p5uJw5RLqwkmJj+V760L9Bkw==";
		enum ssh_keytypes_e keytype = SSH_KEYTYPE_RSA;
		ssh_pki_import_pubkey_base64(b64pub, keytype, pkey);
	}
	return ret;
}

void __real_ssh_print_hash(enum ssh_publickey_hash_type type,
			   unsigned char * hash,
			   size_t len);
void __wrap_ssh_print_hash(enum ssh_publickey_hash_type type,
			   unsigned char * hash,
			   size_t len) {
	check_expected(len);
	check_expected_ptr(hash);
}


char* getpath(const char* filename){
	return mock_ptr_type(char*);
}

void test_print_fingerprint(void ** state) {
	(void) state;
	will_return(getpath, strdup("/test/testpubkeyfile"));
	will_return(__wrap_ssh_pki_import_pubkey_file, 0);
	expect_value(__wrap_ssh_print_hash, len, 32);
	expect_string(__wrap_ssh_print_hash, hash, "\261\357D~3\272!G\317\301ֳ\f\212+\325d\217\bPӬ\326\360\350\314)3\243=\312R\0");
	print_fingerprint("testpubkeyfile");
}

void test_print_fingerprint_no_file(void ** state) {
	(void) state;
	will_return(getpath, strdup("/test/testmissingpubkeyfile"));
	will_return(__wrap_ssh_pki_import_pubkey_file, -1);
	print_fingerprint("testpubkeyfile");
}

void test_ReadExec_empty(void ** state) {
	(void) state;
	int ret;
	ssh_channel mockchan = NULL;
	void *vptr=NULL;
	int maxlen=1337;
	will_return(__wrap_ssh_channel_read, 0);
	ret = ReadExec(mockchan, vptr, maxlen);
	assert_int_equal(0, ret);
}

void test_ReadExec_error(void ** state) {
	(void) state;
	int ret;
	ssh_channel mockchan = NULL;
	void *vptr=NULL;
	int maxlen=1337;
	will_return(__wrap_ssh_channel_read, -1);
	ret = ReadExec(mockchan, vptr, maxlen);
	assert_int_equal(-1, ret);
}

int setup (void ** state) {
        return 0;
}

int teardown (void ** state) {
        return 0;
}

int main (void)
{       
	const struct CMUnitTest tests [] =
	{
		cmocka_unit_test (test_print_fingerprint),
		cmocka_unit_test (test_print_fingerprint_no_file),
		cmocka_unit_test (test_ReadExec_empty),
		cmocka_unit_test (test_ReadExec_error),
	};
	
	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);
	
	return count_fail_tests;
}
