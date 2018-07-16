#include "config.h"
#include <errno.h>
#include <stdlib.h>

int valid_port(char *p) {
    char *endptr;
    int port;

    port = strtol(p, &endptr, 10);
    if (MINPORT <= port && port <= MAXPORT && !*endptr && errno == 0)
        return port;
    return -1;
}
