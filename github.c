#include "github.h"
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

int ensure_input(int options)
{
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

void get_keys(const char *username)
{       
        char *key_url = "https://api.github.com/users/%s/keys";
        int arraylen, jtype;
        int new_len = strlen(username)+strlen(key_url)-2+1;
        char *built_url;
        built_url = malloc(new_len * sizeof(char));
        snprintf(built_url, new_len, key_url, username);
        
        CURL *curl;
        CURLcode res;
        struct MemoryStruct chunk;
        struct json_object *jobj, *tuplejobj, *keyjobj;
        chunk.memory = malloc(1);
        chunk.size = 0;
        
        curl = curl_easy_init();
        if(curl) { 
                if (!built_url){
                        printf("Error: malloc fuckup.\n");
                        return;
                }
                curl_easy_setopt(curl, CURLOPT_URL, built_url); 
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
                }
        }
        curl_easy_cleanup(curl);
        free(chunk.memory);
        free(built_url);
}

int find_user(char *name)
{
        char *baseurl = "https://api.github.com/search/users?q=";
        char *url;
        struct json_object *jobj, *userjobj, *usernamejobj;
        struct json_object *returnObj;
        CURL *curl;
        CURLcode res;
        struct MemoryStruct chunk;

        int arraylen, target;

        chunk.memory = malloc(1);
        chunk.size = 0;

        curl = curl_easy_init();
        if(curl) {
                url = malloc(strlen(baseurl)+strlen(name)+1);
                if (!url) {
                        printf("Error, malloc failed; no memory available.\n");
                        return EXIT_FAILURE;
                }
                strcpy(url, baseurl);
                strcat(url, name);

                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_USERAGENT, "Kraken/1.0");
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
                res = curl_easy_perform(curl);

                if(res != CURLE_OK){
                        fprintf(stderr, "Error: %s\n",
                                curl_easy_strerror(res));
                } else {

                        jobj = json_tokener_parse(chunk.memory);
                        returnObj = json_object_object_get(jobj, "items");

                        arraylen = json_object_array_length(returnObj);

                        for (int i=0; i<arraylen; i++){
                                userjobj = json_object_array_get_idx(returnObj, i);
                                usernamejobj = json_object_object_get(userjobj, "login");
                                printf("%i: %s\n", i, json_object_get_string(usernamejobj));
                        }
                        target = ensure_input(arraylen);
                        get_keys(json_object_get_string(json_object_object_get(json_object_array_get_idx(returnObj, target), "login")));
                	json_object_put(jobj);
                }
                curl_easy_cleanup(curl);
                free(url);
                free(chunk.memory);
        }
        return EXIT_SUCCESS;
}

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

