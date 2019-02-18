#ifndef KEYS_FILEOPS_HEIMDALLR_H_
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include "keys.h"

#define KEYS_FILEOPS_HEIMDALLR_H_

char *import_pubkey_comment(const char *);

#endif //KEYS_FILEOPS_HEIMDALLR_H_
