#include "config.h"
#include <errno.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

/*
 * Function: valid_port
 *
 * ensure the given string contains an integer between MINPORT and MAXPORT
 *
 * *p: the pointer to a string containing digits
 *
 * returns: the converted resulted from string to int, if the int result is beteen MINPORT and MAXPORT
 * if anything went wrong returns -1
 */

int valid_port(char *p) {
	errno = 0;
	char *endptr;
	int port;

	port = strtol(p, &endptr, 10);
	if (MINPORT <= port && port <= MAXPORT && !*endptr && errno == 0)
		return port;
	return -1;
}

/*
 * Function: homedir
 *
 * finds the fullpath of the currents users homedirectory
 * reading the content of the environment variable 'HOME'
 * and if that fails, reading the result of getpwuid()
 *
 * returns: the home directories fullpath like '/home/foobar'
 */

const char* homedir(){
        const char *homedir;
        if ((homedir = getenv("HOME")) == NULL) {
                homedir = getpwuid(getuid())->pw_dir;
        }
        return homedir;
}

/*
 * Function: getpath
 *
 * builds the full path from heimdallrs config directory and the given filename
 *
 * filename: the filename as string
 *
 * returns: the full path consisting of the current homedir,
 *          the config folder, as well as the filename
 */

char* getpath(char* filename){
        char *relative_path = "/.config/heimdallr/";
        const char *home = homedir();
        int newlen = strlen(filename) + strlen(relative_path) + strlen(home) + 1;
        char *fullpath;
        fullpath = (char *)calloc(sizeof(char), newlen);
        strcat(fullpath, home);
        strcat(fullpath, relative_path);
        strcat(fullpath, filename);
        return fullpath;
}

/*
 * ensure_config_dir
 *
 * creates the directory structure '<homedir>/.config/heidallr/' if it does not exists
 *
 * returns: 0 if it succeeded a larger number if not
 */

int ensure_config_dir(){
	char *dir_path = getpath("");
	int ret = mkdir(dir_path, S_IRWXU);
	if (-1 == ret && errno == 17){
		ret = 1;
		errno = 0;
	}
	free(dir_path);
	return ret;
}

/*
 * generate_key
 *
 * creates a file containing a valid 2048 bit rsa-private key
 * in the config directory, using openssl
 *
 * this is done in order not to interact with the user-keys
 *
 * returns: 1 if everything went well, 0 if not
 */

int generate_key()
{
	int	ret = 0;
	RSA	*r = NULL;
	BIGNUM	*bne = NULL;
	BIO	*bp_public = NULL, *bp_private = NULL;

	int		bits = 2048;
	unsigned long	e = RSA_F4;

	bne = BN_new();
	ret = BN_set_word(bne,e);
	if(ret != 1){
		goto free_all;
	}

	r = RSA_new();
	ret = RSA_generate_key_ex(r, bits, bne, NULL);
	if(ret != 1){
		goto free_all;
	}

	char *key_path = getpath("private.pem");
	bp_private = BIO_new_file(key_path, "w+");
	ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);

	free(key_path);

	free_all:

	BIO_free_all(bp_public);
	BIO_free_all(bp_private);
	RSA_free(r);
	BN_free(bne);

	return (ret == 1);
}
