#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "sshserver.h"
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/buffer.h>

ssh_session session;
ssh_bind sshbind;

int run_ssh_server(int port){

	signal(SIGINT, (void(*)())free_glob);

	int timeout = 30;
	char * ip = "::";

	sshbind = ssh_bind_new();
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDADDR, ip);
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT, &port);
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_HOSTKEY, "ssh-rsa");
	char *key_path = getpath("private.pem");
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY,key_path);

	// bind to the given port
	if (ssh_bind_listen(sshbind) < 0) {
		printf("Error listening to socket: %s\n", ssh_get_error(sshbind));
		free(key_path);
		return -1;
	}

	print_remote_help(port, ip);
	print_fingerprint("public.pem");
	// wait for incoming connections for forever
	while (1) {
		session = ssh_new();
		if (session == NULL) {
			fprintf(stderr, "Failed to allocate session\n");
			continue;
		}
		ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeout);
		if (ssh_bind_accept(sshbind, session) == SSH_ERROR) {
			printf("Error: Connection could not get accepted: '%s'.\n",ssh_get_error(sshbind));
			return -1;
		}
		switch (fork())  {
			case -1:
				fprintf(stderr, "Fork returned error: '%d'.\n", -1);
				exit(-1);

			case 0:
				process_client();
				ssh_disconnect(session);
				ssh_free(session);
				exit(0);

			default:
				break;
		}
		ssh_disconnect(session);
		ssh_free(session);
	}
	ssh_bind_free(sshbind);
	ssh_finalize();
	return EXIT_SUCCESS;
}

/*
 * Function: print_remote_help
 *
 * give a copiable line to the user, which he may pass to the remote machines owner
 * if the ip address is the wildcard '::' replace its occurence with '<this machines ip>'
 *
 * port: the port on which the remote host should connect
 *
 * ip: the ip-address to which the remote host should connect
 */

void print_remote_help(int port, char * ip){
	printf("remote command:\n");
	if (strcmp(ip, "::")){
		printf("ssh-copy-id -p %d %s\n", port, ip);
	} else {
		printf("ssh-copy-id -p %d <this machines ip>\n", port);
	}
}

/*
 * Function: print_fingerprint
 *
 * print the sha256 fingerprint of a ssh-public-key in OpenSSH format
 *
 * file: the filename of the public key within the config-directory to print
 */

void print_fingerprint(const char * filename){
	char *pubpath;
	char *sha256fp=NULL;
	pubpath = getpath(filename);
	if (NULL == (sha256fp = fingerprint(pubpath))){
		printf("Warning: Could not determine the local pubkeys fingerprint.\n");
		free(pubpath);
		return;
	}
	free(pubpath);

	printf("SHA256:%s\n", sha256fp);

	free(sha256fp);
}

/*
 * Function: free_glob
 *
 * cleans up the to globals ssh session and the bind
 * is meant to be registered as interrupt handler
 */

void free_glob(void){
	ssh_disconnect(session);
	ssh_free(session);
	ssh_bind_free(sshbind);
	ssh_finalize();
	exit(EXIT_SUCCESS);
}

int process_client() {
	struct connection con;
	con.session = session;

	/* Perform key exchange. */
	if (ssh_handle_key_exchange(con.session)) {
		printf("Error exchanging keys: '%s'.\n", ssh_get_error(con.session));
		return -1;
	}

	ssh_message message;
	ssh_channel chan=0;
	char buffer[2048];
	int auth=0;
	int exec=0;
	int i;

	/* proceed to authentication */
	auth = authenticate(&con);
	if(!auth){
		ssh_disconnect(con.session);
		return 1;
	}
	do {
		message = ssh_message_get(con.session);
		if(message){
			if(ssh_message_type(message) == SSH_REQUEST_CHANNEL_OPEN
			&& ssh_message_subtype(message) == SSH_CHANNEL_SESSION) {
				chan = ssh_message_channel_request_open_reply_accept(message);
				ssh_message_free(message);
				break;
			} else {
				ssh_message_reply_default(message);
				ssh_message_free(message);
			}
		} else {
			break;
		}
	} while(!chan);

	if(!chan) {
		printf("Error: Client did not ask for a channel session (%s)\n",
		ssh_get_error(con.session));
		ssh_finalize();
		return 1;
	}

	do {
		message = ssh_message_get(con.session);
		if(message != NULL) {
			if(ssh_message_type(message) == SSH_REQUEST_CHANNEL) {
				if (ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_EXEC){
					exec = 1;
					ssh_message_channel_request_reply_success(message);
					ssh_message_free(message);
					continue;
				}
			}
			ssh_message_reply_default(message);
			ssh_message_free(message);
		} else {
			break;
		}
	} while(!exec);


	if (exec) {
		if ((i = ReadExec(chan, buffer, 1024)) > 0){
			char buf[i];
			snprintf(buf, sizeof buf, "%s", buffer);

			show_content(&con, buf);
			ssh_channel_write(chan,"INFO: Received keys, thanks for using heimdallr!\r\n",50);
			ssh_channel_close(chan);
			ssh_disconnect(session);
			ssh_finalize();
			return 0;
		}
	}
	ssh_disconnect(con.session);
	return 0;
}

int authenticate(struct connection *c) {
	ssh_message message;
	do {
		message = ssh_message_get(session);
		if(!message)
			break;
		switch(ssh_message_type(message)){
			case SSH_REQUEST_AUTH:
				switch(ssh_message_subtype(message)){
					case SSH_AUTH_METHOD_INTERACTIVE:
						ssh_message_auth_reply_success(message,0);
						ssh_message_free(message);
						return 1;
					case SSH_AUTH_METHOD_NONE:
					default:
						ssh_message_auth_set_methods(message,
							SSH_AUTH_METHOD_INTERACTIVE);
						ssh_message_reply_default(message);
						break;
				}
				break;
				default:
				ssh_message_auth_set_methods(message,
					SSH_AUTH_METHOD_INTERACTIVE);
				ssh_message_reply_default(message);
		}
		ssh_message_free(message);
	} while (ssh_get_status(session) != SSH_CLOSED ||
		ssh_get_status(session) != SSH_CLOSED_ERROR);
	return 0;
}

int ReadExec(ssh_channel chan, void *vptr, int maxlen) {
	int n=0, rc=0, ctr=0;
	char    c, *buffer;
	buffer = vptr;

	for ( n = 1; n < maxlen; n++ ) {
		if ( (rc = ssh_channel_read(chan, &c, 1, 0)) == 1 ) {
			if(ctr > 0){
				ctr = ctr +1;
			}
			if(ctr > 3){
				ctr = 0;
			}
			if ( c == '\r' || c == '\n' ){
				break;
			}
			if(c != '\r' || c != '\n' || c != '\0'){
				*buffer++ = c;
			}
			if ( c == '\r' ){
				break;
			}
		} else if ( rc == 0 ) {
			if ( n == 1 )
				return 0;
			else
				break;
		} else {
			if ( errno == EINTR )
				continue;
			return -1;
		}
	}
	*buffer = 0;
	return n;
}

int show_content(struct connection *c, char* command) {
    printf("%s\n", command);
    return 0;
}

/*
 * Function: fingerprint
 *
 * calculates the sha256 fingerprint of a given public key file in OpenSSH format
 *
 * path: the path of a file holding a public key file in OpenSSH format
 *
 * returns: the string holding the fingerprint . Needs to be freed by the caller.
 */

char* fingerprint(const char * path){
	char *fingerprint64;
	unsigned char *fingerprint;
	char last_ch=EOF, ch;
	FILE *fp;
	int maxlen = 16384+1;
	int word=0;
	int c=0;
	char buf[16384+1]="";
	size_t test, k;
	unsigned char *d;
	char *output_pad, *output_unpad;

	// read pubkey base64 string into fingerprint64
	fp = fopen(path, "r");
	if (fp == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}
	while((ch = fgetc(fp)) != '\n') {
		if (last_ch!=' ' && ch == ' '){
			word++;
		} else if (last_ch == ' ' && ch == ' ') {
			continue;
		}
		if (' ' != ch && 1 == word && c<maxlen){
			buf[c]=ch;
			c++;
		}
		last_ch = ch;
	}
	fclose(fp);
	fingerprint64 = strdup(buf);

	base64Decode(buf, &fingerprint, &test);
	d = SHA256(fingerprint, calcDecodeLength(fingerprint64), 0);

	// remove padding
	output_pad = base64Encode(d, 32);
	k=strlen(output_pad);
	for (; output_pad[k-1]=='=';k--);
	output_unpad = malloc(k+1);
	strncpy(output_unpad, output_pad, k);
	output_unpad[k] = '\0';

	free(fingerprint);
	free(fingerprint64);
	free(output_pad);

	return output_unpad;
}

/*
 * Function: calcDecodeLength
 * author:  john<at>nachtimwald<dot>com
 *
 * b64input: calculates the length of a decoded base64 string
 *
 * returns: the length of the decoded data
 */

size_t calcDecodeLength(const char* b64input) {
	size_t len = strlen(b64input),
		padding = 0;

	if (b64input[len-1] == '=' && b64input[len-2] == '=')
		padding = 2;
	else if (b64input[len-1] == '=')
		padding = 1;

	return (len*3)/4 - padding;
}

/*
 * Function: base64Decode
 * author:  john<at>nachtimwald<dot>com
 *
 * decodes base64 strings into byte data
 *
 * b64message: the base64 string
 *
 * buffer: where to store the decoded data
 *
 * length: how long the decoded data will be (use calcDecodeLength())
 */

int base64Decode(char* b64message, unsigned char** buffer, size_t* length) {
	BIO *bio, *b64;

	int decodeLen = calcDecodeLength(b64message);
	*buffer = (unsigned char*)malloc(decodeLen + 1);
	(*buffer)[decodeLen] = '\0';

	bio = BIO_new_mem_buf(b64message, -1);
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
	*length = BIO_read(bio, *buffer, strlen(b64message));
	assert(*length == decodeLen);
	BIO_free_all(bio);

	return (0);
}

/*
 * Function: base64Encode
 * author:  john<at>nachtimwald<dot>com
 *
 * encodes given byte data into a base64 encoded string
 *
 * input: the unencoded byte data
 *
 * length: the length of the byte data, as it lacks a null terminator
 *
 * returns: the pointer to the base64 string. The caller needs to free it after using it.
 */

char *base64Encode(const unsigned char *input, int length)
{
	BIO *bmem, *b64;
	BUF_MEM *bptr;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	char *buff = (char *)malloc(bptr->length);
	memcpy(buff, bptr->data, bptr->length-1);
	buff[bptr->length-1] = 0;

	BIO_free_all(b64);

	return buff;
}
