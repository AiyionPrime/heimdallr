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


int valid_port(char *p) {
	char *endptr;
	int port;

	port = strtol(p, &endptr, 10);
	if (MINPORT <= port && port <= MAXPORT && !*endptr && errno == 0)
		return port;
	return -1;
}

const char* homedir(){
        const char *homedir;
        if ((homedir = getenv("HOME")) == NULL) {
                homedir = getpwuid(getuid())->pw_dir;
        }
        return homedir;
}

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

int ensure_config_dir(){
	char *dir_path = getpath("");
	int ret = mkdir(dir_path, S_IRWXU);
	free(dir_path);
	return ret;
}

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

	free_all:

	free(key_path);
	BIO_free_all(bp_public);
	BIO_free_all(bp_private);
	RSA_free(r);
	BN_free(bne);

	return (ret == 1);
}
