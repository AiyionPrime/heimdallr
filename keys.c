#include "keys.h"

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
