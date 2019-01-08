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

	signal(SIGINT, (void (*)())free_glob);

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
			printf("Error: Connection could not get accepted: '%s'.\n",
			       ssh_get_error(sshbind));
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
	if (strcmp(ip, "::")) {
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
	unsigned char *hash = NULL;
	ssh_key srv_pubkey = NULL;
	int rc = 0;
	char *pubpath;
	size_t hlen;

	pubpath = getpath(filename);

	rc = ssh_pki_import_pubkey_file(pubpath, &srv_pubkey);
	free(pubpath);
	if (rc < 0) {
		return;
	}

	rc = ssh_get_publickey_hash(srv_pubkey,
				    SSH_PUBLICKEY_HASH_SHA256,
				    &hash,
				    &hlen);
	ssh_key_free(srv_pubkey);
	if (rc < 0) {
		return;
	}

	ssh_print_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
	free(hash);
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
	if(!auth) {
		ssh_disconnect(con.session);
		return 1;
	}
	do {
		message = ssh_message_get(con.session);
		if(message) {
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
				if (ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_EXEC) {
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
		if ((i = ReadExec(chan, buffer, 1024)) > 0) {
			char buf[i];
			snprintf(buf, sizeof buf, "%s", buffer);

			show_content(&con, buf);
			ssh_channel_write(chan,
			                  "INFO: Received keys, thanks for using heimdallr!\r\n",
			                  50);
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
		switch(ssh_message_type(message)) {
		case SSH_REQUEST_AUTH:
			switch(ssh_message_subtype(message)) {
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
	int n=0, rc=0;
	char c, *buffer;
	buffer = vptr;

	for ( n = 1; n < maxlen; n++ ) {
		if ( (rc = ssh_channel_read(chan, &c, 1, 0)) == 1 ) {
			if ( c == '\r' || c == '\n' ) {
				break;
			}
			if(c != '\r' || c != '\n' || c != '\0') {
				*buffer++ = c;
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

