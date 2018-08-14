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
 * returns: the result from string to int conversion, if the int result is between MINPORT and MAXPORT
 *          if anything went wrong returns -1
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
 * finds the fullpath of the current user's homedirectory
 * reading the content of the environment variable 'HOME'
 * and if that fails, reading the result of getpwuid()
 *
 * returns: the home directories full path like '/home/foobar'
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
 * builds the full path from heimdallr's config directory and the given filename
 *
 * filename: the filename as string
 *
 * returns: the full path of a filename by concatenating the current homedir,
 *          the config folder, as well as the filename
 *          like this: <homedir>/.config/heimdallr/<filename>
 */

char* getpath(const char* filename){
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
 * Function: ensure_config_dir
 *
 * creates the directory structure '<homedir>/.config/heimdallr/', if it does not exist
 *
 * returns: 0 if it succeeded, a larger number if not
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
 * Function: generate_key
 *
 * creates a file containing a valid 2048-bit rsa privatekey
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

/*
 * Function: generate_pukey_from_private
 *
 * create a file containing a ssh public key in base64 format,
 * based on a given private key file
 *
 * private: the filename of the private key within the configdirectory,
 *          to read the details from
 *
 * returns; zero if everything went well; and one if not.
 */

int generate_pubkey_from_private(char * private){
	char * pub_path;
	char * key_path;

	ssh_key privkey;
	ssh_key pubkey;

	pub_path = getpath("public.pem");
	if (access(pub_path, F_OK) != -1){
		// pubkey already exists, do nothing
		free(pub_path);
		return 0;
	}

	key_path = getpath("private.pem");
	if (ssh_pki_import_privkey_file(key_path, NULL, NULL, NULL, &privkey) != SSH_OK){
		free(pub_path);
		free(key_path);
		ssh_key_free(privkey);
		return 1;

	}
	free(key_path);

	if (SSH_OK != ssh_pki_export_privkey_to_pubkey(privkey, &pubkey)){
		printf("Error: Could not generate public- from private-key file.\n");
		free(pub_path);
		ssh_key_free(privkey);
		ssh_key_free(pubkey);
		return 1;
	}
	ssh_key_free(privkey);

	if (SSH_OK != ssh_pki_export_pubkey_file(pubkey, pub_path)){
		printf("Error: Could not export the public key to file.\n");
		free(pub_path);
		ssh_key_free(pubkey);
		return 1;
	}

	free(pub_path);
	ssh_key_free(pubkey);
	return 0;
}

/*
 * Function: ssh_pki_export_pubkey_file
 *
 * Export a public key to a file on disk in OpenSSH format.
 *
 * pubkey: the ssh_key handle
 *
 * filename: the path where to store the public-key-file
 *
 * returns: SSH_OK on success, SSH_ERROR on error
 */

int ssh_pki_export_pubkey_file(const ssh_key pubkey, const char * filename){
	enum ssh_keytypes_e type;
	char * b64;
	char * keytype;

	if(SSH_KEYTYPE_UNKNOWN == (type = ssh_key_type(pubkey))){
		printf("Error: Could not determine the public keys type.\n");
		return SSH_ERROR;
	}
	keytype = strdup(ssh_key_type_to_char(type));

	if (SSH_OK != ssh_pki_export_pubkey_base64(pubkey, &b64)){
		printf("Error: Could not export public key.\n");
		ssh_string_free_char(keytype);
		return SSH_ERROR;
	}

	FILE *fp = fopen(filename, "ab");
	if (fp != NULL)
	{
		fputs(keytype, fp);
		fputs(" ", fp);
		fputs(b64, fp);
		fputs("\n", fp);
		fclose(fp);
	}

	ssh_string_free_char(b64);
	ssh_string_free_char(keytype);
	return SSH_OK;
}
