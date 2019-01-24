#include "keys.h"

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
		current = current->next;
		if (holds(*(current), pubkey)) {
			return 1;
		}
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
