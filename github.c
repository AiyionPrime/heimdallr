#include "github.h"
#include <string.h>
#include <curl/curl.h>

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
	if (options<1) {
		printf("Error: there are no options.\n");
		return -1;
	}
	int input=-1, temp, status;
	while (input < 0 || input >= options){
		printf("Specify a target in range [0..%i]:\n", options-1);
		status = scanf("%d", &input);
		while (status != 1){
			while((temp=getchar()) != EOF && temp != '\n');
			printf("Invalid Input.\nSpecify a target in range [0..%i]:\n", options-1);
			status = scanf("%d", &input);
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
	if(arraylength<resultamount){
		printf("Warning: The search produced %d results, but only %d are shown.\n", resultamount, arraylength);
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

struct json_object* fetch_jobj(char *url)
{
	CURL *curl;
	CURLcode res;
	struct MemoryStruct chunk;
	chunk.memory = malloc(1);
	chunk.size = 0;
	struct json_object *jobj = NULL;

	curl = curl_easy_init();
	if(curl){
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Kraken/1.0");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		// set wmcallback as writefunction and chunk as target
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK){
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

	built_url = malloc(new_len * sizeof(char));
	snprintf(built_url, new_len, key_url, username);

	if (!built_url){
		printf("Error, malloc failed; no memory available.\n");
		return EXIT_FAILURE;
	}

	jobj = fetch_jobj(built_url);
	jtype = json_object_get_type(jobj);
	if (json_type_array == jtype){
		arraylen = json_object_array_length(jobj);
		for (int i=0; i<arraylen; i++){
			tuplejobj = json_object_array_get_idx(jobj, i);
			keyjobj = json_object_object_get(tuplejobj, "key");
			printf("%s %s@github\n", json_object_get_string(keyjobj), username);
		}
	}else{
		printf("Error: User was not found.\n");
	}
	json_object_put(jobj);
	free(built_url);
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
	struct json_object *jobj = NULL, *userjobj, *usernamejobj;
	struct json_object *returnObj, *amountObj;

	int arraylen, target, resultamount;

	escaped_name = curl_escape(name, 0);
	url = malloc(strlen(baseurl)+strlen(escaped_name)+1);

	if (!url){
		printf("Error, malloc failed; no memory available.\n");
		return EXIT_FAILURE;
	}
	strcpy(url, baseurl);
	strcat(url, escaped_name);
	curl_free(escaped_name);

	jobj = fetch_jobj(url);
	returnObj = json_object_object_get(jobj, "items");
	amountObj = json_object_object_get(jobj, "total_count");

	arraylen = json_object_array_length(returnObj);
	resultamount = json_object_get_int(amountObj);

	for (int i=0; i<arraylen; i++){
		userjobj = json_object_array_get_idx(returnObj, i);
		usernamejobj = json_object_object_get(userjobj, "login");
		printf("%i: %s\n", i, json_object_get_string(usernamejobj));
	}
	if (arraylen>0){
		capped_amount_warning(arraylen, resultamount);
		target = ensure_input(arraylen);
		get_keys(json_object_get_string(json_object_object_get(json_object_array_get_idx(returnObj, target), "login")));
	} else {
		printf("Info: could not find a user with a name similar to '%s'.\n", name);
	}
	json_object_put(jobj);
	free(url);
	return EXIT_SUCCESS;
}

/*
 * Function WriteMemoryCallback
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

