#include <stdlib.h>

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>

#include "../keys.c"

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

void *__real__test_malloc(const size_t size, const char* file, const int line);
void *__wrap__test_malloc(size_t size, const char* file, const int line) {
	int fail = (int) mock();
	if (fail) {
		return NULL;
	} else {
		return __real__test_malloc(size, file, line);
	}
}

void test_struct(void **state) {
	(void) state;
	will_return(__wrap__test_malloc, 0);

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
	will_return_always(__wrap__test_malloc, 0);

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

	ssh_key_free(*(upk_p->pubkey));
	ssh_key_free(*(upk2_p->pubkey));
	free(upk_p->pubkey);
	free(upk2_p->pubkey);
	free(upk_p);
	free(upk2_p);

	assert_int_equal(1, ret);
	assert_int_equal(0, ret2);
}

void test_build_content(void **state) {
	(void) state;
	will_return_always(__wrap__test_malloc, 0);

	char *stringified;
	char *reference = "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAAAgQDPKtx0gYki7FQ6Id/pzOOQKtAoOK+CB7Bz1yTySwLEXjiTDJd5NbUbUWY3xmrIS+rni5g7E3JFLZKDLYXg3diKCYCjgSKjZ07MQEBM7e4Jf8kQE4uuxyjp/6l4/r/nRgSrrkj08bY538OXliRV/0p5uJw5RLqwkmJj+V760L9Bkw== testuser@github\n";

	struct UserPubkey upk = {
		"testuser",
		generate_testpubkey(0),
		"testuser@github",
		NULL
	};

	stringified = build_content(&upk);
	assert_string_equal(reference, stringified);

	ssh_key_free(*(upk.pubkey));
	free(upk.pubkey);
	free(stringified);
}

void test_build_filename(void **state) {
	(void) state;
	will_return_always(__wrap__test_malloc, 0);

	char* expected = "752700c40a96ffb72d2b7ac6a18ce7c8.pub";
	char* result;

	struct UserPubkey *upk;
	upk = calloc(1, sizeof(struct UserPubkey));
	upk->pubkey = generate_testpubkey(0);
	upk->username=calloc(strlen("testuser")+1, sizeof(char));
	upk->comment=calloc(strlen("testuser@github")+1, sizeof(char));
	strcpy(upk->username, "testuser");
	strcpy(upk->comment, "testuser@github");

	result = build_filename(upk);
	ssh_key_free(*(upk->pubkey));
	free(upk->comment);
	free(upk->pubkey);
	free(upk->username);
	free(upk);

	assert_string_equal(expected, result);
	free(result);
}

void test_contains(void **state) {
	(void) state;
	will_return_always(__wrap__test_malloc, 0);

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
	will_return_always(__wrap__test_malloc, 0);
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
	will_return_always(__wrap__test_malloc, 0);

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
	will_return_always(__wrap__test_malloc, 0);
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
	will_return_always(__wrap__test_malloc, 0);

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

void test_print_content(void **state) {
	(void) state;
	will_return_always(__wrap__test_malloc, 0);

	int ret = 0;

	struct UserPubkey *upk;
	upk = malloc(sizeof(struct UserPubkey));
	upk->pubkey = generate_testpubkey(0);
	upk->username=calloc(strlen("testuser")+1, sizeof(char));
	upk->comment=calloc(strlen("testuser@host")+1, sizeof(char));
	strcpy(upk->username, "testuser");
	strcpy(upk->comment, "testuser@host");

	will_return(__wrap_printf, 226);
	ret = print_content(upk);
	free(upk->username);
	ssh_key_free(*(upk->pubkey));
	free(upk->pubkey);
	free(upk->comment);
	free(upk);

	assert_int_equal(ret, 226);
}

void test_print_keys(void **state) {
	(void) state;
	will_return_always(__wrap__test_malloc, 0);

	int ret=-1;
	int ret2=-1;
	struct UserPubkey *upk;
	struct UserPubkey *upk2;

	upk = calloc(1, sizeof(struct UserPubkey));
	upk->pubkey = generate_testpubkey(0);
	upk->username=calloc(strlen("testuser")+1, sizeof(char));
	upk->comment=calloc(strlen("testuser@host")+1, sizeof(char));
	strcpy(upk->username, "testuser");
	strcpy(upk->comment, "testuser@host");
	upk2 = calloc(1, sizeof(struct UserPubkey));
	upk2->pubkey = generate_testpubkey(1);
	upk2->username=calloc(strlen("testuser2")+1, sizeof(char));
	upk2->comment=calloc(strlen("testuser2@host")+1, sizeof(char));
	strcpy(upk2->username, "testuser2");
	strcpy(upk2->comment, "testuser2@host");

	will_return(__wrap_printf, 226);
	ret = print_keys(upk);
	upk->next = upk2;
	will_return(__wrap_printf, 226);
	will_return(__wrap_printf, 228);
	ret2 = print_keys(upk);

	free(upk->username);
	ssh_key_free(*(upk->pubkey));
	free(upk->pubkey);
	free(upk->comment);
	free(upk);
	free(upk2->username);
	ssh_key_free(*(upk2->pubkey));
	free(upk2->pubkey);
	free(upk2->comment);
	free(upk2);

	assert_int_equal(1, ret);
	assert_int_equal(2, ret2);
}

void test_read_comment_oneline(void **state) {
	(void) state;
	will_return_always(__wrap__test_malloc, 0);

	char * ans=NULL;
	ans = read_comment_oneline("ssh-rsa AAAABgibberish== a test coment");
	assert_string_equal("a test coment", ans);
	free(ans);
}

void test_read_ssh_key_oneline(void **state) {
	(void) state;
	will_return_always(__wrap__test_malloc, 0);

	int res=0;
	ssh_key *key=NULL;
	ssh_key *reference = generate_testpubkey(0);
	const char *input = "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAAAgQDPKtx0gYki7FQ6Id/pzOOQKtAoOK+CB7Bz1yTySwLEXjiTDJd5NbUbUWY3xmrIS+rni5g7E3JFLZKDLYXg3diKCYCjgSKjZ07MQEBM7e4Jf8kQE4uuxyjp/6l4/r/nRgSrrkj08bY538OXliRV/0p5uJw5RLqwkmJj+V760L9Bkw==";

	key = read_ssh_key_oneline(input);

	assert_int_equal(res, 0);
	assert_non_null(key);
	char *b64=NULL;
	ssh_pki_export_pubkey_base64(*key, &b64);
	assert_string_equal(b64, input+8);
	free(b64);
	ssh_key_free(*key);
	ssh_key_free(*reference);
	free(key);
	free(reference);

}

void test_strip_chars(void **state) {
	(void) state;
	will_return_always(__wrap__test_malloc, 0);
	char *res=NULL;
	res=strip_chars("one-or-more-dashes","-");
	assert_string_equal(res, "oneormoredashes");
	free(res);
	res=strip_chars("a/slash/or/two/","/");
	assert_string_equal(res, "aslashortwo");
	free(res);
	res=strip_chars("but/wont/touch/these/","-");
	assert_string_equal(res, "but/wont/touch/these/");
	free(res);
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
		cmocka_unit_test(test_build_content),
		cmocka_unit_test(test_build_filename),
		cmocka_unit_test(test_contains),
		cmocka_unit_test(test_count),
		cmocka_unit_test(test_create_userpubkey),
		cmocka_unit_test(test_holds),
		cmocka_unit_test(test_free_all),
		cmocka_unit_test(test_free_last),
		cmocka_unit_test(test_print_content),
		cmocka_unit_test(test_print_keys),
		cmocka_unit_test(test_read_comment_oneline),
		cmocka_unit_test(test_read_ssh_key_oneline),
		cmocka_unit_test(test_strip_chars),
	};

	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);

	return count_fail_tests;
}
