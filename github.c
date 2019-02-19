#include "github.h"

/*
 * Function; concat_dir
 *
 * Concatenate a variable amount of paths with single '/'-signs
 *
 * amount: the amount of given path fragments
 *
 * returns: the concatenated path
*/

char *concat_dir(size_t amount, ...) {
	char *concatenated = NULL;
	char *reduced = NULL;
	char joinchar = '/';
	int i = 0;
	va_list path_p;
	size_t length = 0;

	if (amount<1)
		return NULL;

	va_start(path_p, amount);
	for (i=0; i<amount; i++) {
		const char *s = va_arg(path_p, char *);
		length += strlen(s);
	}
	va_end(path_p);

	concatenated = malloc((length+1+amount)*sizeof(char));
	if (NULL == concatenated)
		return NULL;

	char *dst=concatenated;

	va_start(path_p, amount);
	for (i=0; i<amount; i++) {
		const char *s = va_arg(path_p, char *);
		while ((*dst++ = *s++));
		dst--;
		*dst++ = joinchar;
	}
	dst--;
	*dst++='\0';
	va_end(path_p);
	reduced = reduce_slashes(concatenated);
	free(concatenated);
	return reduced;
}

/*
 * Function: ensure_input
 *
 * asks the user to choose a number in range of zero and a given option amount
 * repeats, if the input is invalid, until it's not
 *
 * options: the amount of options the user needs to choose from
 *          (must be a positive integer)
 *
 * returns: the valid option the user has chosen at last
 *          or -1 if the amount of options given is not a positive integer
 */

int ensure_input(int options)
{
	char *line = NULL;
	size_t n;
	if (options<1) {
		printf("Error: there are no options.\n");
		return -1;
	}
	int input=-1, status;
	while (input < 0 || input >= options) {
		printf("Specify a target in range [0..%i]:\n", options-1);
		getline(&line, &n, stdin);
		status = sscanf(line, "%d", &input);
		free(line);
		line = NULL;
		while (status != 1) {
			printf("Invalid Input.\nSpecify a target in range [0..%i]:\n", options-1);
			getline(&line, &n, stdin);
			status = sscanf(line, "%d", &input);
			free(line);
			line = NULL;
		}
	}
	return input;
}

/*
 * Function: capped_amount_warning
 *
 * print a warning, if not all results are shown by find_users()
 *
 * arraylength: how many results are actually shown
 *
 * resultamount: how many results were found by a search
 */

void capped_amount_warning(int arraylength, int resultamount){
	if(arraylength<resultamount) {
		printf("Warning: The search produced %d results, but only %d are shown.\n",
		       resultamount, arraylength);
		printf("         Please specify the search query to reduce the number.\n");
	}
}

/*
 * Function: fetch_jobj
 *
 * run a curl request against a json-based api and get the json_object
 *
 * url: a string containing the target url
 *
 * returns: the pointer to a json_object struct
 */

__attribute__((weak))
struct json_object* fetch_jobj(char *url)
{
	CURL *curl;
	CURLcode res;
	struct MemoryStruct chunk;
	chunk.memory = malloc(1);
	chunk.size = 0;
	struct json_object *jobj = NULL;

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Kraken/1.0");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		// set wmcallback as writefunction and chunk as target
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK) {
			fprintf(stderr, "Error: %s\n",
			        curl_easy_strerror(res));
		} else {
			jobj = json_tokener_parse(chunk.memory);
		}
	}
	curl_easy_cleanup(curl);
	free(chunk.memory);
	return jobj;
}

/*
 * Function: get_keys
 *
 * prints all public keys a given user has uploaded to GitHub, using GitHubs API
 *
 * username: a string holding a (hopefully) valid GitHub username
 *
 * returns: an integer, whether the function ran into memory issues
 */

int get_keys(const char *username)
{
	char *key_url = "https://api.github.com/users/%s/keys";
	int arraylen, jtype;
	int new_len = strlen(username)+strlen(key_url)-2+1;
	char *built_url;
	struct json_object *jobj = NULL, *tuplejobj, *keyjobj;
	ssh_key *currentkey_p;
	int max_keys = 32;
	struct UserPubkey *tempupk=NULL;
	struct UserPubkey *rootupk=NULL;

	char *tempcomment=NULL;
	built_url = malloc(new_len * sizeof(char));
	snprintf(built_url, new_len, key_url, username);

	if (!built_url) {
		printf("Error, malloc failed; no memory available.\n");
		return EXIT_FAILURE;
	}

	jobj = fetch_jobj(built_url);
	jtype = json_object_get_type(jobj);
	if (json_type_array == jtype) {
		arraylen = json_object_array_length(jobj);
		for (int i=0; i<arraylen; i++) {
			tuplejobj = json_object_array_get_idx(jobj, i);
			json_object_object_get_ex(tuplejobj, "key", &keyjobj);
			const char * keyb64 = json_object_get_string(keyjobj);
			currentkey_p = read_ssh_key_oneline(keyb64);
			if (NULL == currentkey_p) {
				continue;
			}
			char *commentpattern = "%s@github";
			int userlen = strlen(username);
			int commentpatternlen = strlen(commentpattern)-2;
			if (count(rootupk) < max_keys ) {
				tempcomment = malloc((commentpatternlen+userlen+1)*sizeof(char));
				snprintf(tempcomment, commentpatternlen+userlen+1, commentpattern, username);
				tempupk = create_userpubkey(username, currentkey_p, tempcomment);
				free(tempcomment);
				if (NULL == rootupk) {
					rootupk = tempupk;
				}else{
					add_if_not_exist(rootupk, tempupk);
				}
			}
		}
	}else{
		printf("Error: User was not found.\n");
	}
	json_object_put(jobj);
	free(built_url);

	print_keys(rootupk);
	free_all(rootupk);
	return EXIT_SUCCESS;
}

/*
 * Function: find_user
 *
 * searches for a GitHub username interactively and afterwords
 * prints all public keys, the chosen account has uploaded to GitHub, using GitHubs API
 *
 * username: a string holding a partial or similar username to an existing accountname, which will be looked up
 *
 * returns: an integer, whether the functions ran into memory issues
 */

int find_user(char *name)
{
	char *baseurl = "https://api.github.com/search/users?q=";
	char *url;
	char *escaped_name;
	struct json_object *jobj = NULL, *userjobj, *usernamejobj, *keyjobj;
	struct json_object *returnObj, *amountObj;

	int arraylen, jtype, target, resultamount;

	escaped_name = curl_escape(name, 0);
	url = malloc(strlen(baseurl)+strlen(escaped_name)+1);

	if (!url) {
		printf("Error, malloc failed; no memory available.\n");
		return EXIT_FAILURE;
	}
	strcpy(url, baseurl);
	strcat(url, escaped_name);
	curl_free(escaped_name);

	jobj = fetch_jobj(url);
	free(url);
	jtype = json_object_get_type(jobj);
	if (json_type_object != jtype) {
		json_object_put(jobj);
		return EXIT_FAILURE;
	}
	json_object_object_get_ex(jobj, "items", &returnObj);
	json_object_object_get_ex(jobj, "total_count", &amountObj);

	arraylen = json_object_array_length(returnObj);
	resultamount = json_object_get_int(amountObj);

	for (int i=0; i<arraylen; i++) {
		userjobj = json_object_array_get_idx(returnObj, i);
		json_object_object_get_ex(userjobj, "login", &usernamejobj);
		printf("%i: %s\n", i, json_object_get_string(usernamejobj));
	}
	if (arraylen>0) {
		capped_amount_warning(arraylen, resultamount);
		target = ensure_input(arraylen);
		json_object_object_get_ex(json_object_array_get_idx(
						  returnObj, target),
		                          "login", &keyjobj);
		get_keys(json_object_get_string(keyjobj));
	} else {
		printf("Info: could not find a user with a name similar to '%s'.\n", name);
	}
	json_object_put(jobj);
	return EXIT_SUCCESS;
}


/*
 * Function: read_githubkeys
 *
 * Read all available keys for a given GitHub-user from the config directory
 *
 * keys: a pointer to an array of strings to store the keys in
 *
 * username: the GitHub username whos keys to read
 *
 * returns: the amount of keys stored within the array
 */

size_t read_githubkeys(struct UserPubkey **keys, char *username) {
	if (!validate_githubname(username)) {
		return 0;
	}
	char* keydirpath = get_githubuser_dir(username);
	DIR *d;
	struct dirent *dir;
	ssh_key *cur_pubkey = NULL;
	int rc;
	char * fullpath = NULL;
	int max_keys = 32;
	size_t keyamount = 0;

	char *curcomment = NULL;
	struct UserPubkey *tempupk = NULL;

	d = opendir(keydirpath);
	if (!d)
	{
		return keyamount;
	}
	while ((dir = readdir(d)) != NULL)
	{
		if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
			continue;
		fullpath = concat_dir(3, keydirpath, "/", dir->d_name);
		if (keyamount < max_keys ) {
			cur_pubkey = calloc(1, sizeof(ssh_key));
			rc = ssh_pki_import_pubkey_file(fullpath, cur_pubkey);
			if (SSH_OK == rc) {
					curcomment=import_pubkey_comment(fullpath);
					tempupk = create_userpubkey(username, cur_pubkey, curcomment);

					if (NULL==*keys) {
						*keys=tempupk;
					}else{
						add_if_not_exist(*keys, tempupk);
					}
					keyamount++;
			} else {
				free(cur_pubkey);
			}
		}
		free(fullpath);
	}
	closedir(d);
	return keyamount;
}

/*
 * Function: reduce_slashes
 *
 * Produce a copy of a string without duplicate consecutive slashes
 *
 * tomodify: the string holding a path, possibly with redundant slashes to remove
 *
 * returns: qualitatively the same path as the input, but with no duplicate slashes
 */

char *reduce_slashes(char *tomodify) {
	size_t i, c=0;
	char *new;

	for (i=0; tomodify[i] != '\0'; i++) {
		c += (tomodify[i] != tomodify[i+1] || tomodify[i] != '/');
	}

	new = malloc(c+1);

	for (i=c=0; tomodify[i] != '\0'; i++) {
		if (tomodify[i] != tomodify[i+1] || tomodify[i] != '/') {
			new[c++] = tomodify[i];
		}
	}
	new[c]='\0';
	return new;
}

/*
 * Function: validate_githubname
 *
 * determine, whether a github username may be valid or not
 *
 * username: the username to validate
 *
 * returns: 1 if the username is valid and 0, if not
 */

int validate_githubname(char* username) {
	regex_t regex;
	int retint;
	int ret;
	retint = regcomp(&regex, "^[a-zA-Z0-9][a-zA-Z0-9\\-]\\{0,38\\}$", 0);
	retint = regexec(&regex, username, 0, NULL, 0);
	if (!retint) {
		ret = 1;
	} else if (retint == REG_NOMATCH) {
		ret = 0;
	} else {
		regerror(retint, &regex, username, sizeof(username));
		fprintf(stderr, "Regex match failed: %s\n", username);
		regfree(&regex);
		exit(1);
	}
	regfree(&regex);
	return ret;
}

/*
 * Function: WriteMemoryCallback
 *
 * A callback function to stepwise increase the size of a memory struct by one, as needed,
 * in order to store data of priorly unknown size in ram, primarily used for curl requests
 *
 * in fact, the function creates a whole new MemoryStruct each time called just in order to
 * seem like slowly increasing over time
 *
 * contents: a pointer to where the data will lie after the function call
 *
 * size: the size of one member in the MemoryStruct
 *
 * nmemb: the number of members in the MemoryStruct
 *
 * userp: the void pointer where the data lies
 *
 * returns: the size of the data as size_t after inreasing it
 */

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		/* out of memory! */
		printf("Error: not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}

