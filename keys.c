#include "keys.h"

/*
 * Function: add_if_not_exits
 *
 * add a UserPubkey to the linked list, if it's not already part of it
 *
 * upk: the UserPubkey to add
 *
 * returns: how often the key was added, -1 if something went wrong
 */

int add_if_not_exist(struct UserPubkey *upk, struct UserPubkey *upk2) {
	struct UserPubkey * current = upk;

	if (contains(*upk, *(upk2->pubkey))) {
		return 0;
	}

	while (NULL != current->next) {
		current = current->next;
	}

	current->next = upk2;
	return 1;
}

/*
 * Function: contains
 *
 * return, whether a public ssh_key is part of the linked list
 *
 * upk: the UserPubkey list to search in
 *
 * pubkey: the public ssh_key to look for
 *
 * returns: 1 if the key was found, 0 if not
 */

int contains(struct UserPubkey upk, ssh_key pubkey) {
	struct UserPubkey * current=&upk;
	while (current!=NULL) {
		if (holds(*(current), pubkey)) {
			return 1;
		}
		current = current->next;
	}
	return 0;
}

/*
 * Function: count
 *
 * count the amount of pubkeys in the given UserPubkey list
 *
 * upk; the list of pubkeys to determine the size of
 *
 * returns: the amount of pubkeys
 */

int count(struct UserPubkey *upk) {
	int i = 0;
	struct UserPubkey * current=upk;

	while (current != NULL) {
		current = current->next;
		i++;
	}
	return i;
}

/*
 * Function: create_userpubkey
 *
 * create a new userpubkey struct from the given values
 *
 * username: the users human readable identifier
 *
 * pubkey: the libssh pubkey to represent (this ssh_key instance will be wrapped by the struct)
 *
 * comment: the string describing details of the ssh key
 *
 * returns: all the above bundled as a struct called UserPubkey
 */

struct UserPubkey *create_userpubkey(char* username, ssh_key* pubkey, char *comment) {
	struct UserPubkey *new;
	new = calloc(1, sizeof(struct UserPubkey));
	new->username=calloc(strlen(username)+1, sizeof(char));
	strcpy(new->username, username);
	new->pubkey = pubkey;
	new->comment=calloc(strlen(comment)+1, sizeof(char));
	strcpy(new->comment, comment);
	new->next=NULL;
	return new;
}

/*
 * Function: free_all
 *
 * upk: the list off UserPubkeys to free
 *
 * returns: the amount of freed UserPubkeys
 */

int free_all(struct UserPubkey *upk) {
	int ret = 0;
	int ctr = count(upk);

	for (; ctr>0; ctr--) {
		ret += free_last(upk);
	}
	return ret;
}

/*
 * Function: free_last
 *
 * frre the last struct in the list of UserPubkeys
 *
 * upk: the list to remove an item from
 *
 * returns: the amount of removed nodes (1 or 0)
 */

int free_last(struct UserPubkey *upk) {
	struct UserPubkey *current = upk;

	if (NULL == upk->next) {
		ssh_key_free(*(upk->pubkey));
		free(upk->pubkey);
		free(upk);
		return 1;
	}

	//find the secondlast node
	while (NULL != current->next->next) {
		current = current->next;
	}

	free(current->next->username);
	ssh_key_free(*(current->next->pubkey));
	free(current->next->pubkey);
	free(current->next->comment);
	free(current->next);
	current->next = NULL;
	return 1;
}

/*
 * Fucntion: holds
 *
 * return, whether a specific UserPubkey contains a given libssh pubkey
 *
 * upk: the userpubkey struct which may hold the pubkey
 *
 * returns: 1 if the key was found, 0 if not
 */

int holds(struct UserPubkey upk, ssh_key pubkey) {
	return !ssh_key_cmp(*(upk.pubkey), pubkey, 0);
}
