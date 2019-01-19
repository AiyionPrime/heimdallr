#ifndef GITHUB_HEIMDALLR_H_
#include <stdlib.h>
#include <json-c/json.h>
#include <dirent.h>
#include <string.h>
#include <curl/curl.h>
#include <regex.h>
#include <stdarg.h>

#include "config.h"
#define GITHUB_HEIMDALLR_H_

struct MemoryStruct { 
	char *memory;
	size_t size;
};

int ensure_input(int options);

int find_user(char *name);

int get_keys(const char *username);

int validate_githubname(char* username);

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

void capped_amount_warning(int arraylength, int resultamount);

#endif //GITHUB_HEIMDALLR_H_

