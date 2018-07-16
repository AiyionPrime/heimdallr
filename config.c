#include "config.h"
#include <errno.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>



int valid_port(char *p) {
    char *endptr;
    int port;

    port = strtol(p, &endptr, 10);
    if (MINPORT <= port && port <= MAXPORT && !*endptr && errno == 0)
        return port;
    return -1;
}

const char* homedir(){
        const char *homedir;
        if ((homedir = getenv("HOME")) == NULL) {
                homedir = getpwuid(getuid())->pw_dir;
        }
        return homedir;
}
