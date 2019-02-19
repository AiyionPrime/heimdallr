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
 * Function: build_content
 *
 * create a concatenation of keytype, base64 representation and comment of the pubkey
 *
 * upk: the UserPubkey to stringify
 *
 * returns: a string containing the contents - the caller needs to free this
 */

char* build_content(struct UserPubkey *upk) {
	char *new_content=NULL;
	char *b64;
	ssh_pki_export_pubkey_base64(*(upk->pubkey), &b64);
	const char *type = ssh_key_type_to_char(ssh_key_type(*(upk->pubkey)));
	new_content=calloc(strlen(type)+1+strlen(b64)+1+strlen(upk->comment)+1+1, sizeof(char));
	strcpy(new_content, type);
	strcat(new_content, " ");
	strcat(new_content, b64);
	strcat(new_content, " ");
	strcat(new_content, upk->comment);
	strcat(new_content, "\n");

	ssh_string_free_char(b64);

	return new_content;
}

/*
 * Function: build_filename
 *
 * build a pretty unique filename for a ssh pubkey
 *
 * upk: the userpubkey to generate a filename for
 *
 * returns: the filename of a ssh-key
 */

char* build_filename(struct UserPubkey* upk) {
	char *buffer=NULL;
	char *stripped=NULL;
	char *result=NULL;
	unsigned char *hash=NULL;
	size_t len;
	enum ssh_publickey_hash_type htype;
	htype = SSH_PUBLICKEY_HASH_MD5;

	ssh_get_publickey_hash(*(upk->pubkey), htype, &hash, &len);
	buffer = ssh_get_fingerprint_hash(htype, hash, len);
	ssh_clean_pubkey_hash(&hash);
	stripped = strip_chars(buffer+3*sizeof(char),":");
	ssh_string_free_char(buffer);
	result = malloc((strlen(stripped)+4+1)*sizeof(char));
	strcpy(result, stripped);
	free(stripped);
	strcat(result, ".pub");
	return result;
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

struct UserPubkey *create_userpubkey(const char* username, ssh_key* pubkey, const char *comment) {
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
		free(current->username);
		free(current->comment);
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
 * Function: holds
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

/*
 * Function: print_keys
 *
 * Print all pubkeys, in pubkeyfile format
 *
 * upk: the list of UserPubkeys to print
 *
 * returns: the amount of printed keys
 */

int print_keys(struct UserPubkey *upk) {
	int amount=0;
	struct UserPubkey *current = upk;
	amount+=(0<print_content(current));
	while (NULL != current->next) {
		current = current->next;
		amount+=(0<print_content(current));
	}
	return amount;
}

/*
 * Function: print_content
 *
 * print the concatenation of keytype, base64 encoded key and its comment
 *
 * upk: the UserPubkey to print
 *
 * returns: the number of printed characters
 */

int print_content(struct UserPubkey *upk) {
	char *content = build_content(upk);
	int ret=0;
	ret = printf("%s", content);
	free(content);
	return ret;
}

/*
 * Function: read_ssh_key_oneline
 *
 * read an ssh pubkey in online format (type first followed by base64 representation)
 *
 * author: asn (libssh pki.c)
 *
 * oneline: the ssh-key to read
 *
 * returns: a pointer to a libssh ssh-key or null on failure
*/

ssh_key* read_ssh_key_oneline(const char* oneline)
{
	ssh_key *key=malloc(sizeof(ssh_key));
	enum ssh_keytypes_e type;
	char *q, *p;
	size_t buflen = strlen(oneline);
	int i=0;
	int rc=0;
	p = malloc(strlen(oneline)+1);
	strcpy(p, oneline);

	q = p;
	for (i = 0; i < buflen; i++) {
		if (' '==(int)p[i] || '\t'==(int)p[i]) {
			p[i] = '\0';
			break;
		}
	}

	type = ssh_key_type_from_name(q);
	if (type == SSH_KEYTYPE_UNKNOWN) {
		free(p);
		return NULL;
	}

	q = &p[i + 1];
	for (; i < buflen; i++) {
		if (' '==(int)p[i] || '\t'==(int)p[i]) {
			p[i] = '\0';
			break;
		}
	}

	rc = ssh_pki_import_pubkey_base64(q, type, key);
	free(p);
	if (SSH_ERROR == rc){
		return NULL;
	}
	return key;
}

/*
 * Function: read_comment_oneline
 *
 * read the comment of an ssh pubkey in online format
 * this function is based on asn's read pubkey function
 *
 * oneline: the ssh-key to read
 *
 * returns: a pointer to a comment of a pubkey
 * the caller needs to free the returned string
*/

char *read_comment_oneline(const char* oneline)
{
	char *p;
	size_t buflen = strlen(oneline);
	int i=0;

	for (i = 0; i < buflen; i++) {
		if (' '==(int)oneline[i] || '\t'==(int)oneline[i]) {
			break;
		}
	}

	i++;

	for (; i < buflen; i++) {
		if (' '==(int)oneline[i] || '\t'==(int)oneline[i]) {
			break;
		}
	}

	p = malloc(strlen(oneline+i+1)+1);
	if (NULL == p){
		return NULL;
	}

	strcpy(p,oneline+i+1);

	return p;
}

/*
 * Function: strip_chars
 *
 * generate a copy of a string less some given characters
 * the caller needs to free the returned string
 *
 * string: the string holding the original data
 *
 * chars: the characters to strip from the given string
 *
 * returns: a new string, which contains the original one less the given characters
 */

char *strip_chars(const char *string, const char *chars)
{
	char * newstr = malloc(strlen(string) + 1);
	int counter = 0;

	for ( ; *string; string++) {
		if (!strchr(chars, *string)) {
			newstr[ counter ] = *string;
			++counter;
		}
	}

	newstr[counter] = 0;
	return newstr;
}

