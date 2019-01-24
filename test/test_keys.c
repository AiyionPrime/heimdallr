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
	b64[1]="AAAAB3NzaC1yc2EAAAADAQABAAAAgQDPKtx0gYki7FQ6Id/pzOOQKtAoOK+CB7Bz1yTySwLEXjiTDJd5NbUbUWY3xmrIS+rni5g7E3JFLZKDLYXg3diKCYCjgSKjZ07MQEBM7e4Jf8kQE4uuxyjp/6l4/r/nRgSrrkj08bY538OXliRV/0p5uJw5RLqwkmJj+V760L9Bkw==";

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
	};

	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);

	return count_fail_tests;
}
