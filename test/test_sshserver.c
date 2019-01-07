#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../sshserver.h"

int __real_printf(const char *format, ...);
int __wrap_printf(const char *format, ...) {
        return 0;
}

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

char* getpath(const char* filename){
	return mock_ptr_type(char*);
}


void test_base64Decode(void **state) {
        (void) state;
        char * b64string = "c3VwZXJjYWxpZnJhZ2lsaXN0aWNleHBpYWxpZG9jaW91cw==";
        unsigned char * test;
        int ret;
        size_t length;
        ret = base64Decode(b64string, &test, &length);
        char * ref = "supercalifragilisticexpialidocious";
        (void) ref;
        for (int i=0; i < 33; i++){
                assert_int_equal(*(ref+i)==*(test+i), 1);
        }
        free(test);
        assert_int_equal(ret, 0);
}

void test_base64Encode(void **state) {
        (void) state;
        char * result;
        result = base64Encode((unsigned char *) "supercalifragilisticexpialidocious", 34);
        assert_string_equal(result, "c3VwZXJjYWxpZnJhZ2lsaXN0aWNleHBpYWxpZG9jaW91cw==");
        free(result);
}

void test_calcDecodeLength(void ** state) {
        (void) state;
        assert_int_equal((int)calcDecodeLength("dGVzdA=="), 4);
        assert_int_equal((int)calcDecodeLength("dGhyZWU="), 5);
        assert_int_equal((int)calcDecodeLength("c3VwZXJjYWxpZnJhZ2lsaXN0aWNleHBpYWxpZG9jaW91cw=="), 34);
}

void test_fingerprint(void **state) {
        (void) state;
        char * finger;
        finger = fingerprint("pubkeyfile");
        assert_string_equal(finger, "fYZelZskZpGMmGOvypQtD7idfJrAyZuvw3SVBN7ZdzA");
        free(finger);
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
		cmocka_unit_test (test_fingerprint),
		cmocka_unit_test (test_calcDecodeLength),
		cmocka_unit_test (test_base64Decode),
		cmocka_unit_test (test_base64Encode),
	};
	
	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);
	
	return count_fail_tests;
}
