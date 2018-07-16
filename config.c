#include "config.h"
#include <errno.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>


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

char* getpath(char* filename){
        char *relative_path = "/.config/heimdallr/";
        const char *home = homedir();
        int newlen = strlen(filename) + strlen(relative_path) + strlen(home) + 1;
        char *fullpath;
        fullpath = (char *)calloc(sizeof(char), newlen);
        strcat(fullpath, home);
        strcat(fullpath, relative_path);
        strcat(fullpath, filename);
        return fullpath;
}

int ensure_config_dir(){
	char *dir_path = getpath("");
	int ret = mkdir(dir_path, S_IRWXU);
	free(dir_path);
	return ret;
}
