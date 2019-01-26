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
	free(upk.pubkey);
}

void test_add_if_not_exist(void **state) {
	(void) state;
	int ret = -1;
	int ret2 = -1;

	struct UserPubkey *upk_p=NULL;
	struct UserPubkey *upk2_p=NULL;
	upk_p = calloc(1,sizeof(struct UserPubkey));
	upk2_p = calloc(1,sizeof(struct UserPubkey));
	upk_p->pubkey = generate_testpubkey(0);
	upk2_p->pubkey = generate_testpubkey(1);

	ret = add_if_not_exist(upk_p, upk2_p);
	ret2 = add_if_not_exist(upk_p, upk2_p);

	free_all(upk_p);

	assert_int_equal(1, ret);
	assert_int_equal(0, ret2);
}

void test_contains(void **state) {
	(void) state;
	int ret = 0;

	struct UserPubkey upk2 = {
		"testuser2",
		generate_testpubkey(1),
		"testuser2@github",
		NULL
	};
	struct UserPubkey upk = {
		"testuser",
		generate_testpubkey(0),
		"testuser@github",
		&upk2
	};
	ssh_key* reference = generate_testpubkey(1);

	ret = contains(upk, *reference);
	ssh_key_free(*(upk.pubkey));
	free(upk.pubkey);
	ssh_key_free(*(upk2.pubkey));
	free(upk2.pubkey);
	ssh_key_free(*reference);
	free(reference);
	assert_int_equal(ret, 1);
}

void test_count(void **state) {
	(void) state;
	int ret = 0;

	struct UserPubkey *upk;
	struct UserPubkey *upk2;
	upk = calloc(1,sizeof(struct UserPubkey));
	upk2 = calloc(1,sizeof(struct UserPubkey));
	upk->next = upk2;

	ret = count(upk);
	free(upk);
	free(upk2);
	assert_int_equal(ret, 2);
}

void test_create_userpubkey(void **state) {
	(void) state;
	struct UserPubkey *upk;
	upk = create_userpubkey("testuser", generate_testpubkey(0),"testuser@host");

	assert_string_equal("testuser", upk->username);
	assert_int_equal(1, ssh_key_is_public(*(upk->pubkey)));
	assert_string_equal("testuser@host", upk->comment);
	assert_null(upk->next);

	free(upk->username);
	ssh_key_free(*(upk->pubkey));
	free(upk->pubkey);
	free(upk->comment);
	free(upk);
}

void test_free_all(void **state) {
	(void) state;
	int ret = -1;

	struct UserPubkey *upk_p=NULL;
	struct UserPubkey *upk2_p=NULL;
	upk_p = calloc(1,sizeof(struct UserPubkey));
	upk2_p = calloc(1,sizeof(struct UserPubkey));
	if (NULL == upk_p){
		return;
	}
	if (NULL == upk2_p){
		return;
	}
	upk_p->next = upk2_p;
	upk_p->pubkey = generate_testpubkey(0);
	upk2_p->pubkey = generate_testpubkey(1);

	ret = free_all(upk_p);
	assert_int_equal(2, ret);
}

void test_free_last(void **state) {
	(void) state;
	int ret = -1;

	struct UserPubkey *upk_p=NULL;
	struct UserPubkey *upk2_p=NULL;
	upk_p = calloc(1,sizeof(struct UserPubkey));
	upk2_p = calloc(1,sizeof(struct UserPubkey));
	if (NULL == upk_p){
		return;
	}
	if (NULL == upk2_p){
		return;
	}
	upk_p->next = upk2_p;
	upk_p->pubkey = generate_testpubkey(0);
	upk2_p->pubkey = generate_testpubkey(1);

	ret = free_last(upk_p);

	ssh_key_free(*(upk_p->pubkey));
	free(upk_p->pubkey);
	assert_null(upk_p->next);
	free(upk_p);

	assert_int_equal(ret, 1);
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
	free(upk.pubkey);
	ssh_key_free(*(upk2.pubkey));
	free(upk2.pubkey);
	ssh_key_free(*reference);
	free(reference);

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
		cmocka_unit_test(test_add_if_not_exist),
		cmocka_unit_test(test_contains),
		cmocka_unit_test(test_count),
		cmocka_unit_test(test_create_userpubkey),
		cmocka_unit_test(test_holds),
		cmocka_unit_test(test_free_all),
		cmocka_unit_test(test_free_last),
	};

	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);

	return count_fail_tests;
}