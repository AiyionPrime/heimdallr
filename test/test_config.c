#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>
#include <stdio.h>

#include "../config.h"


int __real_mkdir(const char *path, mode_t mode);
int __wrap_mkdir(const char *path, mode_t mode) {
        return 0;
}

const char* homedir() {
        return mock_ptr_type(char *);
}


void test_valid_port (void ** state) {
        assert_int_equal(-1, valid_port("-1"));
        assert_int_equal(-1, valid_port("65536"));
        assert_int_equal(65535, valid_port("65535"));
        assert_int_equal(-1, valid_port("0"));
        assert_int_equal(1, valid_port("1"));
}

void test_getpath(void **state) {
        (void) state;
        will_return_count(homedir, "/testhome/test", 2);
        char * out;
        out = getpath("");
        assert_string_equal("/testhome/test/.config/heimdallr/", out);
        free(out);
        out = getpath("subdir");
        assert_string_equal("/testhome/test/.config/heimdallr/subdir", out);
        free(out);

}

void test_ensure_config_dir(void **state) {
        (void) state;
        int ret;
        char *fakehome="/testhome/test";

        will_return_count(homedir, fakehome, 3);
        ret = ensure_config_dir();
        assert_int_equal(0, ret);
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
                cmocka_unit_test (test_getpath),
                cmocka_unit_test (test_ensure_config_dir),
        };

        int count_fail_tests =
            cmocka_run_group_tests (tests, setup, teardown);

        return count_fail_tests;
}
