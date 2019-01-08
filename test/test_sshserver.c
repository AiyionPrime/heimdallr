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

char* getpath(const char* filename){
	return mock_ptr_type(char*);
}

}

}

}

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
	};
	
	int count_fail_tests =
	    cmocka_run_group_tests (tests, setup, teardown);
	
	return count_fail_tests;
}
