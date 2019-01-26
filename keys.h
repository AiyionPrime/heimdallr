#ifndef KEYS_HEIMDALLR_H_
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <stdarg.h>

#include <stdio.h>

#include <libssh/libssh.h>

#define KEYS_HEIMDALLR_H_

struct UserPubkey{
	char *username;
	ssh_key* pubkey;
	char *comment;
	struct UserPubkey * next;
};

int add_if_not_exist(struct UserPubkey*, struct UserPubkey*);

char* build_content(struct UserPubkey*);

int contains(struct UserPubkey, ssh_key);

int count(struct UserPubkey*);

struct UserPubkey *create_userpubkey(char* username, ssh_key* pubkey, char *comment);

int holds(struct UserPubkey, ssh_key);

int free_last(struct UserPubkey*);

int free_all(struct UserPubkey*);

int print_keys(struct UserPubkey*);

int print_content(struct UserPubkey*);

#endif //KEYS_HEIMDALLR_H_
