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
				exit(authenticate());

			default:
				break;
		}
	}
	return EXIT_SUCCESS;
}

int authenticate() {
	return EXIT_SUCCESS;
}
