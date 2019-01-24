#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>

#include "../keys.h"


ssh_key *generate_testpubkey(int id) {
	ssh_key *key=malloc(sizeof(ssh_key*));

	char *b64[2];
	b64[0]="AAAAB3NzaC1yc2EAAAADAQABAAAAgQDPKtx0gYki7FQ6Id/pzOOQKtAoOK+CB7Bz1yTySwLEXjiTDJd5NbUbUWY3xmrIS+rni5g7E3JFLZKDLYXg3diKCYCjgSKjZ07MQEBM7e4Jf8kQE4uuxyjp/6l4/r/nRgSrrkj08bY538OXliRV/0p5uJw5RLqwkmJj+V760L9Bkw==";
	b64[1]="AAAAB3NzaC1yc2EAAAADAQABAAAAgQC0NpinM3ojIUi5/CI3ltCoV2EEpzLh2Ep3dMXUHu2r5iBFuBVnhZNIlT0YwCHj6lyw6RLX9Qrbwe4CRERLgGt980jle5lx88AA4FJr2E78UOP20q6vfMSis6pZD3/qPLuo8Va/Apy3cB8prfxLnk25hP+S0SiVCuLcFjpoiNzP5w==";

	ssh_pki_import_pubkey_base64(b64[id], SSH_KEYTYPE_RSA, key);

	return key;
}


int __real_printf(const char *format, ...);
int __wrap_printf(const char *format, ...) {
	return mock();
}

void test_struct(void **state) {
	(void) state;

	struct UserPubkey upk = {
		"testuser",
		generate_testpubkey(0),
		"testuser@github",
		NULL
	};

	assert_string_equal(upk.username, "testuser");
        assert_int_equal(1, ssh_key_is_public(*(upk.pubkey)));
	assert_string_equal(upk.comment, "testuser@github");
	assert_null(upk.next);

	ssh_key_free(*(upk.pubkey));
}

void test_count(void **state) {
	(void) state;
	int ret = 0;

	struct UserPubkey *upk;
	struct UserPubkey *upk2;
	upk = malloc(sizeof(struct UserPubkey));
	upk2 = malloc(sizeof(struct UserPubkey));
	upk->next = upk2;

	ret = count(upk);
	free(upk);
	free(upk2);
	assert_int_equal(ret, 2);
}

void test_holds(void **state) {
	(void) state;
	int ret = -1;
	int ret2 = -1;

	struct UserPubkey upk = {
		"testuser",
		generate_testpubkey(0),
		"testuser@github",
		NULL
	};
	struct UserPubkey upk2 = {
		"testuser2",
		generate_testpubkey(1),
		"testuser2@github",
		NULL
	};

	ssh_key* reference = generate_testpubkey(1);

	ret = holds(upk, *reference);
	ret2 = holds(upk2, *reference);

	ssh_key_free(*(upk.pubkey));
	free(*(upk2.pubkey));
	assert_int_equal(ret, 0);
	assert_int_equal(ret2, 1);
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
		cmocka_unit_test(test_struct),
		cmocka_unit_test(test_count),
		cmocka_unit_test(test_holds),
	};

	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);

	return count_fail_tests;
}
