#ifndef KEYS_HEIMDALLR_H_
#include <stdlib.h>
#include <dirent.h>
#include <stdarg.h>

#include <libssh/libssh.h>

#define KEYS_HEIMDALLR_H_

struct UserPubkey{
	char *username;
	ssh_key* pubkey;
	char *comment;
	struct UserPubkey * next;
};
#endif //KEYS_HEIMDALLR_H_
