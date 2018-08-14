#include <stdlib.h>
#include <json-c/json.h>
#ifndef GITHUB_HEIMDALLR_H_
#define GITHUB_HEIMDALLR_H_

struct MemoryStruct { 
	char *memory;
	size_t size;
};

int ensure_input(int options);

int find_user(char *name);

int get_keys(const char *username);

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

void capped_amount_warning(int arraylength, int resultamount);

#endif //GITHUB_HEIMDALLR_H_

