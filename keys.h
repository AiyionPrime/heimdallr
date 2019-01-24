#ifndef KEYS_HEIMDALLR_H_
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <stdarg.h>

#include <libssh/libssh.h>

#define KEYS_HEIMDALLR_H_

struct UserPubkey{
	char *username;
	ssh_key* pubkey;
	char *comment;
	struct UserPubkey * next;
};

int contains(struct UserPubkey, ssh_key);

int count(struct UserPubkey*);

int holds(struct UserPubkey, ssh_key);

#endif //KEYS_HEIMDALLR_H_
