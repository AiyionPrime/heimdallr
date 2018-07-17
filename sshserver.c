#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "sshserver.h"
#include <libssh/libssh.h>
#include <libssh/server.h>

ssh_session session;
ssh_bind sshbind;

int run_ssh_server(int port){
	int timeout = 30;

	session = ssh_new();
	ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeout);
	sshbind = ssh_bind_new();
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDADDR, LISTENADDRESS);
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT, &port);
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_HOSTKEY, "ssh-rsa");
	const char *key_path = getpath("private.pem");
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY,key_path);

	// bind to the given port
	if (ssh_bind_listen(sshbind) < 0) {
		printf("Error listening to socket: %s\n", ssh_get_error(sshbind));
		return -1;
	}

	// wait for incoming connections for forever
	while (1) {
		if (ssh_bind_accept(sshbind, session) == SSH_ERROR) {
			printf("Error: Connection could not get accepted: '%s'.\n",ssh_get_error(sshbind));
			return -1;
		}
		switch (fork())  {
			case -1:
				fprintf(stderr, "Fork returned error: '%d'.\n", -1);
				exit(-1);

			case 0:
				exit(process_client());

			default:
				break;
		}
	}
	return EXIT_SUCCESS;
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
		printf("Authentication error: %s\n", ssh_get_error(session));
		ssh_disconnect(session);
		return 1;
	}
	do {
		message = ssh_message_get(session);
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
		ssh_get_error(session));
		ssh_finalize();
		return 1;
	}

	do {
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

	ssh_disconnect(session);

	return 0;
}

int authenticate(struct connection *c) {
	return EXIT_SUCCESS;
}

int ReadExec(ssh_channel chan, void *vptr, int maxlen) {
	return 0;
}

int show_content(struct connection *c, char* command) {
    printf("%s:\n%s\n", c->client_ip, command);
    return 0;
}
