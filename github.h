#include <stdlib.h>
#ifndef GITHUB_HEIMDALLR_H_
#define GITHUB_HEIMDALLR_H_

struct MemoryStruct { 
        char *memory; 
        size_t size; 
};

int ensure_input(int options);

int find_user(char *name);

void get_keys(const char *username);

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

#endif //GITHUB_HEIMDALLR_H_

